#!/usr/bin/env python3

#
# Run the test, compare results against the benchmark
#

# requires: not metric_3d
# Requires: netcdf
# Cores: 4

# Variables to compare
from __future__ import print_function

import pytest

try:
    from builtins import str
except:
    pass

tol = 1e-7  # Absolute tolerance, benchmark values are floats

from boututils.run_wrapper import build_and_log, shell, launch_safe
from boutdata.collect import collect
import numpy as np
from sys import stdout


build_and_log("Gyro-average inversion test")


@pytest.mark.parametrize("var", ["pade1", "pade2"])
def test_gyro(var):

    # Read benchmark values
    print("Reading benchmark data")
    bmk = {}
    bmk[var] = collect(var, path="data", prefix="benchmark", info=False, xguards=False)

    print("Running Gyro-average inversion test")

    for nproc in [1, 2, 4]:
        nxpe = 1
        if nproc > 2:
            nxpe = 2

        cmd = "./test_gyro NXPE=" + str(nxpe)

        shell("rm data/BOUT.dmp.*.nc")

        print("   %d processors (nxpe = %d)...." % (nproc, nxpe))
        s, out = launch_safe(cmd, nproc=nproc, pipe=True)
        with open("run.log." + str(nproc), "w") as f:
            f.write(out)

        # Collect output data
        stdout.write("      Checking variable " + var + " ... ")
        result = collect(var, path="data", info=False, xguards=False)
        # Compare benchmark and output
        if np.shape(bmk[var]) != np.shape(result):
            print("Fail, wrong shape")
            diff = np.max(np.abs(bmk[var] - result))
            assert diff <= tol, f"Fail, maximum difference = {diff}"
