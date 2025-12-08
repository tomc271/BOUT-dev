import pathlib


def find_in_build_directory(this_file: pathlib.Path):
    current_dir = this_file.parent.absolute()
    src_root_dir = current_dir.parent.parent.parent
    src_dir_parent = src_root_dir.parent
    matching_files = list(src_dir_parent.rglob(this_file.name))
    last_three_parts = current_dir.parts[-3:]
    path_must_contain = "/".join(last_three_parts)
    valid_matches = [f for f in matching_files if path_must_contain in str(f)]
    matches_except_src_dir = [f for f in valid_matches if f.parents[3] != src_root_dir]
    if matches_except_src_dir:
        # Select the most recently created (by creation time)
        most_recent = max(matches_except_src_dir, key=lambda p: p.stat().st_ctime)
        return most_recent.parent
    else:
        raise FileNotFoundError(f"Test {this_file.name} not found in build directory ({src_dir_parent}).")
