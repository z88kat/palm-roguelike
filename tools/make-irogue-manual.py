#!/usr/bin/env python3
"""Build iRogue-manual.pdb, iRogue/MANUAL.md as a Palm DOC e-book.

Readable on the device in any Palm DOC reader (AportisDoc, Palm Reader,
CSpotRun, ...). The Markdown is converted to plain text the same way as for
kMoria's manual; this reuses make-kmoria-manual.py for that and for the
PalmDOC writer.

Usage: make-irogue-manual.py <MANUAL.md> <output.pdb>
"""
import importlib.util
import os
import sys

sys.dont_write_bytecode = True  # no __pycache__ next to the tools
_spec = importlib.util.spec_from_file_location(
    "palmdoc", os.path.join(os.path.dirname(os.path.abspath(__file__)),
                            "make-kmoria-manual.py"))
palmdoc = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(palmdoc)


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__.strip().splitlines()[-1])
    md = open(sys.argv[1], encoding="utf-8").read()
    text = palmdoc.to_palm(palmdoc.markdown_to_text(md))
    n = palmdoc.write_pdb(sys.argv[2], "iRogue Manual", text)
    print(f"wrote {sys.argv[2]} ({len(text)} chars, {n} records)")


if __name__ == "__main__":
    main()
