def pytest_addoption(parser):
    parser.addoption("--make", action="store_true", help="Build tests instead of run")
    parser.addoption("--get-list", action="store_true", help="List runnable tests")

def pytest_collection_modifyitems(config, items):
    if config.getoption("--get-list"):
        for item in items:
            print(item.nodeid)
        raise SystemExit(0)
