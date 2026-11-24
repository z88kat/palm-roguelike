#!/usr/bin/env python3
"""Build iRogueTileDB.prc, iRogue's tile set, from iRogue/tiles/tiles.txt.

iRogue (Options > Preferences > Tiles) looks for a resource database called
"iRogueTileDB" and draws map character c with its (c - 32)th bitmap: 10x9
pixels, one for every character from ' ' to '~'. tiles.txt describes the
pictures (see the comments at its top); characters it leaves out get the
glyph from the 2002 starter set, drawn in grey.

The bitmaps are 8-bit (Palm OS 3.5 colour) with a transparent background, so
they work on both the normal and the inverted (black) background.

Usage: make-irogue-tiles.py <tiles.txt> <starter.prc> <out.prc> [preview.png]
"""
import struct
import sys
import time
import zlib

W, H = 10, 9          # tile size, as in iRogue's display.c
FIRST, LAST = 32, 126  # ' ' .. '~'
FIRST_ID = 128         # resource IDs, as in the 2002 set
WEBSAFE = (0xFF, 0xCC, 0x99, 0x66, 0x33, 0x00)
GREY = (0x99, 0x99, 0x99)
# Never drawn, so it can mark transparent pixels. It must not be black: on a
# 16-bit screen Palm OS compares colours rather than indexes.
TRANSPARENT = (0x33, 0xFF, 0xCC)


def palm_index(rgb):
    """Index of a web-safe colour in the Palm OS 8-bit system palette.

    The palette is not in plain R-G-B order; this formula was checked against
    pilrc's palette for all 216 web-safe colours.
    """
    if rgb == (0, 0, 0):
        return 255  # black has two entries; 255 is the usual one
    r, g, b = (WEBSAFE.index(c) for c in rgb)
    return (108 if b >= 3 else 0) + (b % 3) * 6 + g + r * 18


def parse_rgb(s, where):
    try:
        rgb = tuple(int(s[i:i + 2], 16) for i in (0, 2, 4))
        if len(s) != 6 or any(c not in WEBSAFE for c in rgb):
            raise ValueError
    except ValueError:
        sys.exit(f"{where}: {s!r} is not a web-safe rrggbb colour")
    return rgb


def darker(rgb):
    return tuple(max(c - 0x33, 0) for c in rgb)


def read_tiles(path):
    colors = {}
    tiles = {}    # symbol -> 9 rows of RGB tuples (None = transparent)
    copies = []
    lines = open(path, encoding="utf-8").read().split("\n")
    i = 0
    while i < len(lines):
        line = lines[i].rstrip()
        where = f"{path}:{i + 1}"
        i += 1
        if not line or line.startswith("#"):
            continue
        words = line.split()
        if words[0] == "color" and len(words) == 3 and len(words[1]) == 1:
            colors[words[1]] = parse_rgb(words[2].upper(), where)
        elif words[0] == "copy" and len(words) == 3:
            copies.append((words[1], words[2], where))
        elif words[0] == "tile" and len(words) >= 3:
            sym = words[1]
            if len(sym) != 1 or not FIRST < ord(sym) <= LAST:
                sys.exit(f"{where}: bad symbol {sym!r}")
            if sym in tiles:
                sys.exit(f"{where}: second tile for {sym!r}")
            own = None
            if len(words) >= 4 and len(words[-1]) == 6:
                try:
                    int(words[-1], 16)
                    own = parse_rgb(words[-1].upper(), where)
                except ValueError:
                    pass
            pal = dict(colors)
            if own:
                pal["#"], pal["="] = own, darker(own)
            rows = lines[i:i + H]
            i += H
            if len(rows) != H:
                sys.exit(f"{where}: tile {sym!r} needs {H} rows")
            pixels = []
            for n, row in enumerate(rows):
                row = row.rstrip("\n")
                if len(row) != W:
                    sys.exit(f"{path}:{i - H + n + 1}: row must be {W} "
                             f"pixels, not {len(row)}")
                out = []
                for ch in row:
                    if ch == ".":
                        out.append(None)
                    elif ch in pal:
                        out.append(pal[ch])
                    else:
                        sys.exit(f"{path}:{i - H + n + 1}: unknown colour "
                                 f"{ch!r} in tile {sym!r}")
                pixels.append(out)
            tiles[sym] = pixels
        else:
            sys.exit(f"{where}: don't understand {line!r}")
    for sym, other, where in copies:
        if other not in tiles:
            sys.exit(f"{where}: no tile {other!r} to copy")
        tiles[sym] = tiles[other]
    return tiles


