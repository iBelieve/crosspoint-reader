"""Sample EPUB for simulator runs (smoke test, previews)."""

import zipfile

# Single words, because the simulator reports each drawn word separately.
PAGE_ONE_WORD = "Alphasmoke"
PAGE_TWO_WORD = "Betasmoke"


def chapter(title, word):
    return f"""<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
<head><title>{title}</title></head>
<body><h1>{title}</h1><p>{word} marks this page for the simulator smoke test.</p></body>
</html>"""


def make_sample_epub(path):
    """Two one-page chapters, so a single page turn moves to chapter 2."""
    chapters = [("One", PAGE_ONE_WORD), ("Two", PAGE_TWO_WORD)]
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as epub:
        epub.writestr("mimetype", "application/epub+zip", compress_type=zipfile.ZIP_STORED)
        epub.writestr("META-INF/container.xml", """<?xml version="1.0"?>
<container version="1.0" xmlns="urn:oasis:names:tc:opendocument:xmlns:container">
  <rootfiles><rootfile full-path="OEBPS/content.opf" media-type="application/oebps-package+xml"/></rootfiles>
</container>""")
        manifest = []
        spine = []
        for i, (title, word) in enumerate(chapters):
            manifest.append(f'<item id="c{i}" href="c{i}.xhtml" media-type="application/xhtml+xml"/>')
            spine.append(f'<itemref idref="c{i}"/>')
            epub.writestr(f"OEBPS/c{i}.xhtml", chapter(title, word))
        manifest.append('<item id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml"/>')
        points = "".join(
            f'<navPoint id="n{i}" playOrder="{i + 1}"><navLabel><text>{title}</text></navLabel>'
            f'<content src="c{i}.xhtml"/></navPoint>' for i, (title, _) in enumerate(chapters))
        epub.writestr("OEBPS/toc.ncx", f"""<?xml version="1.0" encoding="UTF-8"?>
<ncx xmlns="http://www.daisy.org/z3986/2005/ncx/" version="2005-1">
  <head><meta name="dtb:uid" content="crosspoint-simulator-smoke"/></head>
  <docTitle><text>Simulator Smoke</text></docTitle>
  <navMap>{points}</navMap>
</ncx>""")
        epub.writestr("OEBPS/content.opf", f"""<?xml version="1.0"?>
<package version="2.0" xmlns="http://www.idpf.org/2007/opf" unique-identifier="bookid">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/">
    <dc:title>Simulator Smoke</dc:title>
    <dc:language>en</dc:language>
    <dc:identifier id="bookid">crosspoint-simulator-smoke</dc:identifier>
  </metadata>
  <manifest>{"".join(manifest)}</manifest>
  <spine toc="ncx">{"".join(spine)}</spine>
</package>""")
