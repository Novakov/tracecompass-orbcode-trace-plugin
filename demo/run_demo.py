import argparse
import subprocess
from pathlib import Path
from time import sleep


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument('--orbuculum', type=Path, required=True)
    parser.add_argument('--orbdump', type=Path, required=True)
    parser.add_argument('--orbtrace', type=Path, required=True)
    parser.add_argument('--openocd', type=Path, required=True)
    parser.add_argument('--openocd-config', type=Path, required=True, nargs='+')
    parser.add_argument('--trace-file', type=Path, required=True)
    return parser.parse_args()


def main(args: argparse.Namespace) -> None:
    def run_openocd(commands: list[str]) -> None:
        ocd_args = [args.openocd]

        for config in args.openocd_config:
            ocd_args.extend(['-f', config])

        for cmd in commands:
            ocd_args.extend(['-c', cmd])

        ocd_args.extend(['-c', 'exit'])

        subprocess.run(ocd_args, check=True)

    print('Stopping target')
    run_openocd(['init', 'reset halt'])

    sleep(4)

    # Set 4-bit trace mode
    subprocess.run([args.orbtrace, '-T', '4'], check=True)

    orbuculum = subprocess.Popen([args.orbuculum, '-t', '1', '-m', '1000', '-v', '0', '--no-colour'], stdin=subprocess.PIPE)
    try:
        sleep(1)
        print('Starting orbdump')
        orbdump = subprocess.Popen([args.orbdump, '-o', args.trace_file, '-l', '10000'], stdin=subprocess.PIPE)
        sleep(4)
        print('Resuming target')
        run_openocd(['init', 'reset'])
        print('Running')
        orbdump.wait()
    finally:
        orbuculum.kill()

    print('Capture complete')


main(parse_args())