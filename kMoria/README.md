# kMoria

Moria for Palm OS, by Bridget Spitznagel (2000–2002). This directory is a
checkout of the latest CVS revision (tag `Release-2002_03_29`, v0.15) from
the original repository, which is kept with full history in
[`../cvs-repo/kMoria`](../cvs-repo/kMoria).

## Layout

| Path | What it is |
| --- | --- |
| `src/` | The game (C sources, `kMoria.rcp` resources, `Makefile`) |
| `src/kMoriaDB.pdb` | Game-data database (monsters, items, ...) — **must be installed with the app** |
| `src/BuildDB/` | Palm app that originally generated `kMoriaDB.pdb` |
| `src/ConvertSave/` | Host tool to convert saves to/from UMoria format |
| `src/kMoriaUtil/` | Small companion Palm utility |
| `src/MoriaManual/` | Moria manual as text and as a PalmDOC `.pdb` |

## Building

Requires the [palmdev-macos](https://github.com/savaughn/palmdev-macos)
toolchain (prc-tools / m68k-palmos-gcc 2.95) and the Palm OS SDKs in
`/usr/local/palmdev`.

```sh
# one-time: build a pilrc that can read .bmp files on 64-bit macOS
../tools/build-pilrc.sh        # run from the repo root as ./tools/build-pilrc.sh

cd kMoria/src
make                           # -> kMoria.prc
```

If the toolchain lives somewhere other than
`~/Development/palmdev-macos/toolchain`, pass
`make PALMDEV_TOOLCHAIN=/path/to/toolchain`.

The default target is Palm OS 3.5+ with colour. Other targets (OS 2, OS 3.1
mono) are selectable via the commented `CFLAGS` lines in the Makefile;
`make clean` between them.

### Changes made to build on a modern Mac

- `Makefile`: toolchain paths, `m68k-palmos-multigen` instead of `multigen`,
  `pilot-xfer` install target updated for USB.
- The HandEra SDK headers have CRLF line endings that break gcc 2.95; the
  Makefile makes LF copies in `src/compat/Handera/` at build time.
- Stock pilrc 3.2 misreads every `.bmp` on 64-bit hosts;
  `tools/build-pilrc.sh` builds a patched copy.

No game source code was changed.

## Testing in CloudpilotEmu

The native `cloudpilot-emu` binary reads CLI commands from stdin. Installing
fails until PalmOS has finished booting, so feed it commands through a file:

```sh
: > /tmp/cmds.txt
tail -f /tmp/cmds.txt | ~/Development/GitHub/cloudpilot-emu/src/cloudpilot/cloudpilot-emu \
    ~/Development/GitHub/cloudpilot-emu/roms/Palm-m515-4.1-en.rom &

# once the emulator window shows PalmOS:
echo "install $PWD/kMoria.prc $PWD/kMoriaDB.pdb" >> /tmp/cmds.txt
echo "launch kMoria" >> /tmp/cmds.txt
```

On a freshly booted ROM the first `launch` only dismisses the setup screen;
send it a second time.
