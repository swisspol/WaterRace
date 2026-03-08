#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DATA_DIR="$(cd "$SCRIPT_DIR/../WaterRace Data" && pwd)"
TEXTURES_DIR="$SCRIPT_DIR/textures"
EXTRACTOR="$SCRIPT_DIR/extract_textures.py"
DECODER="$SCRIPT_DIR/decode_pict.py"

# (Re)create empty textures directory
rm -rf "$TEXTURES_DIR"
mkdir -p "$TEXTURES_DIR"

# Find all MetaFiles (files with no extension, excluding .rsrc_fork and .DS_Store)
while IFS= read -r -d '' metafile; do
    [[ ! -s "$metafile" ]] && continue  # skip empty files
    name="$(basename "$metafile")"
    out_dir="$TEXTURES_DIR/$name"
    mkdir -p "$out_dir"
    echo "--- $name ---"
    (cd "$out_dir" && python3 "$EXTRACTOR" "$metafile") || true

    # Convert any extracted .pict files to PNG, delete on success
    while IFS= read -r -d '' pict; do
        if python3 "$DECODER" "$pict" 2>&1; then
            rm "$pict"
        fi
    done < <(find "$out_dir" -name "*.pict" -print0)
done < <(find "$DATA_DIR" -type f ! -name "*.rsrc_fork" ! -name ".DS_Store" -print0 | sort -z)

echo ""
echo "Done. Textures saved in: $TEXTURES_DIR"
