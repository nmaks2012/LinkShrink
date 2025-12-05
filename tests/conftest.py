import pathlib
import pytest
from testsuite.databases.pgsql import discover

pytest_plugins = ['pytest_userver.plugins.postgresql']

@pytest.fixture(scope='session')
def service_source_dir():
    """Path to root directory service."""
    return pathlib.Path(__file__).parent.parent

@pytest.fixture(scope='session')
def build_dir(service_source_dir):
    """Path to build directory."""
    return service_source_dir / 'build/debug'

@pytest.fixture(scope='session')
def service_binary(build_dir):
    """Path to service binary."""
    return build_dir / 'linkshrink-app'

@pytest.fixture(scope='session')
def service_config_path(service_source_dir):
    """Path to static_config.yaml."""
    return service_source_dir / 'configs/static_config.yaml'

@pytest.fixture(scope='session')
def service_config_vars_path(service_source_dir):
    """Path to config_vars.yaml."""
    return service_source_dir / 'configs/config_vars.testing.yaml'

@pytest.fixture(scope='session')
def initial_data_path(service_source_dir):
    """Path for find files with data"""
    return [
        service_source_dir / 'postgresql/data',
    ]

@pytest.fixture(scope='session')
def pgsql_local(service_source_dir, pgsql_local_create):
    """Create schemas databases for tests"""
    databases = discover.find_schemas(
        'linkshrink-app',
        [service_source_dir.joinpath('postgresql/schemas')],
    )
    return pgsql_local_create(list(databases.values()))