from pathlib import Path
import pytest

def _is_zeff_enabled_via_cmake(test_file_path: Path) -> bool:
    """
    Return True if Zeff tests should run for the test located at `test_file_path`.
    Looks for the companion CMakeLists.txt in the same directory.
    """
    cmakelists = test_file_path.parent / "CMakeLists.txt"
    if not cmakelists.exists():
        return True                                 # no file → assume enabled

    content = cmakelists.read_text(encoding="utf-8", errors="ignore")

    if "ENABLE_ZEFF_TESTS" in content:              # example flag
        return content.contains("ON") or "TRUE" in content
    if "set(ENABLE_ZEFF_TESTS ON)" in content:
        return True
    if "add_test(zeff" in content.lower():
        return True

    # default when nothing explicit
    return False


@pytest.fixture(autouse=True)
def skip_zeff_if_disabled_in_cmake(request):
    """
    Automatically skips any test marked with @pytest.mark.zeff_test
    if the companion CMakeLists.txt says they are disabled.
    """
    if not request.node.get_closest_marker("zeff_test"):
        return                                 # not a zeff test → do nothing

    # __file__ of the actual test module
    test_file_path = Path(request.fspath).resolve()

    if not _is_zeff_enabled_via_cmake(test_file_path):
        pytest.skip("Zeff tests are disabled in the companion CMakeLists.txt")
