#!/usr/bin/env python3
"""Build kMoria-manual.pdb, the kMoria manual as a Palm DOC e-book.

Readable on the device in any Palm DOC reader (AportisDoc, Palm Reader,
CSpotRun, ...). It has two parts:

  Part 1  kMoria/MANUAL.md, converted from Markdown to plain text: the Palm
          controls, menus, shops, preferences and so on.
  Part 2  The rules chapters of the original Umoria 5.5.2 manual
          (kMoria/src/MoriaManual/moria.txt): the character, map symbols,
          the town, combat, objects, winning and dying. Its chapters 5
          (Commands, for the Unix game) and 12 (Wizards) are left out.

moria.txt is copied byte for byte (it uses Palm's numeric-space character,
0x19, to line up its tables). The Markdown is unwrapped into one line per
paragraph so the reader can wrap it to the screen, and encoded in the Palm
character set (Windows-1252 for the characters used here).

Usage: make-kmoria-manual.py <MANUAL.md> <moria.txt> <output.pdb>
"""
import re
import struct
import sys
import time

RECORD_SIZE = 4096
CHAR_MAP = {"▸": ">", "→": "->", "−": "-", "✅": "yes", "❌": "no",
            "⚠️": "partly", "⚠": "partly"}


# ---------------------------------------------------------------- Markdown

def inline(text):
    text = re.sub(r"!\[([^\]]*)\]\([^)]*\)", r"\1", text)   # images
    text = re.sub(r"\[([^\]]+)\]\([^)]*\)", r"\1", text)    # links
    text = re.sub(r"\*\*(.+?)\*\*", r"\1", text)            # bold
    text = re.sub(r"(?<![\w*])\*(?!\s)(.+?)(?<!\s)\*(?![\w*])", r"\1", text)
    text = re.sub(r"`([^`]*)`", r"\1", text)                # code
    return text.replace("\\|", "|")  # table-escaped pipe


def split_row(line):
    line = line.strip().strip("|")
    cells = re.split(r"(?<!\\)\|", line)
    return [inline(c.strip()) for c in cells]


def table_lines(rows):
    """A Markdown table (raw lines) as one line per row: 'cell - cell'."""
    header, body = split_row(rows[0]), rows[2:]
    out = []
    # "Symbol | Meaning | Symbol | Meaning" tables hold two entries per row
    pairs = (len(header) == 4 and header[0] == header[2]
             and header[1] == header[3])
    for r in body:
        cells = split_row(r)
        groups = [cells[:2], cells[2:]] if pairs else [cells]
        for g in groups:
            g = [c for c in g if c]
            if g:
                out.append(" - ".join(g))
    return out


def markdown_to_text(md):
    out = []
    para = []

    def flush():
        # inline markup is resolved per paragraph: bold or code may wrap
        if para:
            out.append(inline(" ".join(para)))
            para.clear()

    lines = md.splitlines()
    i = 0
    while i < len(lines):
        line = lines[i]
        s = line.strip()
        if s.startswith("```"):
            flush()
            i += 1
            while i < len(lines) and not lines[i].strip().startswith("```"):
                out.append(lines[i])
                i += 1
            out.append("")
        elif s.startswith("|"):
            flush()
            rows = []
            while i < len(lines) and lines[i].strip().startswith("|"):
                rows.append(lines[i])
                i += 1
            out.extend(table_lines(rows))
            out.append("")
            continue
        elif s == "---":
            flush()
            out.append("")
        elif s.startswith("#"):
            flush()
            level = len(s) - len(s.lstrip("#"))
            title = inline(s.lstrip("#").strip())
            if out and out[-1] != "":
                out.append("")
            if level == 1:
                out.append(title.upper())
                out.append("=" * min(len(title), 30))
            elif level == 2:
                out.append(title.upper())
            else:
                out.append(title)
        elif s == "":
            flush()
            if out and out[-1] != "":
                out.append("")
        elif re.match(r"^(\s*)[-*] ", line) or re.match(r"^\s*\d+\. ", line):
            flush()
            indent = len(line) - len(line.lstrip())
            para.append(" " * indent + s)
        elif s.startswith(">"):
            para.append(s.lstrip("> ").strip())
        else:
            para.append(s)
        i += 1
    flush()
    text = "\n".join(out)
    return re.sub(r"\n{3,}", "\n\n", text).strip() + "\n"


