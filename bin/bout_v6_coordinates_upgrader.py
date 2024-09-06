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
    # Replace
    # `c->g11 = bar;`
    # with
    # `const auto g11 = bar;
    # coord->setMetricTensor(ContravariantMetricTensor(g11, g22, g33, g12, g13, g23),
    #                CovariantMetricTensor(g_11, g_22, g_33, g_12, g_13, g_23));`
    # etc

    lines = contents.splitlines()

    # g11_pattern = r"(\b.+\-\>|\.)g11\s?\=\s?(.+)(?=;)"
    # first_g11_match = re.search(g11_pattern, contents)
    # remove_matching_line(g11_pattern, lines)
    # # line_index = index_of_first_matching_line(g11_pattern, lines)
    # # lines.__delitem__(line_index)

    # TODO:
    # g11_matches = re.findall(g11_pattern, contents)
    # if len(g11_matches) > 1:

    pattern_setting_metric_component = r"(\b.+\-\>|\.)(g_?)(\d\d)\s?\=\s?(.+)(?=;)"
    line_matches = re.findall(pattern_setting_metric_component, contents)
    metric_components = {
        match[1] + match[2]: match[3]
        for match in line_matches
    }
    g11 = metric_components.get("g11", None)
    g22 = metric_components.get("g22", None)
    g33 = metric_components.get("g33", None)
    g12 = metric_components.get("g12", None)
    g13 = metric_components.get("g13", None)
    g23 = metric_components.get("g23", None)
    g_11 = metric_components.get("g_11", None)
    g_22 = metric_components.get("g_22", None)
    g_33 = metric_components.get("g_33", None)
    g_12 = metric_components.get("g_12", None)
    g_13 = metric_components.get("g_13", None)
    g_23 = metric_components.get("g_23", None)

    lines_to_remove = indices_of_matching_lines(pattern_setting_metric_component, lines)
    lines_removed_count = 0
    for line_index in lines_to_remove:
        del (lines[line_index - lines_removed_count])
        lines_removed_count += 1

    metric_components_with_value = {key: value for key, value in metric_components.items() if value is not None}
    for key, value in metric_components_with_value.items().__reversed__():
        new_value = replace_metric_tensor_cases(value)
        print(rf"    const auto {key} = {new_value};")
        local_variable_line = rf"    const auto {key} = {new_value};"
        lines.insert(lines_to_remove[0], local_variable_line)

    lines.insert(lines_to_remove[0] + len(metric_components_with_value), "\n")
    lines.insert(lines_to_remove[0] + len(metric_components_with_value), "\n")

    new_metric_tensor_setter = (
        f"    coord->setMetricTensor(ContravariantMetricTensor(g11, g22, g33, g12, g13, g23), \n"
        f"                           CovariantMetricTensor(g_11, g_22, g_33, g_12, g_13, g_23));")

    # last_metric_component = "g" + (metric_tensor_components[-1])[1]
    # last_component_pattern = rf"(\b.+\-\>|\.){last_metric_component}\s?\=\s?(.+)(?=;)"
    # line_index = index_of_first_matching_line(last_component_pattern, lines)

    lines.insert(lines_to_remove[0] + len(metric_components_with_value) + 1, new_metric_tensor_setter)
    # lines.insert(lines_to_remove[0] + len(metric_components_with_value) + 2, "\n")

    # modified = contents
    # modified += "\n".join(lines)
    modified = "\n".join(lines)

    modified = replace_one_line_cases(modified)

    return modified


def replace_one_line_cases(modified):
    patterns_with_replacements = {
        r"(\-\>|\.)d(\w)\s?\=\s?(.+?\b)": r"\1setD\2(\3)",  # Replace `->dx =` with `->setDx()`, etc
        r"(\b.+\-\>|\.)d(\w)\s?\/\=\s?(.+)(?=;)": r"\1setD\2(\1d\2  / (\3))",  # Replace `foo->dx /= bar` with `foo->setDx(foo->dx() / (bar))`
        r"(\-\>|\.)(d\w)\s?(?!=)": r"\1\2()",  # Replace `c->dx` with `c->dx()` but not if is assignment

        r"(\-\>|\.)Bxy\s?\=\s?(.+?\b)": r"\1setBxy(\2)",  # Replace `->Bxy =` with `->setBxy()`, etc
        r"(\b.+\-\>|\.)Bxy\s?\/\=\s?(.+)(?=;)": r"\1setBxy(\1Bxy  / \2)",  # Replace `foo->Bxy /= bar` with `foo->setBxy(foo->Bxy() / (bar))`
        r"(\-\>|\.)Bxy\s?(?!=)": r"\1Bxy()",  # Replace `c->Bxy` with `c->Bxy()` but not if is assignment

        r"(\-\>|\.)J\s?\=\s?(.+?)(?=;)": r"\1setJ(\2)",  # Replace `->J =` with `->setJ()`, etc
        r"(\b.+\-\>|\.)J\s?\/\=\s?(.+)(?=;)": r"\1setJ(\1J  / \2)",  # Replace `foo->J /= bar` with `foo->setJ(foo->J() / (bar))`
        r"(\-\>|\.)J\s?(?!=)": r"\1J()",  # Replace `c->J` with `c->J()` but not if is assignment
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


# def remove_matching_line(regex_pattern, lines):
#     search_result_for_all_lines = [re.search(regex_pattern, line) for line in lines]
#     first_matching_line = [x for x in search_result_for_all_lines if x is not None][0].string
#     lines.remove(first_matching_line)
#     return lines


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
