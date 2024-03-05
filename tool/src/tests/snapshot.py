from io import StringIO
from pathlib import Path
from typing import Any


def dump_result_to(output: StringIO, value: Any) -> None:
    if isinstance(value, list):
        output.write('[\n')
        for item in value:
            dump_result_to(output, item)
            output.write(',\n')
        output.write(']')
    else:
        output.write(repr(value))


class Snapshot:
    def __init__(self, snapshots_dir: Path, nodeid: str) -> None:
        self.expected_path = snapshots_dir / f'{nodeid}.expected.txt'
        self.actual_path = snapshots_dir / f'{nodeid}.actual.txt'

    def __eq__(self, value: object) -> bool:
        expected = ''
        if self.expected_path.exists():
            expected = self.expected_path.read_text()

        actual_io = StringIO()

        dump_result_to(actual_io, value)

        actual = actual_io.getvalue()

        if actual != expected:
            self.actual_path.parent.mkdir(parents=True, exist_ok=True)
            self.actual_path.write_text(actual)
            return False
        else:
            if self.actual_path.exists():
                self.actual_path.unlink()

            return True
