#!/bin/sh
# Build a 64-bit-safe PilRC 3.2 into tools/pilrc/pilrc.
#
# Why: stock PilRC 3.2 (including the Homebrew build) declares several
# BITMAPINFOHEADER fields as `long`, which is 8 bytes on 64-bit macOS.
# Every .bmp is then misparsed and fails with
#   "Bitmap not monochrome, 16, 256, 16bit, 24bit or 32bit color".
# This script downloads the same tarball Homebrew uses, verifies it,
# changes those fields to `int` (4 bytes, as the BMP format requires)
# and builds it locally.
#
# It also fixes FONT resources: font.c keeps the NFNT header in an
# `int header[]`, but CbEmitStruct reads it as `p_int` (= long, 8 bytes, on
# 64-bit). Every other header field is emitted and the rest is junk, so
# custom fonts render as garbage on the device. The header becomes `p_int`.
set -eu

VERSION=3.2
SHA256=f3d6ea3c77f5d2a00707f4372a212377ab7bd77b3d68c3db7e28a553b235903f
URL="https://downloads.sourceforge.net/project/pilrc/pilrc/$VERSION/pilrc-$VERSION.tar.gz"

TOOLS_DIR=$(cd "$(dirname "$0")" && pwd)
OUT_DIR="$TOOLS_DIR/pilrc"
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

echo "Downloading PilRC $VERSION..."
curl -sSL -o "$WORK/pilrc.tar.gz" "$URL"
echo "$SHA256  $WORK/pilrc.tar.gz" | shasum -a 256 -c -

tar xzf "$WORK/pilrc.tar.gz" -C "$WORK"
SRC="$WORK/pilrc-$VERSION"

echo "Patching BMP header struct for 64-bit hosts..."
sed -i '' -E 's/^  long (bi(Width|Height|XPelsPerMeter|YPelsPerMeter));/  int \1;/' "$SRC/bitmap.c"
if grep -qE '^  long bi' "$SRC/bitmap.c"; then
  echo "error: patch did not apply" >&2
  exit 1
fi

echo "Patching font header arrays for 64-bit hosts..."
sed -i '' -E \
  -e 's/^CloseGlyph\(int \*header,/CloseGlyph(p_int *header,/' \
  -e 's/^  int token, value, header\[(1[45])\];/  int token, value; p_int header[\1];/' \
  "$SRC/font.c"
if grep -qE '^CloseGlyph\(int|int token, value, header\[' "$SRC/font.c"; then
  echo "error: font patch did not apply" >&2
  exit 1
fi

echo "Building..."
mkdir -p "$SRC/build"
(cd "$SRC/build" && ../unix/configure >/dev/null && make >/dev/null 2>&1)

mkdir -p "$OUT_DIR"
cp "$SRC/build/pilrc" "$OUT_DIR/pilrc"
echo "Installed $OUT_DIR/pilrc"
