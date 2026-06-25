#!/usr/bin/env bash
set -euo pipefail

TARGET="${1:-All}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LIBRARY="$ROOT/Library"

build_arch() {
  local name="$1"
  local arch="$2"
  local build_dir="$LIBRARY/build/$arch"
  local bin_dir="$LIBRARY/bin/$arch"

  echo "Building $name ($arch)..."
  mkdir -p "$build_dir" "$bin_dir"
  cmake -S "$LIBRARY" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release
  cmake --build "$build_dir"
  echo "Output: $bin_dir/RmqBridge.dll (or .so on Unix)"
}

case "$TARGET" in
  MT4) build_arch MT4 x86 ;;
  MT5) build_arch MT5 x64 ;;
  All)
    build_arch MT4 x86
    build_arch MT5 x64
    ;;
  *)
    echo "Usage: $0 [MT4|MT5|All]"
    exit 1
    ;;
esac

echo "Done."
