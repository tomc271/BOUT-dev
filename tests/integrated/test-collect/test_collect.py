#!/usr/bin/env python3
import os
import pathlib

import pytest
import numpy as np
from boututils.run_wrapper import shell_safe

from tests.utils import find_in_build_directory


def test_collect():
    # This prevents pytest from recursing into boutdata during collection
    collect = pytest.importorskip("boutdata").collect

    this_file = pathlib.Path(__file__)
    try:
        executable_location = find_in_build_directory(this_file)
        this_directory = pathlib.Path(__file__).parent.absolute()
        os.chdir(this_directory)

        shell_safe("make > make.log")
        shell_safe(f"{executable_location}/test-collect")

        # Try collecting data using incorrect case
        # This should be corrected automatically
        a = collect("A", path="data")

        assert np.allclose(a, 1.23), f"Expected 1.23, got {a}"
        print("Passed")

    except FileNotFoundError as e:
        pytest.skip(reason=str(e))
