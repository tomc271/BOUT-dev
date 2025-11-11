import pytest
import glob
import os
import subprocess
import time


# Auto-discover test subdirectories containing runtest file (relative from tests/)
def pytest_generate_tests(metafunc):
    if "test_dir" in metafunc.fixturenames:
        # Glob relative to tests/ directory
        test_paths = [p for p in glob.glob("**/*runtest", root_dir=".", recursive=True) if os.path.isfile(p)]
        test_dirs = [os.path.dirname(p) for p in test_paths]
        # Filter valid existing dirs
        test_dirs = [d for d in test_dirs if d and os.path.exists(d)]
        # Strip leading 'tests/' prefix if present
        test_names = [d.replace('tests/', '', 1) for d in test_dirs]
        metafunc.parametrize("test_dir", test_names, indirect=True, ids=test_names)


@pytest.fixture
def test_dir(request):
    return request.param  # Relative path, e.g., "integrated/test-drift-instability"


@pytest.fixture(scope="function")
def make_cmd(request):
    """Fixture for --make flag access."""
    return "make" if request.config.getoption("make") else "./runtest"


@pytest.fixture(autouse=True, scope="function")
def initial_cwd():
    """Auto-reset CWD to tests/ directory before each test."""
    start_dir = os.path.dirname(os.path.abspath(__file__))  # Always tests/
    original_cwd = os.getcwd()
    os.chdir(start_dir)
    print(f"Reset CWD to: {start_dir}")
    yield
    os.chdir(original_cwd)  # Restore after (cleanup)


def test_runtest(test_dir, tmp_path, make_cmd, initial_cwd):
    # BOUT_TOP from script dir (CWD-independent)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    bout_root = os.path.abspath(os.path.join(script_dir, ".."))
    os.environ["BOUT_TOP"] = bout_root

    # Add pylib (dependencies like boutdata) to PYTHONPATH
    pylib_path = os.path.join(bout_root, "tools", "pylib")
    if os.path.exists(pylib_path):
        os.environ["PYTHONPATH"] = f"{pylib_path}:{os.environ.get('PYTHONPATH', '')}"

    # Pre-build for dependencies like grid.fci.nc
    print(f"Pre-building in {test_dir}")  # Debug
    build_result = subprocess.run("make", shell=True, cwd=test_dir, capture_output=True, text=True, timeout=60)
    if build_result.returncode != 0:
        print(f"Build stderr in {test_dir}: {build_result.stderr}")  # Non-fatal; some tests no-op

    print(f"Chdir to relative: {test_dir} (abs: {os.path.abspath(test_dir)})")
    os.chdir(test_dir)

    # MPI oversubscribe for communications test
    cmd = make_cmd
    if "communications" in test_dir:
        os.environ["OMPI_MCA_rmaps_base_oversubscribe"] = "1"  # Allows 18 procs
    start = time.time()
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=600)
    elapsed = time.time() - start
    if result.returncode != 0:
        print(f"Output: {result.stdout}\nStderr: {result.stderr}")
    assert result.returncode == 0, f"Failed after {elapsed:.3f}s in {test_dir}"
