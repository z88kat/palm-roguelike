#!/usr/bin/env python3
"""Generate kMoriaColorDB.pdb, the colour scheme kMoria reads at startup.

kMoria only offers colour (Set > Preferences > Display > Color) when a
database named "kMoriaColorDB" exists. Originally players built it on the
device with kMoriaUtil. This script builds an equivalent default scheme on
the host, so colour works out of the box.

Layout (see kMoria/src/color.c and kMoriaUtil/main.c):
  record 0  REC_CLIST_COLORS   MAX_CREATURES + 95 + 11 bytes, each an index
                               into the Palm OS 8-bit system palette:
                                 [0, 279)    one per monster
                                 [279, 374)  one per ASCII symbol ' '..'~'
                                 [374, 385)  kMoriaUtil's named colours
  record 1  REC_COLOR_VERSION  UInt16 version (1)

Monsters get the colour of their map letter, then a colour word in their
name ("Red", "Grape", ...) overrides it, the same rules as kMoriaUtil's
"Things->Creatures" and "Colors->Creatures" buttons.

The scheme is tuned for the default white background. Pure black and white
are never used, so kMoria's black/white swap for the "Invert" background
never applies; it stays readable there, if a little dark.

Usage: make-kmoria-colors.py <kMoria/src/BuildDB> <output.pdb>
"""
import re
import struct
import sys
import time

MAX_CREATURES = 279
MAX_ASCII = 95
COLOR_NAMES = ["Black", "Blue", "Brown", "Clear", "Green", "Grey", "Purple",
               "Red", "Silver", "White", "Yellow"]
SYNONYMS = [("Grape", "Purple"), ("Violet", "Purple"), ("Emperor", "Purple"),
            ("Bloodshot", "Red"), ("Crimson", "Red"), ("Crystal", "Clear"),
            ("Fire", "Red")]


