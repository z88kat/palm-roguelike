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

The colours are vivid but dark enough to read on the default white
background, and light enough for the black one (Preferences > Invert).
Black and white swap with the background.

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
    is 111111 rather than black, which lives at 230-255.)
    """
    r, g, b = ((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF)
    for c in (r, g, b):
        if c % 0x33:
            raise ValueError(f"{rgb:06X} is not a palette-cube colour")
    if rgb == 0:
        return 255  # black: 230-255 are all black; kMoria swaps it to white
                    # on the black background
    ri, gi, bi = ((0xFF - c) // 0x33 for c in (r, g, b))
    return (bi // 3) * 108 + ri * 18 + (bi % 3) * 6 + gi


# Vivid colours that read on both backgrounds: every one has about 3:1 or
# better contrast against white and against black (Preferences > Invert),
# so there are no yellows, limes or pastels, which vanish on white.
# Pink, violet and hot pink come from the "Cyber Vibes" palette
# (colormagic.app); its yellow and turquoise are darkened to gold and teal.
RED, BRIGHT_RED, ORANGE, RED_ORANGE = 0xFF3333, 0xFF0000, 0xFF6600, 0xFF3300
AMBER, GOLD, BROWN, COPPER = 0xCC6600, 0xCC9900, 0x996633, 0xCC6633
OLIVE, LEAF, GREEN, SEA = 0x669900, 0x339900, 0x009900, 0x009966
TEAL, CERULEAN, SKY, BLUE = 0x009999, 0x0099CC, 0x0099FF, 0x0066FF
PERIWINKLE, LAVENDER, PURPLE, VIOLET = 0x6666FF, 0x9966FF, 0x9933FF, 0xCC00FF
PINK, HOT_PINK, ROSE = 0xFF00CC, 0xFF0099, 0xFF3366
GREY, SLATE = 0x999999, 0x666699
WHITE = 0xFFFFFF  # kMoria shows white as black on a white background
BLACK = 0x000000  # ... and black as white on a black background

# Named colours, used for monsters with a colour word in their name.
NAMED = {
    "Black": BLACK, "Blue": BLUE, "Brown": BROWN,
    "Clear": CERULEAN, "Green": GREEN, "Grey": GREY,
    "Purple": VIOLET, "Red": BRIGHT_RED, "Silver": 0x9999CC,
    "White": WHITE, "Yellow": GOLD,
}

DEFAULT = BLACK  # anything not listed below

SYMBOLS = {
    # dungeon features: plain walls (black, or white when inverted),
    # a dim floor
    '#': BLACK, '%': 0x6633CC, ':': 0x9966CC, ';': 0x6666CC,
    '.': 0x9999CC, "'": BROWN, '+': BROWN, '^': BRIGHT_RED,
    '<': BLUE, '>': BLUE, '@': BLACK,
    '1': TEAL, '2': TEAL, '3': TEAL,
    '4': TEAL, '5': TEAL, '6': TEAL,
    # objects
    '!': PINK, '?': SEA, '=': GOLD, '"': GOLD,
    '$': GOLD, '*': SKY, '-': VIOLET, '_': PURPLE,
    '|': PERIWINKLE, '\\': PERIWINKLE, '/': PERIWINKLE, '}': PERIWINKLE,
    '{': PERIWINKLE, '[': SLATE, '(': COPPER, ']': SLATE,
    ')': SLATE, ',': GREEN, '&': AMBER, '~': ORANGE,
    # monsters, by letter
    'a': ORANGE, 'A': RED_ORANGE, 'b': PURPLE, 'B': SKY,
    'c': OLIVE, 'C': AMBER, 'd': GREEN, 'D': RED,
    'e': PINK, 'E': CERULEAN, 'f': GOLD, 'F': OLIVE,
    'g': PERIWINKLE, 'G': LAVENDER, 'h': COPPER, 'H': ROSE,
    'i': 0x339933, 'I': OLIVE, 'j': TEAL, 'J': GREEN,
    'k': LEAF, 'K': ORANGE, 'l': BROWN, 'L': VIOLET,
    'm': OLIVE, 'M': GREY, 'n': SEA, 'o': GOLD,
    'O': AMBER, 'p': SKY, 'P': RED_ORANGE, 'q': ROSE,
    'Q': PINK, 'r': BROWN, 'R': GREEN, 's': GREY,
    'S': VIOLET, 't': CERULEAN, 'T': 0x339966, 'U': RED,
    'v': CERULEAN, 'V': HOT_PINK, 'w': 0xFF3399, 'W': LAVENDER,
    'X': 0x999966, 'y': GOLD, 'Y': 0x6699CC, 'z': 0x669933,
    'Z': TEAL,
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
            col = palm_index(COPPER)
        if name == "Creeping Gold Coins":
            col = palm_index(GOLD)
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
