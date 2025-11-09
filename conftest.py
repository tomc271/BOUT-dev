import os
import glob
import pytest


# Function to get test dirs (called at collection time)
def get_test_dirs():
    """Return list of test directories with runtest."""
    runtest_paths = glob.glob("**/runtest", recursive=True)
    return [os.path.dirname(p) for p in runtest_paths]


# Custom CLI options
def pytest_addoption(parser):
    parser.addoption("--make", action="store_true", help="Build instead of run")
    parser.addoption("--all", action="store_true", help="Include all tests")
    parser.addoption("--set-bool", action="append", help="Set bool: key=value")


@pytest.fixture(scope="session")
def requirements(pytestconfig):
    """Mimic Requirements; adapt to your module."""
    # Assuming you have 'your_project.requirements' – replace as needed
    try:
        from your_project import Requirements  # e.g., from BOUT-dev/requirements.py
        reqs = Requirements()
    except ImportError:
        # Fallback simple dict if no module
        reqs = {}

    if pytestconfig.getoption("--set-bool"):
        lookup = {"false": False, "no": False, "true": True, "yes": True}
        for arg in pytestconfig.getoption("--set-bool"):
            k, v = arg.split("=")
            reqs[k] = lookup[v.lower()]
    reqs["make"] = pytestconfig.getoption("--make")
    reqs["all_tests"] = pytestconfig.getoption("--all")
    return reqs
