#!/usr/bin/env python3
import pytest
# requires: fftw
# cores: 4

from boututils.run_wrapper import build_and_log, shell, launch_safe
from boutdata.collect import collect
import numpy as np
from sys import stdout

tol = 1e-10  # Absolute tolerance


build_and_log("Delp2 operator test")

# The command to run
exefile = "./test_delp2"

# List of settings to apply
settings = [
    "MXG=2 mesh:nx=36 diffusion:useFFT=true",
    "MXG=1 mesh:nx=34 diffusion:useFFT=true",
    "MXG=2 mesh:nx=36 diffusion:useFFT=false",
    "MXG=1 mesh:nx=34 diffusion:useFFT=false",
]


def test_delp2():

    for i, opts in enumerate(settings):
        # Read benchmark values
        print("Args: " + opts)
        cmd = exefile + " " + opts

        s, out = launch_safe(cmd, nproc=1, pipe=True)
        with open("run.log." + str(i) + ".1", "w") as f:
            f.write(out)

        n0 = collect("n", path="data", info=False)

        for nproc in [2, 4]:
            shell("rm data/BOUT.dmp.*.nc")

            stdout.write("   %d processor...." % (nproc))
            s, out = launch_safe(cmd, nproc=nproc, mthread=1, pipe=True)
            with open("run.log." + str(i) + "." + str(nproc), "w") as f:
                f.write(out)

            # Collect output data
            n = collect("n", path="data", info=False)
            if np.shape(n) != np.shape(n0):
                pytest.fail("Fail, wrong shape")
            diff = np.max(np.abs(n - n0))
            assert diff <= tol, f"Fail, maximum difference = " + str(diff)
