# iLarn

Larn for Palm OS, by Bridget Spitznagel (2000–2001). This directory is a
checkout of the original CVS repository, which is kept with full history in
[`../cvs-repo/iLarn`](../cvs-repo/iLarn).

## Which version this is

iLarn has no branches, so this is the trunk head: the `Release-2001_08_06`
release plus a few small changes from that time that were only committed in
March 2004 (`main.c`, `iLarn.rcp`).

## Layout

| Path | What it is |
| --- | --- |
| `src/` | The game (C sources, `iLarn.rcp` resources, `Makefile`) |
| `src/iLarnDB.pdb` | Game-data database — **must be installed with the app** |
| `src/FixDB/` | Palm utility that originally built/repaired `iLarnDB.pdb` |
| `src/Icon/round/` | Alternative app icon |

## Building

Same toolchain and steps as [kMoria](../kMoria/README.md):

```sh
./tools/build-pilrc.sh     # one-time, from the repo root
cd iLarn/src && make       # -> iLarn.prc
make install               # app + iLarnDB.pdb via pilot-xfer over USB
```

Build changes: toolchain paths, `m68k-palmos-multigen`, and the patched
pilrc. iLarn doesn't use the HandEra headers, so it needs no `compat/`
shim. No game source code was changed.
