#!/usr/bin/env python3
"""
decode_pict.py — Convert a WaterRace .pict file to PNG.

Usage:
    python3 decode_pict.py <path-to.pict>

Output:
    PNG at the same path with .png extension.
    Exits with code 1 and an error message if the file cannot be decoded.

Supports the two DirectBitsRect variants found in this project:
  - packType=4, pixelSize=32, cmpCount=3 → RGB  (component planes, PackBits rows)
  - packType=4, pixelSize=32, cmpCount=4 → ARGB (component planes, PackBits rows)
  - packType=3, pixelSize=16, cmpCount=3 → RGB 5-5-5 (PackBits rows)
"""

import struct
import sys
import os
import zlib


# ---------------------------------------------------------------------------
# PackBits decompression
# ---------------------------------------------------------------------------

def unpack_bits(data: bytes, expected_len: int, word_mode: bool = False) -> bytes:
    """Decompress PackBits data.

    word_mode=True: each unit is a 16-bit word (used by packType=3, 16-bit pixels).
    word_mode=False: each unit is a byte (used by packType=4, 32-bit pixels).
    """
    unit = 2 if word_mode else 1
    result = bytearray()
    i = 0
    while i < len(data):
        flag = data[i]; i += 1
        if flag == 0x80:
            pass  # NOP
        elif flag > 0x80:
            count = 257 - flag
            result.extend(data[i:i + unit] * count); i += unit
        else:
            count = flag + 1
            result.extend(data[i:i + count * unit]); i += count * unit
    if len(result) < expected_len:
        raise ValueError(f"PackBits underflow: got {len(result)}, expected {expected_len}")
    return bytes(result[:expected_len])


# ---------------------------------------------------------------------------
# PNG writer (no dependencies)
# ---------------------------------------------------------------------------

def _png_chunk(tag: bytes, data: bytes) -> bytes:
    crc = zlib.crc32(tag + data) & 0xFFFFFFFF
    return struct.pack('>I', len(data)) + tag + data + struct.pack('>I', crc)


def write_png(path: str, width: int, height: int, pixels: bytes, has_alpha: bool):
    """pixels: raw bytes, row-major, either RGB or RGBA (no padding)."""
    color_type = 6 if has_alpha else 2  # RGBA or RGB
    channels = 4 if has_alpha else 3

    raw = bytearray()
    row_stride = width * channels
    for y in range(height):
        raw.append(0)  # filter type: None
        raw.extend(pixels[y * row_stride:(y + 1) * row_stride])

    ihdr = struct.pack('>IIBBBBB', width, height, 8, color_type, 0, 0, 0)
    idat = zlib.compress(bytes(raw), 6)

    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n')
        f.write(_png_chunk(b'IHDR', ihdr))
        f.write(_png_chunk(b'IDAT', idat))
        f.write(_png_chunk(b'IEND', b''))


# ---------------------------------------------------------------------------
# PICT parsing
# ---------------------------------------------------------------------------

def _parse_pixmap(data: bytes, offset: int):
    """Parse a PixMap record (50 bytes) starting at offset. Returns (info_dict, new_offset)."""
    fmt = '>LHhhhhhhLLLhhhhlLL'
    # baseAddr(4) rowBytes(2) top(2) left(2) bot(2) right(2)
    # pmVersion(2) packType(2) packSize(4) hRes(4) vRes(4)
    # pixelType(2) pixelSize(2) cmpCount(2) cmpSize(2) planeBytes(4) pmTable(4) pmReserved(4)
    size = struct.calcsize(fmt)  # should be 50
    fields = struct.unpack_from(fmt, data, offset)
    info = {
        'row_bytes': fields[1] & 0x7FFF,
        'top': fields[2], 'left': fields[3], 'bottom': fields[4], 'right': fields[5],
        'pack_type': fields[7],
        'pixel_size': fields[12],   # fields[11]=pixelType, fields[12]=pixelSize
        'cmp_count': fields[13],
        'cmp_size': fields[14],
    }
    return info, offset + size


