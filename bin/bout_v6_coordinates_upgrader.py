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
    g11_pattern = r"(\b.+\-\>|\.)g11\s?\=\s?(.+)(?=;)"
    first_g11_match = re.search(g11_pattern, contents)
    # TODO:
    # g11_matches = re.findall(g11_pattern, contents)
    # if len(g11_matches) > 1:
    regex_2 = r"(\b.+\-\>|\.)g(\d\d)\s?\=\s?(.+)(?=;)"
    remaining_metric_components = re.findall(regex_2, contents[first_g11_match.end():])
    metric_components = {
        "g" + component[1]: component[2]
        for component in remaining_metric_components
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
    
    modified = contents
    lines = contents.splitlines()

    new_metric_setter = (
        f"    coord->setMetricTensor(ContravariantMetricTensor({g11}, {g22}, {g33}, {g12}, {g13}, {g23}), "
        f"CovariantMetricTensor({g_11}, {g_22}, {g_33}, {g_12}, {g_13}, {g_23}));")

    metric_components_with_value = [c for c in metric_components if c is not None]
    last_metric_component_with_value = metric_components_with_value[-1]
    last_component_pattern = rf"(\b.+\-\>|\.){last_metric_component_with_value}\s?\=\s?(.+)(?=;)"
    line_index = index_of_first_matching_line(last_component_pattern, lines)
    lines.insert(line_index, new_metric_setter)
    modified += "\n".join(lines)

    patterns_with_replacements = {
        r"(\-\>|\.)d(\w)\s?\=\s?(.+?\b)": r"\1setD\2(\3)",  # Replace `->dx =` with `->setDx()`, etc
        r"(\b.+\-\>|\.)d(\w)\s?\/\=\s?(.+)(?=;)": r"\1setD\2(\1d\2  / (\3))",
        # Replace `foo->dx /= bar` with `foo->setDx(foo->dx() / (bar))`
        r"(\-\>|\.)(d\w)\s?(?!=)": r"\1\2()",  # Replace `c->dx` with `c->dx()` but not if is assignment

        r"(\-\>|\.)Bxy\s?\=\s?(.+?\b)": r"\1setBxy(\2)",  # Replace `->Bxy =` with `->setBxy()`, etc
        r"(\b.+\-\>|\.)Bxy\s?\/\=\s?(.+)(?=;)": r"\1setBxy(\1Bxy  / \2)",
        # Replace `foo->Bxy /= bar` with `foo->setBxy(foo->Bxy() / (bar))`
        r"(\-\>|\.)Bxy\s?(?!=)": r"\1Bxy()",  # Replace `c->Bxy` with `c->Bxy()` but not if is assignment
    }
    for pattern, replacement in patterns_with_replacements.items():
        modified = re.sub(pattern, replacement, modified)

    return modified


def index_of_first_matching_line(last_component_pattern, lines):
    search_result_for_all_lines = [re.search(last_component_pattern, line) for line in lines]
    first_matching_line = [x for x in search_result_for_all_lines if x is not None][0].string
    # first_matching_line = [re.findall(last_component_pattern, line) for line in lines][0]
    return lines.index(first_matching_line)


if __name__ == "__main__":
    main()
