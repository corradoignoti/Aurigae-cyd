#!/usr/bin/env bash
# Build Aurigae and package everything the esp-web-tools web flasher
# (https://aurigae.fizban.net/flash.html, or any esp-web-tools instance) needs
# to flash a device from a browser: manifest.json + the four binary parts it
# points to (bootloader/partitions/boot_app0/app), zipped up together.
#
# Usage: tools/make_webflasher_zip.sh [output.zip]
#   Defaults to webflasher-data/aurigae-webflasher-<version>.zip (created if missing).
#
# Requires: pio (PlatformIO CLI), zip, python3. Builds the esp32-cyd
# environment first unless SKIP_BUILD=1 is set (e.g. you just ran `pio run`).

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

ENV_NAME="esp32-cyd"
BUILD_DIR=".pio/build/$ENV_NAME"

if [[ "${SKIP_BUILD:-0}" != "1" ]]; then
  pio run -e "$ENV_NAME"
fi

for f in bootloader.bin partitions.bin firmware.bin; do
  if [[ ! -f "$BUILD_DIR/$f" ]]; then
    echo "error: $BUILD_DIR/$f not found (build failed or wrong env name?)" >&2
    exit 1
  fi
done

# boot_app0.bin isn't a build output — it's a small static "boot slot 0" image
# shipped with the arduino-esp32 framework, needed because our partition table
# (min_spiffs.csv, see platformio.ini) has an otadata partition.
CORE_DIR="${PLATFORMIO_CORE_DIR:-$HOME/.platformio}"
BOOT_APP0="$CORE_DIR/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
if [[ ! -f "$BOOT_APP0" ]]; then
  echo "error: boot_app0.bin not found at $BOOT_APP0 (framework package missing?)" >&2
  exit 1
fi

# Pull the bare X.Y.Z out of APP_VERSION ("App ver. 2.1.0" -> "2.1.0"), same
# stripping webconfig.cpp's webconfig_current_version() does.
VERSION="$(grep -o '#define APP_VERSION "[^"]*"' src/config.h | grep -oE '[0-9]+\.[0-9]+\.[0-9]+')"
if [[ -z "$VERSION" ]]; then
  echo "error: couldn't parse APP_VERSION out of src/config.h" >&2
  exit 1
fi

OUT_DIR="webflasher-data"
mkdir -p "$OUT_DIR"

OUT_ZIP="${1:-$OUT_DIR/aurigae-webflasher-$VERSION.zip}"
# Resolve to an absolute path now, since we build the zip from a scratch dir.
case "$OUT_ZIP" in
  /*) ;;
  *) OUT_ZIP="$REPO_ROOT/$OUT_ZIP" ;;
esac

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT

cp "$BUILD_DIR/bootloader.bin" "$WORKDIR/"
cp "$BUILD_DIR/partitions.bin" "$WORKDIR/"
cp "$BUILD_DIR/firmware.bin" "$WORKDIR/"
cp "$BOOT_APP0" "$WORKDIR/"

# Offsets match this project's partition table (min_spiffs.csv) and the ESP32
# (non-S2/S3/C3) bootloader offset. If board/partitions ever change, these
# need to change with them.
cat > "$WORKDIR/manifest.json" <<EOF
{
  "name": "Aurigae",
  "version": "$VERSION",
  "home_assistant_domain": "esphome",
  "new_install_prompt_erase": true,
  "builds": [
    {
      "chipFamily": "ESP32",
      "parts": [
        { "path": "bootloader.bin", "offset": 4096 },
        { "path": "partitions.bin", "offset": 32768 },
        { "path": "boot_app0.bin", "offset": 57344 },
        { "path": "firmware.bin", "offset": 65536 }
      ]
    }
  ]
}
EOF

rm -f "$OUT_ZIP"
(cd "$WORKDIR" && zip -q -X "$OUT_ZIP" manifest.json bootloader.bin partitions.bin boot_app0.bin firmware.bin)

echo "Wrote $OUT_ZIP"
unzip -l "$OUT_ZIP"
