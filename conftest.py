import os
import glob
import pytest


class TestDir(pytest.Directory):
    def collect(self):
        # Scan for directories with 'runtest' (like the script's glob)
        runtest_paths = glob.glob("**/runtest", recursive=True)
        test_dirs = [os.path.dirname(path) for path in runtest_paths]
        for dir_path in test_dirs:
            # Yield a single parametrized test per directory
            yield TestDirItem(dir_path, self)


class TestDirItem(pytest.Item):
    def __init__(self, dir_path, parent):
        super().__init__(os.path.basename(dir_path), parent)
        self.dir_path = dir_path

    def runtest(self):
        # This will be overridden by the actual test function; here we just collect
        pass

    def repr_failure(self, excinfo, style=None):
        return "Failed to run test in %s" % self.dir_path


# Hook to use custom collector
def pytest_collect_directory(path, parent):
    if path.is_dir() and path.joinpath('runtest').exists():
        return TestDir(str(path), parent)