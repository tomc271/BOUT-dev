#!/usr/bin/env python3
import pathlib
import sys

import pytest
# Test enabling/disabling exception backtrace from environment variable

# requires all_tests

from boututils.run_wrapper import shell
import os

project_root = pathlib.Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(project_root))
from tests.utils import find_in_build_directory


def test_backtrace():
    try:
        del os.environ["BOUT_SHOW_BACKTRACE"]
    except KeyError:
        pass

    success = True

    this_file = pathlib.Path(__file__)
    executable_location = find_in_build_directory(this_file)
    executable_path = executable_location / "boutexcept"

    this_directory = pathlib.Path(__file__).parent.absolute()
    os.chdir(this_directory)

    s, output = shell([f"{executable_path}"], pipe=True)

    if "troublemaker" in output:
        success = False
        pytest.fail("Fail: detected offending function name in output when not expected")

    _, output = shell(["BOUT_SHOW_BACKTRACE=yes ./boutexcept"], pipe=True)

    if "troublemaker" not in output:
        success = False
        print("Fail: did not detect offending function name in output when expected")
        print(output)

    if success:
        print("=> All BoutException backtrace tests passed")
        exit(0)
    assert success, f"Test failed"

    print("=> Some failed tests")
    exit(1)
