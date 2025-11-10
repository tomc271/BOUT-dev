import pytest
import glob
import os
import subprocess
import time

# Auto-discover test subdirs with runtest (relative to integrated/)
def pytest_generate_tests(metafunc):
    if "test_dir" in metafunc.fixturenames:
        # Find immediate subdirs with runtest (adjust glob for depth, e.g., "**/runtest" for recursive)
        test_paths = [p for p in glob.glob("*/*runtest") if os.path.isfile(p)]  # e.g., test-drift-instability/runtest
        test_dirs = [os.path.dirname(p) for p in test_paths]  # e.g., ["test-drift-instability"]
        print(f"Discovered test dirs: {test_dirs}")  # Debug: Remove later
        metafunc.parametrize("test_dir", test_dirs, indirect=True, ids=test_dirs)  # IDs = dir names for -v output

@pytest.fixture
def test_dir(request):
    return request.param  # Relative path, e.g., "test-drift-instability"

def test_runtest(test_dir, tmp_path):
    os.chdir(test_dir)  # chdir to discovered relative path
    cmd = "make" if pytest.config.getoption("make") else "./runtest"
    start = time.time()
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=600)
    elapsed = time.time() - start
    if result.returncode != 0:
        print(f"Output: {result.stdout}\nStderr: {result.stderr}")
    assert result.returncode == 0, f"Failed after {elapsed:.3f}s in {test_dir}"