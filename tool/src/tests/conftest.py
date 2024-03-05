from pathlib import Path
from typing import Any, Optional
import pytest
import _pytest

from .snapshot import Snapshot


def pytest_assertrepr_compare(op: str, left: Any, right: Any) -> Optional[list[str]]:
    del left
    if op == '==' and isinstance(right, Snapshot):
        return [
            f'Actual values does not match expected snapshot in {right.expected_path}',
        ]

    return None


@pytest.fixture()
def snapshot(request: pytest.FixtureRequest, pytestconfig: _pytest.config.Config) -> Snapshot:
    test_id = request.node.location[2]

    snapshots_dir = (Path(pytestconfig.rootpath) / Path(request.node.location[0])).parent / '__snapshots__'

    return Snapshot(snapshots_dir, test_id)
