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
        modified_contents = modify(contents)

        # boutupgrader.apply_or_display_patch(
        #     filename, original, modified_contents, args.patch_only, args.quiet, args.force
        # )

        return modified_contents


def modify(original_string):
    using_new_metric_accessor_methods = use_metric_accessors(original_string)
    without_geometry_calls = remove_geometry_calls(using_new_metric_accessor_methods)
    without_geometry_calls.append("")  # insert a blank line at the end of the file
    lines_as_single_string = "\n".join(without_geometry_calls)
    modified_contents = replace_one_line_cases(lines_as_single_string)
    return modified_contents


def remove_geometry_calls(lines):
    # Remove lines calling geometry()
    geometry_method_call_pattern = r"geometry\(\)"
    lines_to_remove = indices_of_matching_lines(geometry_method_call_pattern, lines)
    for line_index in lines_to_remove:
        # If both the lines above and below are blank then remove one of them
        if lines[line_index - 1].strip() == "" and lines[line_index + 1].strip() == "":
            del lines[line_index + 1]
        del (lines[line_index])
    return lines


def use_metric_accessors(original_string):

    lines = original_string.splitlines()

    # find lines like: c->g_11 = x; and c.g_11 = x;
    pattern_setting_metric_component = r"(\b.+\-\>|\.)(g_?)(\d\d)\s?\=\s?(.+)(?=;)"
    line_matches = re.findall(pattern_setting_metric_component, original_string)
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
        new_value = re.sub(r"(\b\w+->|\.)(g_?\d\d)", r"\2", value)  # Replace `c->g11` with `g11`, etc
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
    return lines


# Deal with the basic find-and-replace cases that do not involve multiple lines
def replace_one_line_cases(modified):

    arrow_or_dot = r"(\b.+\-\>|\.)"
    metric_component = r"(g_?\d\d)"
    mesh_spacing = r"d([xyz])"
    not_followed_by_equals = r"(?!\s?=)"
    equals_something = r"\=\s?(.+)(?=;)"
    
    patterns_with_replacements = {

        # Replace `c->g_11` with `c->g_11()` etc, but not if is assignment
        fr"{arrow_or_dot}{metric_component}{not_followed_by_equals}":
            r"\1\2()",

        # Replace `->dx =` with `->setDx()`, etc
        fr"{arrow_or_dot}{mesh_spacing}\s?{equals_something}":
            r"\1setD\2(\3)",
        # Replace `foo->dx /= bar` with `foo->setDx(foo->dx() / (bar))`
        fr"{arrow_or_dot}{mesh_spacing}\s?\/{equals_something}":
            r"\1setD\2(\1d\2 / (\3))",
        # Replace `c->dx` with `c->dx()` etc, but not if is assignment
        fr"{arrow_or_dot}{mesh_spacing}{not_followed_by_equals}":
            r"\1d\2()",

        # Replace `->Bxy =` with `->setBxy()`, etc
        fr"{arrow_or_dot}Bxy\s?{equals_something}":
            r"\1setBxy(\2)",
        # Replace `foo->Bxy /= bar` with `foo->setBxy(foo->Bxy() / (bar))`
        fr"{arrow_or_dot}Bxy\s?\/{equals_something}":
            r"\1setBxy(\1Bxy / \2)",
        # Replace `c->Bxy` with `c->Bxy()` etc, but not if is assignment
        fr"{arrow_or_dot}Bxy{not_followed_by_equals}":
            r"\1Bxy()",

        # Replace `->J =` with `->setJ()`, etc
        fr"{arrow_or_dot}J\s?{equals_something}":
            r"\1setJ(\2)",
        # Replace `foo->J /= bar` with `foo->setJ(foo->J() / (bar))`
        fr"{arrow_or_dot}J\s?\/{equals_something}":
            r"\1setJ(\1J / \2)",
        # Replace `c->J` with `c->J()` etc, but not if is assignment
        fr"{arrow_or_dot}J{not_followed_by_equals}":
            r"\1J()",

        # Replace `->IntShiftTorsion =` with `->setIntShiftTorsion()`, etc
        fr"{arrow_or_dot}IntShiftTorsion\s?{equals_something}":
            r"\1setIntShiftTorsion(\2)",
        # Replace `foo->IntShiftTorsion /= bar` with `foo->setIntShiftTorsion(foo->IntShiftTorsion() / (bar))`
        fr"{arrow_or_dot}IntShiftTorsion\s?\/{equals_something}":
            r"\1setIntShiftTorsion(\1IntShiftTorsion / \2)",
        # Replace `c->IntShiftTorsion` with `c->IntShiftTorsion()` etc, but not if is assignment
        fr"{arrow_or_dot}IntShiftTorsion{not_followed_by_equals}":
            r"\1IntShiftTorsion()",
    }
    for pattern, replacement in patterns_with_replacements.items():
        modified = re.sub(pattern, replacement, modified)
    return modified


def indices_of_matching_lines(pattern, lines):
    search_result_for_all_lines = [re.search(pattern, line) for line in lines]
    matches = [x for x in search_result_for_all_lines if x is not None]
    return [lines.index(match.string) for match in matches]


if __name__ == "__main__":
    main()
