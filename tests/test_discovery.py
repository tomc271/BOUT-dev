import pytest
import glob
import os
import subprocess
import time


# Auto-discover test subdirs with runtest (recursive from tests/)
def pytest_generate_tests(metafunc):
    if "test_dir" in metafunc.fixturenames:
        # Recursive glob to find all (e.g., integrated/test-drift-instability/runtest, unit/runtest)
        test_paths = [p for p in glob.glob("**/*runtest", root_dir=".", recursive=True) if os.path.isfile(p)]
        test_dirs = [os.path.dirname(p) for p in
                     test_paths]  # Relative paths, e.g., ["integrated/test-drift-instability", "unit"]
        print(f"Discovered test dirs: {test_dirs}")  # Debug: Remove later
        test_dirs = ['integrated/test-drift-instability']
        metafunc.parametrize("test_dir", test_dirs, indirect=True, ids=test_dirs)  # IDs for -v output


@pytest.fixture
def test_dir(request):
    return request.param  # Relative path, e.g., "integrated/test-drift-instability"


@pytest.fixture(scope="function")
def make_cmd(request):
    """Fixture for --make flag access."""
    return "make" if request.config.getoption("--make") else "./runtest"


def test_runtest(test_dir, tmp_path, make_cmd):
    # Set BOUT_TOP for makefiles (from tests/ to BOUT-dev root)
    bout_root = os.path.abspath(
        "..")  # Adjust if running from root: os.path.abspath(os.path.join(test_dir, "../../../"))
    os.environ["BOUT_TOP"] = bout_root

    os.chdir(test_dir)  # chdir to discovered relative path
    cmd = make_cmd  # Use fixture for CLI flag
    start = time.time()
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=600)
    elapsed = time.time() - start
    if result.returncode != 0:
        print(f"Output: {result.stdout}\nStderr: {result.stderr}")
    assert result.returncode == 0, f"Failed after {elapsed:.3f}s in {test_dir}"