from dataclasses import dataclass
from pathlib import Path
import argparse
import copy
import textwrap
import os
import re
from patch_functions import *


def get_full_filepath(filepath):
    main_directory = Path(os.path.abspath(__file__)).parent.parent
    return Path(main_directory) / filepath


def update_version_number_in_file(relative_filepath, pattern, new_version_number):
    full_filepath = get_full_filepath(relative_filepath)

    with open(full_filepath, "r", encoding="UTF-8") as file:
        file_contents = file.read()
        original = copy.deepcopy(file_contents)

        modified = apply_fixes(pattern, new_version_number, file_contents)
        patch = create_patch(str(full_filepath), original, modified)

        if args.patch_only:
            print(patch)
            return

        if not patch:
            if not args.quiet:
                print(f"No changes to make to {full_filepath}")
            return

        make_change = possibly_apply_patch(
            patch, full_filepath, quiet=args.quiet, force=args.force
        )

        if make_change:
            with open(full_filepath, "w", encoding="UTF-8") as the_file:
                the_file.write(modified)


def bump_version_numbers(new_version_number):
    short_version_number = ShortVersionNumber(
        new_version_number.major_version, new_version_number.minor_version
    )
    bout_next_version_number = VersionNumber(
        new_version_number.major_version,
        new_version_number.minor_version + 1,
        new_version_number.patch_version,
    )

    update_version_number_in_file(
        "configure.ac",
        r"^AC_INIT\(\[BOUT\+\+\],\[(\d+\.\d+\.\d+)\]",
        new_version_number,
    )
    update_version_number_in_file(
        "CITATION.cff", r"^version: (\d+\.\d+\.\d+)", new_version_number
    )
    update_version_number_in_file(
        "manual/sphinx/conf.py", r"^version = \"(\d+\.\d+)\"", short_version_number
    )
    update_version_number_in_file(
        "manual/sphinx/conf.py", r"^release = \"(\d+\.\d+\.\d+)\"", new_version_number
    )
    update_version_number_in_file(
        "manual/doxygen/Doxyfile_readthedocs",
        r"^PROJECT_NUMBER         = (\d+\.\d+\.\d+)",
        new_version_number,
    )
    update_version_number_in_file(
        "manual/doxygen/Doxyfile",
        r"^PROJECT_NUMBER         = (\d+\.\d+\.\d+)",
        new_version_number,
    )
    update_version_number_in_file(
        "CMakeLists.txt",
        r"^set\(_bout_previous_version \"v(\d+\.\d+\.\d+)\"\)",
        new_version_number,
    )
    update_version_number_in_file(
        "CMakeLists.txt",
        r"^set\(_bout_next_version \"(\d+\.\d+\.\d+)\"\)",
        bout_next_version_number,
    )
    update_version_number_in_file(
        "tools/pylib/_boutpp_build/backend.py",
        r"_bout_previous_version = \"v(\d+\.\d+\.\d+)\"",
        new_version_number,
    )
    update_version_number_in_file(
        "tools/pylib/_boutpp_build/backend.py",
        r"_bout_next_version = \"(\d+\.\d+\.\d+)\"",
        bout_next_version_number,
    )


@dataclass
class VersionNumber:
    major_version: int
    minor_version: int
    patch_version: int

    def __str__(self):
        return "%d.%d.%d" % (self.major_version, self.minor_version, self.patch_version)


@dataclass
class ShortVersionNumber:
    major_version: int
    minor_version: int

    def __str__(self):
        return "%d.%d" % (self.major_version, self.minor_version)


def apply_fixes(pattern, new_version_number, source):
    """Apply the various fixes for each factory to source. Returns
    modified source

    Parameters
    ----------
    pattern
        Regex pattern to apply for replacement
    new_version_number
        New version number to use in replacement
    source
        Text to update
    """

    def get_replacement(match):
        return match[0].replace(match[1], str(new_version_number))

    modified = re.sub(pattern, get_replacement, source, flags=re.MULTILINE)

    return modified


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=textwrap.dedent(
            """\
            Update the software version number to the specified version, 
            to be given in the form major.minor.patch, 
            e.g. 5.10.3
            
            Where the 3rd ('patch') part of the version is omitted, 
            only the 'major' and 'minor' parts will be used, 
            e.g. 5.10.3 -> 5.10 
            
            For the 'bout-next' version number, 
            the 'minor' version number of the provided version will be incremented by 1, 
            e.g. 5.10.3 -> 5.11.3
            
            """
        ),
    )

    parser.add_argument(
        "--force", "-f", action="store_true", help="Make changes without asking"
    )
    parser.add_argument(
        "--quiet", "-q", action="store_true", help="Don't print patches"
    )
    parser.add_argument(
        "--patch-only", "-p", action="store_true", help="Print the patches and exit"
    )
    parser.add_argument("new_version", help="Specify the new version number")

    args = parser.parse_args()

    if args.force and args.patch_only:
        raise ValueError("Incompatible options: --force and --patch")
    major, minor, patch = map(int, args.new_version.split("."))
    bump_version_numbers(new_version_number=VersionNumber(major, minor, patch))
