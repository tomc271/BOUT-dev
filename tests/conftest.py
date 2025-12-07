from pathlib import Path
import pytest
import re
from typing import Set

from boutconfig import has as bout_has

# Regex to capture any REQUIRES line, including continued ones
_RE_REQUIRES = re.compile(
    r'REQUIRES\s+([A-Z0-9_]+)',
    re.MULTILINE
)

def _get_required_features_from_cmakelists(test_file_path: Path) -> Set[str]:
    cmakelists_path = test_file_path.parent / "CMakeLists.txt"
    if not cmakelists_path.exists():
        return set()

    content = cmakelists_path.read_text(encoding="utf-8", errors="ignore")
    matches = _RE_REQUIRES.findall(content)

    # Also catch continued lines like:
    # REQUIRES BOUT_HAS_NETCDF \
    #          BOUT_HAS_FFTW
    continued = re.findall(r'REQUIRES\s+.*\\\s*\n\s*([A-Z0-9_]+)', content, re.MULTILINE)
    matches.extend(continued)

    return set(matches)


def _normalize_feature_name(cmake_name: str) -> str:
    # Remove common BOUT++ prefixes
    normalized = (cmake_name
                  .replace("BOUT_HAS_", "")
                  .replace("BOUT_USE_", "")
                  .replace("BOUT_ENABLE_", ""))
    return normalized.lower()


@pytest.fixture(autouse=True)
def skip_if_build_requirements_not_met(request):
    test_file = Path(request.fspath).resolve()
    cmakelists = test_file.parent / "CMakeLists.txt"
    if not cmakelists.exists():
        return  # Assume test should run if no CMakeLists.txt

    required_cmake_features = _get_required_features_from_cmakelists(test_file)
    if not required_cmake_features:
        return

    missing = []
    for feat in required_cmake_features:
        normalized_feature_name = _normalize_feature_name(feat)
        if not bout_has.get(normalized_feature_name, False):
            missing.append(normalized_feature_name)

    if missing:
        missing_str = ", ".join(sorted(missing))
        pytest.skip(f"Build missing required features: {missing_str}")
