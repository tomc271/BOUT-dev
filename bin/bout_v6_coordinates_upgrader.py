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
        modified_contents = get_modified_contents(contents)

        # boutupgrader.apply_or_display_patch(
        #     filename, original, modified_contents, args.patch_only, args.quiet, args.force
        # )

        return modified_contents


def get_modified_contents(contents):

    lines = contents.splitlines()

    pattern_setting_metric_component = r"(\b.+\-\>|\.)(g_?)(\d\d)\s?\=\s?(.+)(?=;)"
    line_matches = re.findall(pattern_setting_metric_component, contents)
    metric_components = {
        match[1] + match[2]: match[3]
        for match in line_matches
    }

    lines_to_remove = indices_of_matching_lines(pattern_setting_metric_component, lines)
    lines_removed_count = 0
    for line_index in lines_to_remove:
        del (lines[line_index - lines_removed_count])
        lines_removed_count += 1

    metric_components_with_value = {key: value for key, value in metric_components.items() if value is not None}
    newline_inserted = False
    for key, value in metric_components_with_value.items().__reversed__():
        new_value = replace_metric_tensor_cases(value)
        if not key.startswith("g_") and not newline_inserted:
            lines.insert(lines_to_remove[0], "")
            newline_inserted = True
        local_variable_line = rf"    const auto {key} = {new_value};"
        lines.insert(lines_to_remove[0], local_variable_line)

    lines.insert(lines_to_remove[0] + len(metric_components_with_value) + 1, "")  # insert a blank line

    new_metric_tensor_setter = (
        f"    coord->setMetricTensor(ContravariantMetricTensor(g11, g22, g33, g12, g13, g23),\n"
        f"                           CovariantMetricTensor(g_11, g_22, g_33, g_12, g_13, g_23));")

    lines.insert(lines_to_remove[0] + len(metric_components_with_value) + 2, new_metric_tensor_setter)
    del (lines[lines_to_remove[-1] + 3])

    lines.append("")  # insert a blank line

    modified = "\n".join(lines)

    modified = replace_one_line_cases(modified)

    return modified


# Deal with the basic find-and-replace cases that do not involve multiple lines
def replace_one_line_cases(modified):
    patterns_with_replacements = {
        r"(\-\>|\.)(g_?\d\d)(?!\s?=)": r"\1\2()",  # Replace `c->g_11` with `c->g_11()` etc, but not if is assignment

        r"(\-\>|\.)d([xyz])\s?\=\s?(.+?)(?=;)": r"\1setD\2(\3)",  # Replace `->dx =` with `->setDx()`, etc
        r"(\b.+\-\>|\.)d([xyz])\s?\/\=\s?(.+)(?=;)": r"\1setD\2(\1d\2 / (\3))",  # Replace `foo->dx /= bar` with `foo->setDx(foo->dx() / (bar))`
        r"(\-\>|\.)(d[xyz])(?!\s?=)": r"\1\2()",  # Replace `c->dx` with `c->dx()` etc, but not if is assignment

        r"(\-\>|\.)Bxy\s?\=\s?(.+?)(?=;)": r"\1setBxy(\2)",  # Replace `->Bxy =` with `->setBxy()`, etc
        r"(\b.+\-\>|\.)Bxy\s?\/\=\s?(.+)(?=;)": r"\1setBxy(\1Bxy / \2)",  # Replace `foo->Bxy /= bar` with `foo->setBxy(foo->Bxy() / (bar))`
        r"(\-\>|\.)Bxy(?!\s?=)": r"\1Bxy()",  # Replace `c->Bxy` with `c->Bxy()` etc, but not if is assignment

        r"(\-\>|\.)J\s?\=\s?(.+?)(?=;)": r"\1setJ(\2)",  # Replace `->J =` with `->setJ()`, etc
        r"(\b.+\-\>|\.)J\s?\/\=\s?(.+)(?=;)": r"\1setJ(\1J / \2)",  # Replace `foo->J /= bar` with `foo->setJ(foo->J() / (bar))`
        r"(\-\>|\.)J(?!\s?=)": r"\1J()",  # Replace `c->J` with `c->J()` etc, but not if is assignment

        r"(\-\>|\.)IntShiftTorsion\s?\=\s?(.+?)(?=;)": r"\1setIntShiftTorsion(\2)",  # Replace `->IntShiftTorsion =` with `->setIntShiftTorsion()`, etc
        r"(\b.+\-\>|\.)IntShiftTorsion\s?\/\=\s?(.+)(?=;)": r"\1setIntShiftTorsion(\1IntShiftTorsion / \2)",  # Replace `foo->IntShiftTorsion /= bar` with `foo->setIntShiftTorsion(foo->IntShiftTorsion() / (bar))`
        r"(\-\>|\.)IntShiftTorsion(?!\s?=)": r"\1IntShiftTorsion()",  # Replace `c->IntShiftTorsion` with `c->IntShiftTorsion()` etc, but not if is assignment
    }
    for pattern, replacement in patterns_with_replacements.items():
        modified = re.sub(pattern, replacement, modified)
    return modified


def replace_metric_tensor_cases(input_text):
    # Replace `c->g11` with `c->g11()`, etc, but not if is assignment
    pattern = r"(\b\w+\-\>|\.)(g_?\d\d)"
    replacement = r"\2"
    modified = re.sub(pattern, replacement, input_text)
    return modified


def indices_of_matching_lines(last_component_pattern, lines):
    search_result_for_all_lines = [re.search(last_component_pattern, line) for line in lines]
    matches = [x for x in search_result_for_all_lines if x is not None]
    return [lines.index(match.string) for match in matches]


def index_of_first_matching_line(last_component_pattern, lines):
    search_result_for_all_lines = [re.search(last_component_pattern, line) for line in lines]
    first_matching_line = [x for x in search_result_for_all_lines if x is not None][0].string
    return lines.index(first_matching_line)


if __name__ == "__main__":
    main()
