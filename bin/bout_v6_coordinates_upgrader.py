#!/usr/bin/env python3
import os
import sys

import boutupgrader

import argparse
import copy
import pathlib
import re
import textwrap


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
    args = parser.parse_args(args=args)

    path = pathlib.Path(args.files[0])
    if pathlib.Path.is_dir(path):
        filepaths = [os.path.join(dir_path, f)
                     for (dir_path, dir_names, filenames) in os.walk(path)
                     for f in filenames]
    else:
        filepaths = args.files

    for filename in filepaths:
        try:
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

        boutupgrader.apply_or_display_patch(
            filename, original, modified, args.patch_only, args.quiet, args.force
        )


if __name__ == "__main__":
    main()
