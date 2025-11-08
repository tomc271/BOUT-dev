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
            yield TestDirItem.from_parent(parent=self, name=os.path.basename(dir_path), dir_path=dir_path)


class TestDirItem(pytest.Item):
    def __init__(self, name, parent, **kwargs):
        super().__init__(name, parent, **kwargs)
        self.dir_path = kwargs.get("dir_path")

    @classmethod
    def from_parent(cls, parent, **kwargs):
        item = super().from_parent(parent, **kwargs)
        item.dir_path = kwargs.get("dir_path")
        return item

    def runtest(self):
        # This will be overridden by the actual test function; here we just collect
        pass

    def repr_failure(self, excinfo, style=None):
        return "Failed to run test in %s" % self.dir_path


# Hook to use custom collector
def pytest_collect_directory(path, parent):
    if path.is_dir() and path.joinpath('runtest').exists():
        return TestDir.from_parent(parent, path=path)
    return None