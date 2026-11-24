# Roguelikes for Palm OS

Three classic roguelike games for Palm OS handhelds: **iRogue**, **iLarn** and
**kMoria**. Bridget Spitznagel wrote them between 2000 and 2004 for the
[roguelike-palm](https://roguelike-palm.sourceforge.net/) project on
SourceForge. This repository revives them: the original CVS repository is kept
with its full history, the games build with a modern toolchain on macOS, and
each has had a round of bug fixes and new features (see below) for 2026.

| Game | Port of | Version | Details |
| --- | --- | --- | --- |
| iRogue | Rogue (via Rogue Clone) | 1.2 | [README](iRogue/README.md) · [Manual](iRogue/MANUAL.md) |
| iLarn | Larn (via ULarn) | 0.40 | [README](iLarn/README.md) · [Manual](iLarn/MANUAL.md) |
| kMoria | Moria (UMoria 5.5.2) | 0.16 | [README](kMoria/README.md) · [Manual](kMoria/MANUAL.md) |

## What's new in the 2026 versions

- **Neon colour scheme** shared by all three games: every monster and kind
  of item has its own colour, readable on both the white and the inverted
  (black) background. It is built into iRogue and iLarn, where the old
  `iRogue-RGB` / `iLarn-RGB` memos still override individual symbols, and
  comes with kMoria as the optional `kMoriaColorDB.pdb`.
- **5-way navigator** on Palm OS 5 devices such as the Tungsten C, so the
  games can be played without the stylus.
- **Bug fixes** from a code review of each game: crashes, memory leaks,
  broken saves and rules mistakes. The iRogue and iLarn READMEs list them.
- About, License and History screens updated, and a player's manual
  (`MANUAL.md`) for each game, also built as a Palm DOC e-book.

## Installing

Ready-built files are committed in each game's `build/` directory, and the
website's download pages
([iRogue](https://z88kat.github.io/palm-roguelike/iRogue/download.html),
[iLarn](https://z88kat.github.io/palm-roguelike/iLarn/download.html),
[kMoria](https://z88kat.github.io/palm-roguelike/kMoria/download.html))
link to them. `make` (below) rebuilds them:

| Game | Install | Optional |
| --- | --- | --- |
| iRogue | `iRogue.prc` | `iRogueTileDB.prc`, `iRogue-manual.pdb` |
| iLarn | `iLarn.prc` **and** `iLarnDB.pdb` | `iLarn-manual.pdb` |
| kMoria | `kMoria.prc` **and** `kMoriaDB.pdb` | `kMoriaColorDB.pdb`, `kMoria-manual.pdb` |

iLarn and kMoria will not run without their `*DB.pdb` game-data database.
Install the files with HotSync, `pilot-xfer`, or by dragging them onto an
emulator such as [CloudpilotEmu](https://cloudpilot-emu.github.io/).

The games are built for Palm OS 3.5 and later, and are tested in
CloudpilotEmu as a Palm m515 (Palm OS 4.1) and a Tungsten C (Palm OS 5).

## Building

You need:

- the [palmdev-macos](https://github.com/savaughn/palmdev-macos) toolchain
  (prc-tools, `m68k-palmos-gcc` 2.95), expected in
  `~/Development/palmdev-macos/toolchain`;
- the Palm OS SDKs in `/usr/local/palmdev`;
- Python 3 (for the manual and colour databases);
- Homebrew `rcs`, only for working with `cvs-repo/` and rebuilding the website.

```sh
./tools/build-pilrc.sh        # one-time: patched pilrc for 64-bit macOS

cd iRogue/src && make         # -> iRogue/build/
cd ../../iLarn/src && make    # -> iLarn/build/
cd ../../kMoria/src && make   # -> kMoria/build/
```

Each game's README covers build options, the changes needed for a modern Mac,
and how to run it in CloudpilotEmu.

## Repository layout

| Path | What it is |
| --- | --- |
| `iRogue/`, `iLarn/`, `kMoria/` | The games: sources, manual and README for each |
| `cvs-repo/` | The original CVS repository (RCS `,v` files), with full history, untouched |
| `docs/` | The original project website as static HTML (generated, see below) |
| `tools/build-pilrc.sh` | Builds a patched pilrc that reads `.bmp` files and fonts correctly on 64-bit hosts |
| `tools/cvs-resolve-tag.py` | Works out what `cvs checkout -r TAG` would give from the raw `,v` files |
| `tools/make-*-manual.py` | Turn each `MANUAL.md` into a Palm DOC e-book |
| `tools/make-kmoria-colors.py` | Generates kMoria's optional colour database |
| `tools/build-website.py`, `tools/website/` | Rebuild the website in `docs/` |

The game directories began as checkouts of `cvs-repo/`: the trunk head for
iLarn and kMoria, and the HandEra branch head for iRogue, where its later
development happened. Each game's README says exactly which revision it
started from.

## The original website

The original roguelike-palm.sourceforge.net site (last updated in 2006), with
its news, how-to-play guides, screenshots and colour documentation, is live
again at **<https://z88kat.github.io/palm-roguelike/>**. It is served by
GitHub Pages from [`docs/`](docs/) on `main`. The download links still point
at the original releases on SourceForge.

The site was written in PHP and lives in `cvs-repo/htdocs/`.
`tools/build-website.py` checks it out, fills in the PHP includes, and writes
static pages to `docs/`. Some included files were never committed to CVS;
they were recovered from the live SourceForge site into `tools/website/`.
News since the revival is in `tools/website/news.txt`, which the build adds
to the top of the home page's News box. Pages that had gone out of date (each
game's download, source and what's-new pages) have updated copies in
`tools/website/<game>/`, which the build uses instead of the CVS versions;
SourceForge-era links on the other pages are fixed by `REPLACEMENTS` in the
script. `docs/` is wiped on every build, so
edit `cvs-repo/htdocs/` or `tools/website/` instead:

```sh
python3 tools/build-website.py
```

## Credits and licence

The games are by Bridget Spitznagel (Half-Dead Spider Entertainment). They
build on the work of the authors of Rogue and Rogue Clone, Larn and ULarn,
and Moria and UMoria. The original credits are kept in each game's About
and License screens.

The games are free software under the GNU General Public License, version 2
or (at your option) any later version, as stated in their source files. See
[`LICENSE`](LICENSE) for the GPL version 3 text.
