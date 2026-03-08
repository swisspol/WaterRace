#!/usr/bin/env python3
"""
extract_textures.py — Extract textures from WaterRace MetaFiles.

Usage:
    python3 extract_textures.py <path-to-metafile>

Output:
    One file per texture in the current directory, named by texture ID.
    JPEG-compressed textures are saved as .jpg; others as raw .pict.

MetaFile format (big-endian, mac68k 2-byte aligned):
  Header (16 bytes):
    short         version          (should be 0x0104)
    ushort        flags
    OSType (4)    creatorCode
    long (4)      tagTablePosition
    long (4)      tagTableSize

  Tag Table at tagTablePosition:
    ulong (4)     tagCount
    Entry[tagCount], each 24 bytes:
      uchar       depth
      uchar       flags
      short       version
      OSType (4)  tag
      OSType (4)  subTag
      OSType (4)  ID
      long (4)    dataSize
      long (4)    dataPosition

Textures are stored as pairs:
  Entry[i]:   tag='cptx', ID=<texture name>
  Entry[i+1]: tag='<-->', subTag='txim'  ← raw PICT bytes at dataPosition
"""

import struct
import sys
import os


# MetaFile constants
HEADER_FMT = '>hH4sll'  # short, ushort, 4s, long, long  = 16 bytes
HEADER_SIZE = struct.calcsize(HEADER_FMT)  # should be 16

ENTRY_FMT = '>BBh4s4s4sll'  # uchar, uchar, short, 4s, 4s, 4s, long, long = 24 bytes
ENTRY_SIZE = struct.calcsize(ENTRY_FMT)   # should be 24

TAG_CPTX = b'cptx'
TAG_INCOMPLETE = b'<-->'
SUBTAG_TXIM = b'txim'


def ostype_str(b: bytes) -> str:
    """Convert 4-byte OSType to a printable string."""
    try:
        return b.decode('mac_roman')
    except Exception:
        return b.hex()


def parse_header(data: bytes):
    version, flags, creator, tag_table_pos, tag_table_size = struct.unpack_from(HEADER_FMT, data, 0)
    return {
        'version': version,
        'flags': flags,
        'creator': creator,
        'tag_table_pos': tag_table_pos,
        'tag_table_size': tag_table_size,
    }


def parse_entries(data: bytes, tag_table_pos: int):
    tag_count = struct.unpack_from('>L', data, tag_table_pos)[0]
    entries = []
    offset = tag_table_pos + 4
    for _ in range(tag_count):
        depth, eflags, version, tag, subtag, eid, data_size, data_pos = struct.unpack_from(ENTRY_FMT, data, offset)
        entries.append({
            'depth': depth,
            'flags': eflags,
            'version': version,
            'tag': tag,
            'subtag': subtag,
            'id': eid,
            'data_size': data_size,
            'data_position': data_pos,
        })
        offset += ENTRY_SIZE
    return entries


def extract_textures(metafile_path: str):
    with open(metafile_path, 'rb') as f:
        data = f.read()

    header = parse_header(data)
    if header['version'] != 0x0104:
        print(f"Warning: unexpected MetaFile version 0x{header['version']:04x} (expected 0x0104)")

    entries = parse_entries(data, header['tag_table_pos'])
    print(f"MetaFile: {len(entries)} tag entries")

    textures_found = 0
    i = 0
    while i < len(entries):
        entry = entries[i]
        if entry['tag'] == TAG_CPTX:
            tex_id = ostype_str(entry['id'])
            # The PICT data is in the very next entry: tag='<-->', subtag='txim'
            if i + 1 < len(entries):
                next_entry = entries[i + 1]
                if next_entry['tag'] == TAG_INCOMPLETE and next_entry['subtag'] == SUBTAG_TXIM:
                    pict_size = next_entry['data_size']
                    pict_pos = next_entry['data_position']
                    pict_data = data[pict_pos:pict_pos + pict_size]

                    print(f"  Found texture '{tex_id}': {pict_size} bytes of PICT at offset {pict_pos}")
                    _save_texture(pict_data, tex_id)
                    textures_found += 1
                    i += 2
                    continue
                else:
                    print(f"  Warning: cptx entry for '{tex_id}' not followed by txim (got tag={ostype_str(next_entry['tag'])} subtag={ostype_str(next_entry['subtag'])})")
            else:
                print(f"  Warning: cptx entry for '{tex_id}' at end of table with no following entry")
        i += 1

    print(f"\nExtracted {textures_found} texture(s).")


def _save_texture(pict_data: bytes, tex_id: str):
    safe_name = ''.join(c if c.isalnum() or c in '-_' else '_' for c in tex_id)

    jpeg = _extract_jpeg_if_compressed_quicktime(pict_data)
    if jpeg is not None:
        out_path = os.path.abspath(f"{safe_name}.jpg")
        with open(out_path, 'wb') as f:
            f.write(jpeg)
        print(f"    Saved JPEG: {out_path}")
    else:
        out_path = os.path.abspath(f"{safe_name}.pict")
        with open(out_path, 'wb') as f:
            f.write(pict_data)
        print(f"    Saved PICT: {out_path}")


def _extract_jpeg_if_compressed_quicktime(pict_data: bytes):
    """Walk PICT v2 opcodes. If the image opcode is CompressedQuickTime (0x8200),
    extract and return the embedded JPEG stream. Otherwise return None."""
    if len(pict_data) < 12:
        return None
    offset = 10  # skip picSize(2) + picFrame(8)
    while offset < len(pict_data) - 2:
        try:
            op = struct.unpack_from('>H', pict_data, offset)[0]
        except struct.error:
            return None
        offset += 2
        if op == 0x0011:    offset += 2
        elif op == 0x0C00:  offset += 24
        elif op == 0x001E:  pass
        elif op == 0x0001:
            sz = struct.unpack_from('>H', pict_data, offset)[0]; offset += sz
        elif op == 0x00A1:
            offset += 2
            sz = struct.unpack_from('>H', pict_data, offset)[0]; offset += 2 + sz
        elif op == 0x8200:
            # CompressedQuickTime: 4-byte size, then QuickTime data containing JPEG
            qt_size = struct.unpack_from('>L', pict_data, offset)[0]
            qt_data = pict_data[offset + 4: offset + 4 + qt_size]
            soi = qt_data.find(b'\xff\xd8')
            eoi = qt_data.rfind(b'\xff\xd9')
            if soi >= 0 and eoi > soi:
                return qt_data[soi:eoi + 2]
            return None
        else:
            return None  # DirectBitsRect or unknown — not a JPEG PICT
    return None


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <path-to-metafile>")
        sys.exit(1)

    metafile_path = sys.argv[1]
    if not os.path.isfile(metafile_path):
        print(f"Error: file not found: {metafile_path!r}")
        sys.exit(1)

    extract_textures(metafile_path)
