#!/usr/bin/env python3
import os
import sys

import boutupgrader

import argparse
import copy
import pathlib
import re
import textwrap

# # Regular expression dx, dy, or dz
# dx_dy_or_dz_regex = re.compile(
#     r"""(\.d\w)
#     """,
#     re.VERBOSE | re.MULTILINE,
# )

RHS_RE = re.compile(r"solver\s*->\s*setRHS\(\s*([a-zA-Z0-9_]+)\s*\)")

PRECON_RE = re.compile(r"solver\s*->\s*setPrecon\(\s*([a-zA-Z0-9_]+)\s*\)")

JACOBIAN_RE = re.compile(r"solver\s*->\s*setJacobian\(\s*([a-zA-Z0-9_]+)\s*\)")

SPLIT_OPERATOR_RE = re.compile(
    r"solver\s*->\s*setSplitOperator\(\s*([a-zA-Z0-9_]+),\s*([a-zA-Z0-9_]+)\s*\)"
)


def main(*args, **kwargs):

    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=textwrap.dedent(
            """\
            Upgrade files to use the refactored Coordinates class. 
            
            For example, changes coords->dx to coord->dx()
            """
        ),
    )
    parser = boutupgrader.default_args(parser)
    # parser.add_argument(
    # )
    args = parser.parse_args(args=args)

    path = pathlib.Path(args.files[0])
    if pathlib.Path.is_dir(path):

        filepaths = [os.path.join(dir_path, f)
                     for (dir_path, dir_names, filenames) in os.walk(path)
                     for f in filenames]
        # files = os.listdir(args.files[0], )
        # filepaths = [file]
    else:
        filepaths = args.files

    for filename in filepaths:
        try:
            # with open(filename, "r") as f:
            with pathlib.Path(filename).open() as f:
                contents = f.read()
        except Exception as e:
            print(f"Error reading {filename}", file=sys.stderr)
            print(e, file=sys.stderr)

        original = copy.deepcopy(contents)

        patterns_with_replacements = {
            r"(\-\>|\.)d(\w)\s?\=\s?(.+?\b)": r"\1setD\2(\3)",  # Replace `->dx =` with `->setDx()`
            r"(\-\>|\.)(d\w)\s?(?!=)": r"\1\2()",  # Replace `->dx` with `->dx()` but not if is assignment
        }

        modified = contents
        for pattern, replacement in patterns_with_replacements.items():
            modified = re.sub(pattern, replacement, modified)

        # match = dx_dy_or_dz_regex.search(original)
        # if match is not None:
        #     new_name = match.group("name")
        # else:
        #     new_name = args.name or pathlib.Path(filename).stem.capitalize().replace(
        #         " ", "_"
        #     )

        # try:
        #     if re.match(r"^[0-9]+.*", new_name) and not args.force:
        #         raise ValueError(
        #             f"Invalid name: '{new_name}'. Use --name to specify a valid C++ identifier"
        #         )
        #     modified = re.sub(dx_dy_or_dz_regex, r"\1", original)
        # except (RuntimeError, ValueError) as e:
        #     error_message = textwrap.indent(f"{e}", " ")
        #     print(
        #         f"There was a problem applying automatic fixes to {filename}:\n\n{error_message}"
        #     )
        #     continue

        boutupgrader.apply_or_display_patch(
            filename, original, modified, args.patch_only, args.quiet, args.force
        )


if __name__ == "__main__":
    main()
