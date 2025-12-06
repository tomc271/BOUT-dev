#!/usr/bin/env python3

from boutdata import collect
from boututils.run_wrapper import build_and_log, launch_safe
import numpy

datapath = "data"
nproc = 1
tol = 1.0e-13

build_and_log("twistshift test")

s, out = launch_safe("./test-twistshift", nproc=nproc, pipe=True)
with open("run.log." + str(nproc), "w") as f:
    f.write(out)

test = collect("test", path=datapath, yguards=True, info=False)
test_aligned = collect("test_aligned", path=datapath, yguards=True, info=False)
result = collect("result", path=datapath, yguards=True, info=False)

# from boututils.showdata import showdata
# showdata([test, test_aligned, result], titles=['test', 'test_aligned', 'result'])



# Check test_aligned is *not* periodic in y
def test1(ylower, yupper):
    assert not numpy.any(
            numpy.abs(test_aligned[:, yupper, :] - test_aligned[:, ylower, :]) < 1.0e-6
    ), f"Fail - test_aligned should not be periodic jy={yupper} and jy={ylower} should be different"


def test_twistshift_1():

    test1(0, -4)
    test1(1, -3)
    test1(2, -2)
    test1(3, -1)

    # Check test and result are the same
    assert not numpy.any(numpy.abs(result - test) > tol), \
        "Fail - result has not been communicated correctly - is different from input"


# Check result is periodic in y
def test2(ylower, yupper):
    yupper_ylower_different = numpy.any(numpy.abs(result[:, yupper, :] - result[:, ylower, :]) > tol)
    assert not yupper_ylower_different, f"Fail - result should be periodic jy={yupper} and jy={ylower} should not be different"
    if yupper_ylower_different:
        print(ylower, result[:, ylower, :])
        print(yupper, result[:, yupper, :])
        print(result[:, ylower, :] - result[:, yupper, :])


def test_twistshift_2():

    test2(0, -4)
    test2(1, -3)
    test2(2, -2)
    test2(3, -1)
