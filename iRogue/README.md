# iRogue

Rogue for Palm OS, by Bridget Spitznagel (2000–2004). This directory is a
checkout of the original CVS repository, which is kept with full history in
[`../cvs-repo/iRogue`](../cvs-repo/iRogue).

## Which version this is

iRogue's CVS trunk stops in April 2001. All later development happened on
the **HandEra branch** (`Release-2001_04_26-Handera-branch`), so this
checkout is the **head of that branch** (16 Jan 2004), not the trunk:

| Point in history | Tag | Notes |
| --- | --- | --- |
| Apr 2001 | `Release-2001_04_26` | trunk head; what a plain `cvs checkout` gives |
| May 2002 | `Release-2002_05_10-on-Handera-branch` | last official release |
| Nov 2003 | — | big reorganisation: `iRogue.c` split into `form_*.c`, old shared-library code merged back in |
| Jan 2004 | `pre-internationalization_20040113` | adds item naming ('c'), on-device colour settings, save-file limit |
| 16 Jan 2004 | *(branch head — this checkout)* | only adds `strings.h` changes, which nothing includes yet |

To check out a different point, use
[`tools/cvs-resolve-tag.py`](../tools/cvs-resolve-tag.py), which works out
what `cvs checkout -r TAG` would produce (branch tags, `Attic/`, deleted
files) from the raw `,v` files:

```sh
cd cvs-repo/iRogue
python3 ../../tools/cvs-resolve-tag.py . Release-2002_05_10-on-Handera-branch |
  while IFS="$(printf '\t')" read rev date file src; do
    mkdir -p "/some/dir/$(dirname "$file")"
    co -q -ko -p"$rev" "$src" > "/some/dir/$file"
  done
```

## Building

Same toolchain and steps as [kMoria](../kMoria/README.md):

```sh
./tools/build-pilrc.sh     # one-time, from the repo root
cd iRogue/src && make      # -> iRogue.prc
```

The build changes are the same too: toolchain paths, `m68k-palmos-multigen`,
LF copies of the CRLF HandEra SDK headers in `src/compat/`, and the patched
pilrc. No game source code was changed.

## Running

Only `iRogue.prc` needs installing. On first run the game creates
`iRogueDB` itself (saved game, high scores). An optional `iRogueTileDB`
adds graphical tiles; without it the game uses text characters.
