#!/usr/bin/env python3
"""Boot the firmware in the headless desktop simulator and read a book.

The run fails if the firmware crashes, hangs, or does not show the expected
screens: Home, the file browser, page 1 of a generated EPUB, then page 2
after a page turn. Usage:

    pio run -e simulator
    python3 test/simulator/smoke.py .pio/build/simulator/program

The output folder keeps the simulator log and one BMP screenshot per step.
"""

import argparse
import os
import subprocess
import sys
import tempfile
from pathlib import Path

from sample_book import PAGE_ONE_WORD, PAGE_TWO_WORD, make_sample_epub

BOOK = "smoke.epub"
# Screenshots in script order; every one must be saved by this run.
SHOTS = ("1-home", "2-files", "3-page1", "4-page2")
WAIT_TIMEOUT_MS = 30000
REPO_ROOT = Path(__file__).resolve().parents[2]


def shot(out, index):
    return out / f"{SHOTS[index]}.bmp"


def input_script(out):
    # Times count from the previous WAIT, and events at the same time keep
    # their order, so a WAIT timed with its key press starts after it. A WAIT
    # that never matches ends the run with exit status 1 after WAIT_TIMEOUT_MS.
    steps = [
        "0:WAIT:Entering activity: Home",
        "500:WAIT:SCREEN:.*Browse Files",
        f"0:SHOT:{shot(out, 0)}",
        "300:ENTER",
        "300:WAIT:Entering activity: FileBrowser",
        # The browser lists the name, with or without ".epub", or the title.
        # The page text never has "smoke" as a separate word.
        "0:WAIT:SCREEN:.*([:|] smoke( \\||\\.epub|$)|Simulator Smoke)",
        f"0:SHOT:{shot(out, 1)}",
        "300:ENTER",
        f"300:WAIT:SCREEN:.*{PAGE_ONE_WORD}",
        f"0:SHOT:{shot(out, 2)}",
        "300:DOWN",
        f"300:WAIT:SCREEN:.*{PAGE_TWO_WORD}",
        f"0:SHOT:{shot(out, 3)}",
        "300:QUIT",
    ]
    script = ";".join(steps)
    if any(";" in step for step in steps):
        raise ValueError("input script paths must not contain ';'")
    return script


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("program", type=Path, help="simulator binary, e.g. .pio/build/simulator/program")
    parser.add_argument(
        "--out",
        type=Path,
        default=REPO_ROOT / "sim-artifacts",
        help="folder for the log and screenshots (default: sim-artifacts/ in the repo root)",
    )
    args = parser.parse_args()

    program = args.program.resolve()
    if not program.is_file():
        parser.error(f"simulator binary not found: {args.program} (build with: pio run -e simulator)")
    out = args.out.resolve()
    if ";" in str(out):
        parser.error("--out must not contain ';' (it separates input script steps)")
    out.mkdir(parents=True, exist_ok=True)
    # The simulator exits 0 even when a SHOT saves nothing, so stale files
    # from an earlier run must not count.
    for index in range(len(SHOTS)):
        shot(out, index).unlink(missing_ok=True)

    script = input_script(out)
    # Every WAIT may use its full timeout; the margin covers startup and delays.
    run_timeout_s = script.count(":WAIT:") * WAIT_TIMEOUT_MS // 1000 + 60
    env = {
        key: value for key, value in os.environ.items() if not key.startswith(("CROSSPOINT_SIM_", "CROSSPOINT_EMU_"))
    }
    with tempfile.TemporaryDirectory(prefix="crosspoint-sim-smoke-") as temp:
        sd = Path(temp) / "sd"
        sd.mkdir()
        make_sample_epub(sd / BOOK)
        env.update(
            SDL_VIDEODRIVER="dummy",
            # The simulator asks for an accelerated renderer, which the dummy
            # driver lacks; without one SHOT saves nothing. Naming the
            # software renderer makes SDL create it anyway.
            SDL_RENDER_DRIVER="software",
            CROSSPOINT_SIM_SD=str(sd),
            CROSSPOINT_SIM_SCREEN_TEXT="1",
            CROSSPOINT_SIM_WAIT_TIMEOUT_MS=str(WAIT_TIMEOUT_MS),
            # Avoid clashing with anything else on the default 8080/8081.
            CROSSPOINT_SIM_HTTP_PORT="18180",
            CROSSPOINT_SIM_INPUT_SCRIPT=script,
        )
        log_path = out / "simulator.log"
        with log_path.open("w") as log:
            try:
                result = subprocess.run(
                    [str(program)],
                    cwd=temp,
                    env=env,
                    stdout=log,
                    stderr=subprocess.STDOUT,
                    timeout=run_timeout_s,
                    check=False,
                )
                status = result.returncode
            except subprocess.TimeoutExpired:
                status = None
            except OSError as error:
                print(f"simulator smoke test FAILED\n  could not start {program}: {error}", file=sys.stderr)
                return 1

    log_text = log_path.read_text(errors="replace")
    # SDL creates the file before writing it, so an empty file is a failed save.
    missing = [SHOTS[i] for i in range(len(SHOTS)) if not shot(out, i).is_file() or shot(out, i).stat().st_size == 0]
    if status == 0 and not missing:
        print(f"simulator smoke test passed; artifacts in {out}")
        return 0

    print("simulator smoke test FAILED", file=sys.stderr)
    if status is None:
        print(f"  the simulator did not exit within {run_timeout_s} s", file=sys.stderr)
    else:
        print(f"  exit status {status}", file=sys.stderr)
    if missing:
        print(f"  missing screenshots: {', '.join(missing)}", file=sys.stderr)
    print("  last log lines:", file=sys.stderr)
    for line in log_text.splitlines()[-40:]:
        print(f"    {line}", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
