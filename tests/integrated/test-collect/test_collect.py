#!/usr/bin/env python3
import pytest
import numpy as np
from boututils.run_wrapper import shell_safe

def test_collect():
    # This prevents pytest from recursing into boutdata during collection
    collect = pytest.importorskip("boutdata").collect

    shell_safe("make > make.log")
    shell_safe("./test-collect")

    # Try collecting data using incorrect case
    # This should be corrected automatically
    a = collect("A", path="data")

    assert np.allclose(a, 1.23), f"Expected 1.23, got {a}"
    print("Passed")
