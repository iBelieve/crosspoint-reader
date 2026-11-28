"""Tests for web_site.py: run with
python3 -m unittest discover -s test/simulator -p 'test_web_site.py'"""

import contextlib
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

import web_site


class WebSiteTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.build = self.root / "build"
        self.build.mkdir()
        for name in ("program.html", "program.js", "program.wasm"):
            (self.build / name).write_text(name)

    def tearDown(self):
        self.temp.cleanup()

    def run_main(self, *args):
        argv = ["web_site.py", str(self.build), *map(str, args)]
        with mock.patch.object(sys, "argv", argv), contextlib.redirect_stdout(io.StringIO()), \
                contextlib.redirect_stderr(io.StringIO()):
            return web_site.main()

    def test_builds_a_site(self):
        out = self.root / "site"
        self.assertEqual(self.run_main(out, "--label", "PR #1"), 0)
        self.assertEqual((out / "index.html").read_text(), "program.html")
        self.assertTrue((out / "books" / "sample.epub").is_file())
        self.assertEqual(json.loads((out / "files.json").read_text()), web_site.BOOKS)
        self.assertIn("Cross-Origin-Embedder-Policy: require-corp", (out / "_headers").read_text())
        self.assertEqual(json.loads((out / "build.json").read_text())["label"], "PR #1")

    def test_replaces_a_site_it_made(self):
        out = self.root / "site"
        self.run_main(out)
        (out / "stale.txt").write_text("old")
        self.assertEqual(self.run_main(out), 0)
        self.assertFalse((out / "stale.txt").exists())

    def test_refuses_a_folder_it_did_not_make(self):
        out = self.root / "mine"
        out.mkdir()
        (out / "keep.txt").write_text("keep")
        with self.assertRaises(SystemExit):
            self.run_main(out)
        self.assertEqual((out / "keep.txt").read_text(), "keep")


if __name__ == "__main__":
    unittest.main()
