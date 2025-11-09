import pytest
import subprocess
import time
import os


@pytest.mark.parametrize("test_dir", ["."])  # Or dynamic discovery
def test_runtest(test_dir, tmp_path):
    os.chdir(test_dir)
    cmd = "./runtest"  # Or "make" via CLI flag
    start = time.time()
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=600)  # 10min default
    elapsed = time.time() - start
    assert result.returncode == 0, f"Failed after {elapsed:.3f}s:\n{result.stdout}\n{result.stderr}"
