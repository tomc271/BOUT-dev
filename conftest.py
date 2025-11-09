import os
import glob
import pytest


# Session-scoped fixture: Compute test directories ONCE
@pytest.fixture(scope="session")
def test_dirs():
    """Glob for test directories once, at session start."""
    runtest_paths = glob.glob("**/runtest", recursive=True)
    return [os.path.dirname(path) for path in runtest_paths]


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
        from your_project import Requirements
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