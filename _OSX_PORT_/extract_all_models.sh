#!/usr/bin/env bash
# extract_all_models.sh — Extract OBJ meshes from all WaterRace metafiles.
#
# Usage: bash extract_all_models.sh
# Run from anywhere; paths are relative to the script's location.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DATA_DIR="$REPO_ROOT/WaterRace Data"
MODELS_DIR="$SCRIPT_DIR/models"
PYTHON="$SCRIPT_DIR/.venv/bin/python3"
EXTRACTOR="$SCRIPT_DIR/metafile_to_obj.py"

# Re-create models directory
rm -rf "$MODELS_DIR"
mkdir -p "$MODELS_DIR"

total_files=0
total_shapes=0

while IFS= read -r -d '' metafile; do
    # Skip resource forks and hidden files
    basename="${metafile##*/}"
    case "$basename" in
        .* | *.rsrc_fork) continue ;;
    esac

    # Derive a subdir path mirroring the structure under "WaterRace Data"
    rel="${metafile#$DATA_DIR/}"
    subdir="$MODELS_DIR/$rel"
    mkdir -p "$subdir"

    echo "==> $rel"
    shapes=$(cd "$subdir" && "$PYTHON" "$EXTRACTOR" "$metafile" 2>&1 | tee /dev/stderr \
        | grep -c '  Wrote ' || true)
    total_shapes=$((total_shapes + shapes))
    total_files=$((total_files + 1))

done < <(find "$DATA_DIR" -type f -print0)

echo ""
echo "Done: $total_files metafile(s) processed, $total_shapes OBJ(s) written to $MODELS_DIR"
