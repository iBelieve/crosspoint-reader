#!/usr/bin/env python3
"""Assemble the browser simulator into a static site (PR previews).

    pio run -e simulator_web
    python3 test/simulator/web_site.py .pio/build/simulator_web sim-site --label "PR #12 · abc1234"

The site holds the page, the wasm build, a sample book copied onto fresh SD
cards (files.json), the COOP/COEP headers threads need (_headers, read by
Cloudflare Workers static assets), and build.json with the label shown in the
page header.
"""

import argparse
import json
import shutil
import sys
from pathlib import Path

from sample_book import make_sample_epub

BOOKS = ["books/sample.epub"]
# Marks a folder this script made, so only such a folder is ever replaced.
MARKER = ".crosspoint-sim-site"

HEADERS = """/*
  Cross-Origin-Opener-Policy: same-origin
  Cross-Origin-Embedder-Policy: require-corp
  Cross-Origin-Resource-Policy: same-origin
/program.wasm
  Cache-Control: no-cache
/program.js
  Cache-Control: no-cache
"""


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("build", type=Path, help="build folder, e.g. .pio/build/simulator_web")
    parser.add_argument("out", type=Path, help="new site folder (one this script made is replaced)")
    parser.add_argument("--label", default="", help="text shown in the page header")
    parser.add_argument("--url", default="", help="link for the label, e.g. the PR")
    args = parser.parse_args()

    sources = {"index.html": "program.html", "program.js": "program.js", "program.wasm": "program.wasm"}
    missing = [name for name in sources.values() if not (args.build / name).is_file()]
    if missing:
        parser.error(f"missing in {args.build}: {', '.join(missing)} (build with: pio run -e simulator_web)")

    out = args.out.resolve()
    if out.exists() and any(out.iterdir()):
        if not (out / MARKER).is_file():
            parser.error(f"{args.out} is not empty and was not made by this script; pick a new folder")
        shutil.rmtree(out)
    out.mkdir(parents=True, exist_ok=True)
    (out / MARKER).touch()
    for dest, src in sources.items():
        shutil.copyfile(args.build / src, args.out / dest)

    (args.out / "books").mkdir()
    make_sample_epub(args.out / BOOKS[0])
    (args.out / "files.json").write_text(json.dumps(BOOKS) + "\n")
    (args.out / "_headers").write_text(HEADERS)
    (args.out / "build.json").write_text(json.dumps({"label": args.label, "url": args.url}) + "\n")

    wasm_bytes = (args.out / "program.wasm").stat().st_size
    print(f"site in {args.out}; program.wasm is {wasm_bytes / 1e6:.1f} MB")
    return 0


if __name__ == "__main__":
    sys.exit(main())