def to_palm(text):
    for k, v in CHAR_MAP.items():
        text = text.replace(k, v)
    return text.encode("cp1252")


# --------------------------------------------------------------- moria.txt

def moria_rules(raw):
    """Chapters 1-4 and 6-11 of moria.txt, raw bytes."""
    def chapter(n):
        m = re.search(rb"^%d\.  \S" % n, raw, re.M)
        if not m:
            sys.exit(f"moria.txt: chapter {n} not found")
        return m.start()
    return raw[:chapter(5)] + raw[chapter(6):chapter(12)]


# ----------------------------------------------------------------- PalmDOC

def compress(rec):
    """PalmDOC (LZ77-style) compression of one record."""
    out = bytearray()
    i, n = 0, len(rec)
    while i < n:
        best_len = best_dist = 0
        if i >= 1:
            lo = max(0, i - 2047)
            for length in range(min(10, n - i), 2, -1):
                j = rec.rfind(rec[i:i + length], lo, i + length - 1)
                if j != -1 and j < i:
                    best_len, best_dist = length, i - j
                    break
        if best_len:
            out += struct.pack(">H", 0x8000 | (best_dist << 3)
                               | (best_len - 3))
            i += best_len
            continue
        c = rec[i]
        if c == 0x20 and i + 1 < n and 0x40 <= rec[i + 1] <= 0x7F:
            out.append(rec[i + 1] ^ 0x80)
            i += 2
        elif c == 0 or 0x09 <= c <= 0x7F:
            out.append(c)
            i += 1
        else:  # bytes 1-8 and 0x80+ are sent as a literal run
            j = i
            while (j < n and j - i < 8
                   and not (rec[j] == 0 or 0x09 <= rec[j] <= 0x7F)):
                j += 1
            out.append(j - i)
            out += rec[i:j]
            i = j
    return bytes(out)


def decompress(b):
    out = bytearray()
    i = 0
    while i < len(b):
        c = b[i]
        i += 1
        if c == 0 or 0x09 <= c <= 0x7F:
            out.append(c)
        elif c <= 8:
            out += b[i:i + c]
            i += c
        elif c >= 0xC0:
            out += bytes([0x20, c ^ 0x80])
        else:
            x = (c << 8) | b[i]
            i += 1
            dist, length = (x >> 3) & 0x7FF, (x & 7) + 3
            for _ in range(length):
                out.append(out[-dist])
    return bytes(out)


def write_pdb(path, name, text):
    chunks = [text[k:k + RECORD_SIZE] for k in range(0, len(text), RECORD_SIZE)]
    records = [compress(c) for c in chunks]
    for c, r in zip(chunks, records):
        if decompress(r) != c:
            sys.exit("internal error: compression round trip failed")
    header0 = struct.pack(">HHIHHI", 2, 0, len(text), len(records),
                          RECORD_SIZE, 0)
    records = [header0] + records
    now = int(time.time()) + 2082844800  # Palm epoch is 1904
    hdr = struct.pack(">32sHHIIIIII4s4sIIH", name.encode()[:31], 0x0008, 0,
                      now, now, 0, 0, 0, 0, b"TEXt", b"REAd", 0, 0,
                      len(records))
    offset = len(hdr) + 8 * len(records) + 2
    entries = b""
    for k, rec in enumerate(records):
        entries += struct.pack(">IB3s", offset, 0x40, (k + 1).to_bytes(3, "big"))
        offset += len(rec)
    with open(path, "wb") as f:
        f.write(hdr + entries + b"\0\0" + b"".join(records))
    return len(records) - 1


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__.strip().splitlines()[-1])
    part1 = to_palm(markdown_to_text(open(sys.argv[1], encoding="utf-8").read()))
    part2 = moria_rules(open(sys.argv[2], "rb").read())
    divider = to_palm(
        "\n\n----------------------------------------\n\n"
        "PART 2: THE RULES OF MORIA\n"
        "==========================\n\n"
        "What follows is the original Umoria 5.5.2 manual, minus its "
        "chapters on commands (5) and wizards (12). Its rules apply to "
        "kMoria; where it mentions keys or commands, use kMoria's controls "
        "from Part 1 instead.\n\n")
    text = part1 + divider + part2
    n = write_pdb(sys.argv[3], "kMoria Manual", text)
    print(f"wrote {sys.argv[3]} ({len(text)} chars, {n} records)")


if __name__ == "__main__":
    main()
