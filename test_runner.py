import subprocess
import pytest
from conftest import get_test_dirs


# Parametrize the test directly over the list
@pytest.mark.parametrize("test_dir", get_test_dirs())
def test_run_external(request, test_dir, requirements):
    """Run external command for each test dir."""
    # Check requirements (skip if unmet)
    # Adapt 'check' to your Requirements API; fallback example:
    req_met = True  # Replace with: requirements.check(os.path.join(test_dir, "runtest"))[0]
    if not req_met:
        pytest.skip(f"Requirements not met for {test_dir}")

    # Get config for --make
    make_mode = request.config.getoption("--make")
    cmd = "make" if make_mode else "./runtest"
    full_cmd = f"cd '{test_dir}' && timeout 600 {cmd}"  # 10m timeout; quote for paths with spaces

    # Execute and assert (pipes output)
    result = subprocess.run(full_cmd, shell=True, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Output: {result.stdout}\nError: {result.stderr}")  # For debugging; remove for clean runs
        assert result.returncode == 0, f"Test '{test_dir}' failed (code {result.returncode})"