def read_resources(path):
    """The (type, id, data) resources of a .prc file."""
    d = open(path, "rb").read()
    n = struct.unpack(">H", d[76:78])[0]
    ents = [struct.unpack(">4sHI", d[78 + 10 * k:88 + 10 * k])
            for k in range(n)]
    out = []
    for k, (typ, rid, off) in enumerate(ents):
        end = ents[k + 1][2] if k + 1 < n else len(d)
        out.append((typ, rid, d[off:end]))
    return out


def starter_glyph(data):
    """Pixels of one bitmap of the 2002 set: drawn pixels become grey."""
    w, h, rb = struct.unpack(">HHH", data[:6])
    px = data[16:16 + rb * h]
    return [[GREY if px[y * rb + x] not in (0,) else None
             for x in range(W)] for y in range(H)]


def bitmap(pixels):
    """A Palm OS BitmapType (version 2, 8 bits, transparent)."""
    t = palm_index(TRANSPARENT)
    body = bytearray()
    for row in pixels:
        for p in row:
            body.append(t if p is None else palm_index(p))
    # width, height, rowBytes, flags (hasTransparency), pixelSize, version,
    # nextDepthOffset, transparentIndex, compressionType (none), reserved
    return struct.pack(">HHHHBBHBBH", W, H, W, 0x2000, 8, 2, 0, t, 0xFF,
                       0) + bytes(body)


def write_prc(path, resources):
    now = int(time.time()) + 2082844800  # seconds since 1904
    hdr = struct.pack(">32sHHIIIIII4s4sIIH", b"iRogueTileDB", 0x0009, 1,
                      now, now, 0, 0, 0, 0, b"Tile", b"RoGe", 0, 0,
                      len(resources))
    offset = len(hdr) + 10 * len(resources) + 2
    entries = b""
    for rid, data in resources:
        entries += struct.pack(">4sHI", b"Tbmp", rid, offset)
        offset += len(data)
    with open(path, "wb") as f:
        f.write(hdr + entries + b"\0\0" + b"".join(d for _, d in resources))


def write_png(path, cells):
    """Preview sheet: every tile on white, then on black, 6x size."""
    s, cols = 6, 16
    rows = (len(cells) + cols - 1) // cols
    cw, ch = (W + 2) * s, (H + 2) * s
    width, height = cols * cw, rows * ch * 2
    img = [[(0xCC, 0xCC, 0xCC)] * width for _ in range(height)]
    for half, bg in enumerate(((255, 255, 255), (0, 0, 0))):
        for k, pix in enumerate(cells):
            ox = (k % cols) * cw + s
            oy = half * rows * ch + (k // cols) * ch + s
            for y in range(H):
                for x in range(W):
                    c = pix[y][x] or bg
                    for dy in range(s):
                        r = img[oy + y * s + dy]
                        r[ox + x * s:ox + x * s + s] = [c] * s
    raw = b"".join(b"\0" + bytes(v for p in row for v in p) for row in img)

    def chunk(t, b):
        return (struct.pack(">I", len(b)) + t + b +
                struct.pack(">I", zlib.crc32(t + b)))
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" +
                chunk(b"IHDR", struct.pack(">IIBBBBB", width, height,
                                           8, 2, 0, 0, 0)) +
                chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))


def main():
    if len(sys.argv) not in (4, 5):
        sys.exit(__doc__.strip().split("\n\n")[-1])
    tiles = read_tiles(sys.argv[1])
    starter = read_resources(sys.argv[2])
    for sym, pix in tiles.items():
        if TRANSPARENT in (p for row in pix for p in row):
            sys.exit(f"tile {sym!r} uses the transparent marker colour")
    cells = []
    for c in range(FIRST, LAST + 1):
        if chr(c) in tiles:
            cells.append(tiles[chr(c)])
        else:
            cells.append(starter_glyph(starter[c - FIRST][2]))
    write_prc(sys.argv[3],
              [(FIRST_ID + k, bitmap(p)) for k, p in enumerate(cells)])
    print(f"wrote {sys.argv[3]} ({len(tiles)} drawn tiles, "
          f"{len(cells)} bitmaps)")
    if len(sys.argv) == 5:
        write_png(sys.argv[4], cells)


if __name__ == "__main__":
    main()
