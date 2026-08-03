#!/bin/sh
set -eu

VERSION="${1:-dev}"
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
OUTPUT_DIR="${2:-$SCRIPT_DIR/dist}"
ARCHIVE="$OUTPUT_DIR/retro-chiaki-muos-application-launcher-$VERSION.zip"

mkdir -p "$OUTPUT_DIR"
(
	cd "$SCRIPT_DIR"
	zip -qr "$ARCHIVE" MUOS
)

printf '%s\n' "$ARCHIVE"
