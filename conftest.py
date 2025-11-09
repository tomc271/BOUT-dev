import pytest
import glob
import os


# Custom collector for test dirs with runtest
class TestDirCollector(pytest.Collector):
    def collect(self):
        # Discover test dirs like the original script
        test_paths = glob.glob("**/runtest", recursive=True)
        for path in test_paths:
            test_dir = os.path.dirname(path)  # Dir containing runtest
            if test_dir.startswith("tests/"):  # Scope to tests/ if needed
                # Create a parametrized test item (adjust as needed)
                item = pytest.Item.from_parent(self, name=test_dir)
                item._nodeid = f"{test_dir}::test_runtest[{test_dir}]"
                yield item


# Hook to use the collector during collection
def pytest_collect_file(path, parent):
    if path.name == "conftest.py" or not path.isdir():
        return None
    # For dirs under tests/, use our collector
    if str(path).startswith("tests/"):
        return TestDirCollector.from_parent(parent, name=path.name)
    return None


# Optional: Add CLI options (from earlier suggestions)
def pytest_addoption(parser):
    parser.addoption("--get-list", action="store_true", help="List runnable tests")
    parser.addoption("--make", action="store_true", help="Build tests instead of run")


# For --get-list
def pytest_collection_modifyitems(config, items):
    if config.getoption("--get-list"):
        for item in items:
            print(item.nodeid)
        config.hook.pytest_sessionfinish(session=config, exitstatus=0)
        raise SystemExit(0)
