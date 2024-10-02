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
        filepaths = [
            os.path.join(dir_path, f)
            for (dir_path, dir_names, filenames) in os.walk(path)
            for f in filenames
        ]
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


def indices_of_matching_lines(pattern, lines):
    search_result_for_all_lines = [re.search(pattern, line) for line in lines]
    matches = [x for x in search_result_for_all_lines if x is not None]
    return [lines.index(match.string) for match in matches]


def use_metric_accessors(original_string):

    lines = original_string.splitlines()

    # find lines like: c->g_11 = x; and c.g_11 = x;
    pattern_setting_metric_component = r"(\b.+\-\>|\.)(g_?)(\d\d)\s?\=\s?(.+)(?=;)"
    line_matches = re.findall(pattern_setting_metric_component, original_string)

    if len(line_matches) == 0:
        return lines

    metric_components = {match[1] + match[2]: match[3] for match in line_matches}
    lines_to_remove = indices_of_matching_lines(pattern_setting_metric_component, lines)
    lines_removed_count = 0
    for line_index in lines_to_remove:
        del lines[line_index - lines_removed_count]
        lines_removed_count += 1
    metric_components_with_value = {
        key: value for key, value in metric_components.items() if value is not None
    }
    newline_inserted = False
    for key, value in metric_components_with_value.items().__reversed__():
        # Replace `c->g11` with `g11`, etc
        new_value = re.sub(r"(\b\w+->|\.)(g_?\d\d)", r"\2", value)
        if not key.startswith("g_") and not newline_inserted:
            lines.insert(lines_to_remove[0], "")
            newline_inserted = True
        local_variable_line = rf"    const auto {key} = {new_value};"
        lines.insert(lines_to_remove[0], local_variable_line)
    # insert a blank line
    lines.insert(lines_to_remove[0] + len(metric_components_with_value) + 1, "")
    coordinates_name_and_arrow = line_matches[0][0]
    new_metric_tensor_setter = (
        f"    {coordinates_name_and_arrow}setMetricTensor(ContravariantMetricTensor(g11, g22, g33, g12, g13, g23),\n"
        f"                           CovariantMetricTensor(g_11, g_22, g_33, g_12, g_13, g_23));"
    )
    lines.insert(
        lines_to_remove[0] + len(metric_components_with_value) + 2,
        new_metric_tensor_setter,
    )
    del lines[lines_to_remove[-1] + 3]
    return lines


def remove_geometry_calls(lines):
    # Remove lines calling geometry()
    geometry_method_call_pattern = r"geometry\(\)"
    lines_to_remove = indices_of_matching_lines(geometry_method_call_pattern, lines)
    for line_index in lines_to_remove:
        # If both the lines above and below are blank then remove one of them
        if lines[line_index - 1].strip() == "" and lines[line_index + 1].strip() == "":
            del lines[line_index + 1]
        del lines[line_index]
    return lines


def assignment_regex_pairs(var):

    arrow_or_dot = r"\b.+\-\>|\."
    not_followed_by_equals = r"(?!\s?=)"
    equals_something = r"\=\s?(.+)(?=;)"

    def replacement_for_assignment(match):
        coord_and_arrow_or_dot = match.groups()[0]
        variable_name = match.groups()[1]
        capitalised_name = variable_name[0].upper() + variable_name[1:]
        value = match.groups()[2]
        return rf"{coord_and_arrow_or_dot}set{capitalised_name}({value})"

    def replacement_for_division_assignment(match):
        coord_and_arrow_or_dot = match.groups()[0]
        variable_name = match.groups()[1]
        capitalised_name = variable_name[0].upper() + variable_name[1:]
        value = match.groups()[2]
        denominator = (
            f"{value}" if value[0] == "(" and value[-1] == ")" else f"({value})"
        )
        return rf"{coord_and_arrow_or_dot}set{capitalised_name}({coord_and_arrow_or_dot}{variable_name} / {denominator})"

    return [
        # Replace `->var =` with `->setVar()`, etc
        (rf"({arrow_or_dot})({var})\s?{equals_something}", replacement_for_assignment),
        # Replace `foo->var /= bar` with `foo->setVar(foo->var() / (bar))`
        (
            rf"({arrow_or_dot})({var})\s?\/{equals_something}",
            replacement_for_division_assignment,
        ),
        # Replace `c->var` with `c->var()` etc, but not if is assignment
        (rf"({arrow_or_dot})({var})(?!\(){not_followed_by_equals}", rf"\1\2()"),
    ]


def mesh_get_pattern_and_replacement():

    # Convert `mesh->get(coord->dx(), "dx")` to `coord->setDx(mesh->get("dx"));`, etc

    def replacement_for_assignment_with_mesh_get(match):
        arrow_or_dot = match.groups()[0]
        coords = match.groups()[1]
        variable_name = match.groups()[3]
        new_value = match.groups()[4]
        capitalised_name = variable_name[0].upper() + variable_name[1:]
        return rf"{coords}{arrow_or_dot}set{capitalised_name}(mesh->get({new_value}))"

    arrow_or_dot = r"\-\>|\."

    mesh_get_pattern_replacement = (
        rf"mesh({arrow_or_dot})get\((\w+)({arrow_or_dot})(\w+)\(?\)?, (\"\w+\")\)",
        replacement_for_assignment_with_mesh_get,
    )
    return mesh_get_pattern_replacement


# Deal with the basic find-and-replace cases that do not involve multiple lines
def replace_one_line_cases(modified):

    metric_component = r"g_?\d\d"
    mesh_spacing = r"d[xyz]"

    patterns_with_replacements = (
        assignment_regex_pairs(metric_component)
        + assignment_regex_pairs(mesh_spacing)
        + assignment_regex_pairs("Bxy")
        + assignment_regex_pairs("J")
        + assignment_regex_pairs("IntShiftTorsion")
        + assignment_regex_pairs("G1")
        + assignment_regex_pairs("G2")
        + assignment_regex_pairs("G3")
    )

    patterns_with_replacements.append(mesh_get_pattern_and_replacement())

    for pattern, replacement in patterns_with_replacements:
        MAX_OCCURRENCES = 12
        count = 0
        while re.search(pattern, modified) and count < MAX_OCCURRENCES:
            count += 1
            modified = re.sub(pattern, replacement, modified)
    return modified


if __name__ == "__main__":
    main()