def palm_index(rgb):
    """Index of an RGB colour in the Palm OS 8-bit system palette.

    The first 216 entries are the 6x6x6 web-safe cube (levels FF..00 in steps
    of 33), split in two halves by blue: entries 0-107 have blue FF/CC/99 and
    108-215 have blue 66/33/00. Within a half, red is the outer loop, then
    blue, then green. (Read from the Tungsten C ROM's palette table; entry 215
    is 111111 rather than black, which lives at 230+.)
    """
    r, g, b = ((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF)
    for c in (r, g, b):
        if c % 0x33:
            raise ValueError(f"{rgb:06X} is not a palette-cube colour")
    if rgb == 0:
        raise ValueError("use 333333; pure black is not in the cube")
    ri, gi, bi = ((0xFF - c) // 0x33 for c in (r, g, b))
    return (bi // 3) * 108 + ri * 18 + (bi % 3) * 6 + gi


# Named colours, readable on a white background.
NAMED = {
    "Black": 0x333333, "Blue": 0x0000FF, "Brown": 0x996633,
    "Clear": 0x66CCCC, "Green": 0x009900, "Grey": 0x666666,
    "Purple": 0x9900CC, "Red": 0xFF0000, "Silver": 0x999999,
    "White": 0x999999, "Yellow": 0xCC9900,
}

DEFAULT = 0x333333  # anything not listed below

SYMBOLS = {
    # dungeon features
    '#': 0x666666, '%': 0x996633, ':': 0x999999, ';': 0x666666,
    '.': 0x999999, "'": 0x993300, '+': 0x993300, '^': 0xFF0000,
    '<': 0x0000FF, '>': 0x0000FF, '@': 0x333333,
    '1': 0x006699, '2': 0x006699, '3': 0x006699,
    '4': 0x006699, '5': 0x006699, '6': 0x006699,
    # objects
    '!': 0xCC00CC, '?': 0x996600, '=': 0xCC9900, '"': 0xCC9900,
    '$': 0xCC9900, '*': 0x00CCCC, '-': 0x6633CC, '_': 0x996633,
    '|': 0x666699, '\\': 0x666699, '/': 0x666699, '}': 0x996633,
    '{': 0x666699, '[': 0x666666, '(': 0x996633, ']': 0x666666,
    ')': 0x666666, ',': 0x009900, '&': 0x996633, '~': 0xFF9900,
    # monsters, by letter
    'a': 0x993300, 'A': 0x996633, 'b': 0x663300, 'B': 0xFF0000,
    'c': 0x996600, 'C': 0x00CCCC, 'd': 0x009900, 'D': 0xCC0000,
    'e': 0x9900CC, 'E': 0xFF6600, 'f': 0x009900, 'F': 0x669900,
    'g': 0x996633, 'G': 0x6666CC, 'h': 0xCC6600, 'H': 0x993300,
    'i': 0x669966, 'I': 0x666600, 'j': 0x9933CC, 'J': 0x9933CC,
    'k': 0x009966, 'K': 0x663300, 'l': 0x999966, 'L': 0xCC00CC,
    'm': 0x669900, 'M': 0x999966, 'n': 0x339966, 'o': 0x996633,
    'O': 0x669933, 'p': 0x0033CC, 'P': 0x993300, 'q': 0x990099,
    'Q': 0xCC0066, 'r': 0x996633, 'R': 0x339933, 's': 0x999999,
    'S': 0x660066, 't': 0x666633, 'T': 0x336633, 'U': 0x663300,
    'v': 0x6666CC, 'V': 0x990000, 'w': 0x996666, 'W': 0x666699,
    'X': 0x666666, 'y': 0x996600, 'Y': 0x6699CC, 'z': 0x669966,
    'Z': 0x996633,
}


def load_monsters(builddb):
    names = re.findall(r'"([^"]*)"',
                       open(f"{builddb}/creaturenames.dat").read())
    chars = re.findall(r"^\{.*?,'(.)',",
                       open(f"{builddb}/creature.dat").read(), re.M)
    if len(names) != MAX_CREATURES or len(chars) != MAX_CREATURES:
        sys.exit(f"expected {MAX_CREATURES} monsters, got "
                 f"{len(names)} names / {len(chars)} symbols")
    return list(zip(names, chars))


def build_colors(monsters):
    sym = [palm_index(SYMBOLS.get(chr(c), DEFAULT))
           for c in range(32, 32 + MAX_ASCII)]
    named = [palm_index(NAMED[n]) for n in COLOR_NAMES]
    mon = []
    for name, ch in monsters:
        col = sym[ord(ch) - 32]
        for word in COLOR_NAMES:
            if word in name:
                col = named[COLOR_NAMES.index(word)]
                break
        else:
            for word, base in SYNONYMS:
                if word in name:
                    col = named[COLOR_NAMES.index(base)]
                    break
        if name == "Creeping Copper Coins":
            col = palm_index(0xCC6633)
        if name == "Creeping Gold Coins":
            col = palm_index(0xCC9900)
        mon.append(col)
    return bytes(mon + sym + named)


def write_pdb(path, records):
    now = int(time.time()) + 2082844800  # Palm epoch is 1904
    header = struct.pack(
        ">32sHHIIIIII4s4sIIH",
        b"kMoriaColorDB", 0x0008,  # dmHdrAttrBackup
        1, now, now, 0, 0, 0, 0, b"Data", b"Mria", 0, 0, len(records))
    offset = len(header) + 8 * len(records) + 2
    entries = b""
    for i, rec in enumerate(records):
        entries += struct.pack(">IB3s", offset, 0x40,  # dmRecAttrDirty
                               (i + 1).to_bytes(3, "big"))
        offset += len(rec)
    with open(path, "wb") as f:
        f.write(header + entries + b"\0\0" + b"".join(records))


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__.strip().splitlines()[-1])
    colors = build_colors(load_monsters(sys.argv[1]))
    write_pdb(sys.argv[2], [colors, struct.pack(">H", 1)])
    print(f"wrote {sys.argv[2]} ({len(colors)} colours)")


if __name__ == "__main__":
    main()
