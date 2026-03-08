#!/usr/bin/env bash
# extract_all_assets.sh — Export all WaterRace metafiles to .glb (binary glTF 2.0).
#
# Usage: bash extract_all_assets.sh
# Run from anywhere; paths are relative to the script's location.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DATA_DIR="$REPO_ROOT/WaterRace Data"
ASSETS_DIR="$SCRIPT_DIR/assets"
PYTHON="$SCRIPT_DIR/.venv/bin/python3"
EXTRACTOR="$SCRIPT_DIR/metafile_to_gltf.py"

# Re-create assets directory
rm -rf "$ASSETS_DIR"
mkdir -p "$ASSETS_DIR"

total_files=0
total_glbs=0

while IFS= read -r -d '' metafile; do
    # Skip resource forks and hidden files
    basename="${metafile##*/}"
    case "$basename" in
        .* | *.rsrc_fork) continue ;;
    esac

    # Derive a subdir path mirroring the structure under "WaterRace Data"
    rel="${metafile#$DATA_DIR/}"
    subdir="$ASSETS_DIR/$(dirname "$rel")"
    mkdir -p "$subdir"

    echo "==> $rel"
    output=$(cd "$subdir" && "$PYTHON" "$EXTRACTOR" "$metafile" 2>&1)
    echo "$output"
    glbs=$(echo "$output" | grep -c '^Wrote' || true)
    total_glbs=$((total_glbs + glbs))
    total_files=$((total_files + 1))

done < <(find "$DATA_DIR" -type f -print0)

echo ""
echo "Done: $total_files metafile(s) processed, $total_glbs .glb file(s) written to $ASSETS_DIR"
