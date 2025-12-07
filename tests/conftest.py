from pathlib import Path
import pytest
import re
from typing import Set

from boutconfig import has as bout_has_options

# Regex to extract REQUIRES lines from bout_add_integrated_test(...)
_RE_REQUIRES = re.compile(
    r'^\s*REQUIRES\s+([A-Z0-9_]+)',
    re.MULTILINE
)

def _get_required_features_from_cmakelists(test_file_path: Path) -> Set[str]:
    """
    Parse the companion CMakeLists.txt and return set of features
    required by the integrated test in the same directory.
    """
    cmakelists_path = test_file_path.parent / "CMakeLists.txt"
    if not cmakelists_path.exists():
        return set()  # no file → no requirements

    content = cmakelists_path.read_text(encoding="utf-8", errors="ignore")

    # Find all REQUIRES XXX lines (even if split across multiple lines)
    requires_lines = _RE_REQUIRES.findall(content)

    # Also support multi-line REQUIRES
    continued = re.findall(r'REQUIRES\s+\\?\s*\n\s*([A-Z0-9_]+)', content, re.MULTILINE)
    requires_lines.extend(continued)

    # Clean up known prefixes (BOUT++ uses BOUT_HAS_*, BOUT_USE_*, etc.)
    cleaned = {
        feat.replace("BOUT_HAS_", "").replace("BOUT_USE_", "").replace("BOUT_ENABLE_", "")
        for feat in requires_lines
    }
    return cleaned


@pytest.fixture(autouse=True)
def skip_if_build_requirements_not_met(request):
    """
    Automatically skip any integrated test whose CMakeLists.txt contains
    REQUIRES flags not satisfied by the current build.
    """
    # Only apply to BOUT++ integrated tests (you can detect by name or marker)
    if not request.node.name.startswith("test_"):
        return
    if not request.node.originalname:  # not parametrized
        test_name = request.node.name
    else:
        test_name = request.node.originalname

    # Heuristic: only check files that likely have a matching CMakeLists.txt
    test_file = Path(request.fspath).resolve()
    if not (test_file.parent / "CMakeLists.txt").exists():
        return

    required_features = _get_required_features_from_cmakelists(test_file)

    if not required_features:
        return  # no REQUIRES → always run

    missing = [
        feature for feature in required_features
        if not bout_has_options.get(feature, False)
           and not bout_has_options.get(f"BOUT_HAS_{feature}", False)
           and not bout_has_options.get(f"BOUT_USE_{feature}", False)
    ]

    if missing:
        missing_str = ", ".join(missing)
        pytest.skip(f"Build missing required features: {missing_str}")