import pytest

# CLI options (from earlier)
def pytest_addoption(parser):
    parser.addoption("--get-list", action="store_true", help="List runnable tests")
    parser.addoption("--make", action="store_true", help="Build tests instead of run")

# For --get-list: Print and exit after collection
def pytest_collection_modifyitems(config, items):
    if config.getoption("--get-list"):
        for item in items:
            print(item.nodeid)
        config.hook.pytest_sessionfinish(session=config, exitstatus=0)
        raise SystemExit(0)