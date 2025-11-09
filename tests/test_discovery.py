import pytest
import glob
import os
import subprocess
import time


# Dynamically find test dirs with runtest
def pytest_generate_tests(metafunc):
    if "test_dir" in metafunc.fixturenames:
        # Mimic script's glob (adjust depth if needed)
        test_paths = glob.glob("**/runtest", root_dir=".", recursive=True)
        test_dirs = [os.path.dirname(p) for p in test_paths]
        # Filter to tests/ subdirs
        test_dirs = [d for d in test_dirs if d.startswith("tests/")]
        metafunc.parametrize("test_dir", test_dirs, indirect=True)


@pytest.fixture
def test_dir(request):
    # Param value is the dir path (relative to root)
    return request.param


# The actual test: Runs ./runtest (or make) in the dir
def test_runtest(test_dir, tmp_path):  # tmp_path for any temp needs
    os.chdir(test_dir)
    cmd = "make" if pytest.config.getoption("make") else "./runtest"
    start = time.time()
    result = subprocess.run(
        cmd, shell=True, capture_output=True, text=True,
        timeout=600  # 10min; adjust via pytest-timeout plugin
    )
    elapsed = time.time() - start
    # Assert & capture output like original script
    if result.returncode != 0:
        print(f"Output: {result.stdout}\nStderr: {result.stderr}")
    assert result.returncode == 0, f"Failed after {elapsed:.3f}s in {test_dir}"
