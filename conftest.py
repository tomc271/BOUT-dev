import os
import glob
import pytest


class TestDir(pytest.Directory):
    def collect(self):
        # Scan recursively within this directory for 'runtest' files
        runtest_paths = glob.glob(str(self.path / "**/runtest"), recursive=True)
        for path in runtest_paths:
            dir_path = os.path.dirname(path)
            name = os.path.basename(dir_path)
            yield TestDirItem.from_parent(
                parent=self,
                name=name,
                dir_path=dir_path
            )


class TestDirItem(pytest.Item):
    def __init__(self, name, parent, dir_path):
        super().__init__(name, parent)
        self.dir_path = dir_path

    @classmethod
    def from_parent(cls, parent, *, name, dir_path):
        item = cls(name, parent)
        item.dir_path = dir_path
        return item

    def runtest(self):
        # Placeholder: Run the 'runtest' script
        # import subprocess
        # result = subprocess.run(['./runtest'], cwd=self.dir_path, capture_output=True, text=True)
        # if result.returncode != 0:
        #     pytest.fail(f"runtest failed in {self.dir_path}:\n{result.stderr}")
        pass

    def repr_failure(self, excinfo, style=None):
        return f"Failed to run test in {self.dir_path}"

    def repr_short(self):
        return f"runtest in {self.dir_path}"


# Hook: Create TestDir only if 'runtest' exists in the directory itself
def pytest_collect_directory(path, parent):
    if path.is_dir() and (path / 'runtest').exists():
        return TestDir.from_parent(parent, name=str(path))
    return None