def _decode_direct_bits_rect(data: bytes, offset: int):
    """Decode from the byte after the 0x009A opcode. Returns (pixels, width, height, has_alpha)."""
    pm, offset = _parse_pixmap(data, offset)

    row_bytes = pm['row_bytes']
    width  = pm['right']  - pm['left']
    height = pm['bottom'] - pm['top']
    pack_type  = pm['pack_type']
    pixel_size = pm['pixel_size']
    cmp_count  = pm['cmp_count']

    # Skip srcRect (8) + dstRect (8) + mode (2)
    offset += 18

    # Determine row length prefix size (1 or 2 bytes)
    long_prefix = row_bytes > 250

    if pack_type == 4 and pixel_size == 32 and cmp_count in (3, 4):
        # 32-bit component planes: each row packed as cmpCount planes of `width` bytes
        uncompressed_row = width * cmp_count
        rows_rgba = []  # list of (A[], R[], G[], B[]) or (R[], G[], B[])

        for _ in range(height):
            if long_prefix:
                byte_count = struct.unpack_from('>H', data, offset)[0]; offset += 2
            else:
                byte_count = data[offset]; offset += 1
            row_raw = unpack_bits(data[offset:offset + byte_count], uncompressed_row)
            offset += byte_count

            if cmp_count == 3:
                rows_rgba.append((None, row_raw[0:width], row_raw[width:2*width], row_raw[2*width:3*width]))
            else:
                # Planes: A R G B (plane 0 is alpha/pad in QuickDraw 32-bit)
                rows_rgba.append((row_raw[0:width], row_raw[width:2*width], row_raw[2*width:3*width], row_raw[3*width:4*width]))

        # Check whether the alpha plane carries real data.
        # In classic QuickDraw 32-bit mode the first byte is a pad byte (always 0).
        # Only emit RGBA if at least one alpha value is non-zero.
        has_alpha = cmp_count == 4 and any(
            a != 0 for A, _, _, _ in rows_rgba if A is not None for a in A
        )

        pixels = bytearray()
        for A, R, G, B in rows_rgba:
            if has_alpha:
                for x in range(width):
                    pixels.extend((R[x], G[x], B[x], A[x]))
            else:
                for x in range(width):
                    pixels.extend((R[x], G[x], B[x]))

        return bytes(pixels), width, height, has_alpha

    elif pack_type == 3 and pixel_size == 16 and cmp_count == 3:
        # 16-bit RGB 5-5-5: each row is word-based PackBits of 16-bit big-endian pixels
        uncompressed_row = width * 2
        pixels = bytearray()

        for _ in range(height):
            if long_prefix:
                byte_count = struct.unpack_from('>H', data, offset)[0]; offset += 2
            else:
                byte_count = data[offset]; offset += 1
            row_raw = unpack_bits(data[offset:offset + byte_count], uncompressed_row, word_mode=True)
            offset += byte_count

            for x in range(width):
                val = struct.unpack_from('>H', row_raw, x * 2)[0]
                r = ((val >> 10) & 0x1F) * 255 // 31
                g = ((val >> 5)  & 0x1F) * 255 // 31
                b = (val         & 0x1F) * 255 // 31
                pixels.extend((r, g, b))

        return bytes(pixels), width, height, False

    else:
        raise ValueError(
            f"Unsupported DirectBitsRect format: packType={pack_type} "
            f"pixelSize={pixel_size} cmpCount={cmp_count}"
        )


def decode_pict(data: bytes):
    """Parse PICT v2 data, decode the first image, return (pixels, width, height, has_alpha)."""
    if len(data) < 16:
        raise ValueError("File too small to be a PICT")

    # PICT header: picSize(2) + picFrame(8) = 10 bytes, then opcodes
    offset = 10
    while offset < len(data) - 2:
        opcode = struct.unpack_from('>H', data, offset)[0]
        offset += 2

        if opcode == 0x0011:    # VersionOp
            offset += 2
        elif opcode == 0x0C00:  # HeaderOp
            offset += 24
        elif opcode == 0x001E:  # DefHilite — no data
            pass
        elif opcode == 0x0001:  # Clip
            sz = struct.unpack_from('>H', data, offset)[0]; offset += sz
        elif opcode == 0x00A1:  # LongComment
            offset += 2  # kind
            sz = struct.unpack_from('>H', data, offset)[0]; offset += 2 + sz
        elif opcode == 0x009A:  # DirectBitsRect
            return _decode_direct_bits_rect(data, offset)
        elif opcode == 0x0098:  # PackBitsRect — not used in this project
            raise ValueError("PackBitsRect (0x0098) is not supported")
        elif opcode == 0x8200:  # CompressedQuickTime
            raise ValueError("CompressedQuickTime (0x8200) is not supported — extract as JPEG instead")
        elif opcode == 0x00FF:  # EndOfPicture
            raise ValueError("EndOfPicture reached without finding image data")
        else:
            raise ValueError(f"Unknown PICT opcode {opcode:#06x} at offset {offset - 2:#x}")

    raise ValueError("Reached end of file without finding image data")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <path-to.pict>", file=sys.stderr)
        sys.exit(1)

    pict_path = sys.argv[1]
    if not os.path.isfile(pict_path):
        print(f"Error: file not found: {pict_path!r}", file=sys.stderr)
        sys.exit(1)

    with open(pict_path, 'rb') as f:
        data = f.read()

    try:
        pixels, width, height, has_alpha = decode_pict(data)
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

    out_path = os.path.splitext(pict_path)[0] + '.png'
    write_png(out_path, width, height, pixels, has_alpha)
    mode = 'RGBA' if has_alpha else 'RGB'
    print(f"Saved {width}×{height} {mode} PNG: {out_path}")


if __name__ == '__main__':
    main()
