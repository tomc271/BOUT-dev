import os
import pytest
import shutil
import boutpp
from pathlib import Path


def pytest_configure(config):
    config.addinivalue_line(
        "markers", "input_dir(name): specify the pre-existing input directory name for this test"
    )

@pytest.fixture
def test_dir(request) -> Path:
    return Path(request.fspath).parent

@pytest.fixture(scope="function", autouse=True)
def copy_and_cwd_to_unique_tmp_dir(request, tmp_path_factory, monkeypatch):
    """
    For each test function, create a unique temporary copy of the test directory
    and change cwd to it.
    """

    test_file_dir = Path(request.fspath).parent

    if not test_file_dir.is_dir():
        pytest.fail(f"Expected test directory '{test_file_dir}' not found")

    # Create a unique temp dir for this test
    run_dir = tmp_path_factory.mktemp(test_file_dir.name)

    # Copy the original test directory into it
    shutil.copytree(test_file_dir, run_dir, dirs_exist_ok=True)

    # Change working directory to the copy
    monkeypatch.chdir(run_dir)

import subprocess

def patched_launch(command, nproc=1, pipe=True, mthread=1, verbose=False, **kwargs):
    # Replicate original behavior: prepend mpirun for nproc > 1
    if nproc > 1:
        full_command = f"mpirun -np {nproc} {command}"
    else:
        full_command = command

    result = subprocess.run(
        full_command,
        shell=True,
        capture_output=pipe,
        text=True,
        **kwargs
    )
    out = result.stdout if pipe else ''
    if result.stderr:
        out += '\nSTDERR:\n' + result.stderr
    return result.returncode, out

def patched_launch_safe(command, *args, **kwargs):
    s, out = patched_launch(command, *args, **kwargs)
    if s != 0:
        raise RuntimeError(
            f"Run failed with {s}.\nCommand was:\n{command}\n\nOutput was\n\n{out}"
        )
    return s, out

@pytest.fixture(autouse=True)
def replace_launch_functions(monkeypatch):
    monkeypatch.setattr("boututils.run_wrapper.launch", patched_launch)
    monkeypatch.setattr("boututils.run_wrapper.launch_safe", patched_launch_safe)
