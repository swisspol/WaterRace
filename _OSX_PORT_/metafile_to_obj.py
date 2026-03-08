#!/usr/bin/env python3
"""
metafile_to_obj.py — Extract OBJ meshes from WaterRace MetaFile assets.

Usage:
    python3 metafile_to_obj.py <path-to-metafile>

Outputs one .obj file per shape into the current working directory.
"""

import struct
import sys
import os

# ── MetaFile header / tag table ─────────────────────────────────────────────

def parse_tag_table(data):
    """Return list of tag-entry dicts from the MetaFile tag table."""
    # File header (mac68k packed, 2-byte alignment):
    #   short version, ushort flags, OSType creatorCode, long tagTablePos, long tagTableSize
    version, flags = struct.unpack_from('>hH', data, 0)
    tag_table_pos, tag_table_size = struct.unpack_from('>ll', data, 8)

    tag_count = struct.unpack_from('>I', data, tag_table_pos)[0]

    # MetaFile_Entry (mac68k packed):
    #   uchar depth, uchar flags, short version,
    #   OSType tag, OSType subTag, OSType ID,
    #   long dataSize, long dataPosition
    # = 1+1+2+4+4+4+4+4 = 24 bytes
    ENTRY_SIZE = 24
    entries = []
    base = tag_table_pos + 4
    for i in range(tag_count):
        off = base + i * ENTRY_SIZE
        depth, eflags, eversion = struct.unpack_from('>BBh', data, off)
        tag    = data[off+4:off+8].decode('mac_roman', errors='replace')
        subtag = data[off+8:off+12].decode('mac_roman', errors='replace')
        eid    = data[off+12:off+16].decode('mac_roman', errors='replace')
        size, pos = struct.unpack_from('>ll', data, off+16)
        entries.append({
            'depth': depth, 'flags': eflags, 'version': eversion,
            'tag': tag, 'subtag': subtag, 'id': eid,
            'size': size, 'pos': pos,
        })
    return entries


# ── Shape struct parsing ─────────────────────────────────────────────────────
# Shape struct layout (PowerPC natural / 4-byte alignment, big-endian):
#   0   long   pointCount
#   4   ptr    pointList          (heap ptr, ignored)
#   8   long   triangleCount
#  12   ptr    triangleList       (heap ptr, ignored)
#  16   short  normalMode
#  18   [2 pad]
#  20   ptr    renderingProc      (heap ptr, ignored)
#  24   long   normalCount
#  28   ptr    normalList         (heap ptr, ignored)
#  32   ulong  flags
#  36   Matrix pos                (4 Vectors × 3 floats = 48 bytes)
#  84   float  rotateX/Y/Z/scale
# 100   long   link
# 104   Vector boundingBox[8]     (96 bytes)
# 200   OSType texture            (4 bytes)
# 204   float  alpha
# 208   ptr    dataPtr
# 212   float  difuse
# 216   float  specular
# 220   uint   backfaceCulling:1  (4 bytes, bitfield container)
# 224   short  unused_1
# 226   ushort shading
# 228   short  unused_2
# 230   Str31  name               (32 bytes: 1 len + 31 chars)
# 262   uchar  buffer[8]
# [2 pad to align ptr]
# 272   ptr    texturePtr
# 276   ptr    parent
# 280   float  sphereRadius
# 284   Vector sphereCenter
# 296   OSType id
# 300   (total)

def parse_shape_header(data, pos):
    d = data[pos:pos+300]
    point_count    = struct.unpack_from('>i', d, 0)[0]
    tri_count      = struct.unpack_from('>i', d, 8)[0]
    normal_mode    = struct.unpack_from('>h', d, 16)[0]
    flags          = struct.unpack_from('>I', d, 32)[0]
    # Matrix pos: row-major, rows = x,y,z,w (each a Vector = 3 floats)
    matrix = []
    for row in range(4):
        base = 36 + row * 12
        matrix.append(struct.unpack_from('>fff', d, base))
    # translation = w row
    translation = matrix[3]
    link         = struct.unpack_from('>i', d, 100)[0]
    texture      = d[200:204].decode('mac_roman', errors='replace')
    shading      = struct.unpack_from('>H', d, 226)[0]
    name_len     = d[230]
    name         = d[231:231+name_len].decode('mac_roman', errors='replace')
    shape_id     = d[296:300].decode('mac_roman', errors='replace')
    return {
        'point_count':  point_count,
        'tri_count':    tri_count,
        'normal_mode':  normal_mode,
        'flags':        flags,
        'matrix':       matrix,
        'translation':  translation,
        'link':         link,
        'texture':      texture,
        'shading':      shading,
        'name':         name,
        'id':           shape_id,
    }


# ── Vertex / triangle data ───────────────────────────────────────────────────
# Vertex struct: Vector point (3 floats), float u, float v, float c = 24 bytes
# TriFace struct: unsigned long corner[3] = 12 bytes

VERTEX_SIZE   = 24
TRIFACE_SIZE  = 12

def parse_vertices(data, pos, count):
    verts = []
    for i in range(count):
        off = pos + i * VERTEX_SIZE
        x, y, z, u, v, c = struct.unpack_from('>ffffff', data, off)
        verts.append((x, y, z, u, v))
    return verts

def parse_triangles(data, pos, count):
    tris = []
    for i in range(count):
        off = pos + i * TRIFACE_SIZE
        a, b, c = struct.unpack_from('>III', data, off)
        tris.append((a, b, c))
    return tris


# ── OBJ writer ───────────────────────────────────────────────────────────────

def sanitize_name(name):
    return ''.join(c if c.isalnum() or c in '-_.' else '_' for c in name) or 'shape'

def write_obj(filename, shape_info, vertices, triangles):
    name = shape_info['name']
    shape_id = shape_info['id'].strip()
    with open(filename, 'w') as f:
        f.write(f'# WaterRace shape: {name} (id={shape_id})\n')
        f.write(f'# {len(vertices)} vertices, {len(triangles)} triangles\n')
        f.write(f'o {sanitize_name(name)}\n\n')
        for (x, y, z, u, v) in vertices:
            f.write(f'v {x:.6f} {y:.6f} {z:.6f}\n')
        f.write('\n')
        if any(u != 0.0 or v != 0.0 for (_, _, _, u, v) in vertices):
            for (x, y, z, u, v) in vertices:
                f.write(f'vt {u:.6f} {v:.6f}\n')
            f.write('\n')
            for (a, b, c) in triangles:
                # OBJ indices are 1-based
                f.write(f'f {a+1}/{a+1} {b+1}/{b+1} {c+1}/{c+1}\n')
        else:
            for (a, b, c) in triangles:
                f.write(f'f {a+1} {b+1} {c+1}\n')
    print(f'  Wrote {filename}  ({len(vertices)} verts, {len(triangles)} tris)')


# ── Main ─────────────────────────────────────────────────────────────────────

def extract_shapes(metafile_path):
    with open(metafile_path, 'rb') as f:
        data = f.read()

    entries = parse_tag_table(data)

    # Index '<-->' subtag entries for quick lookup by position
    # We'll iterate shapes and look for their pnts/trig/nrls siblings
    base_name = os.path.splitext(os.path.basename(metafile_path))[0]
    shape_count = 0
    used_names = {}

    for idx, entry in enumerate(entries):
        if entry['tag'] != 'shap':
            continue

        shape_pos  = entry['pos']
        shape_info = parse_shape_header(data, shape_pos)

        name    = shape_info['name'] or f'shape_{idx}'
        sid     = shape_info['id'].strip('-').strip()
        pc      = shape_info['point_count']
        tc      = shape_info['tri_count']

        print(f"Shape [{sid}] '{name}'  pts={pc} tris={tc}")

        # Find pnts and trig subtags that immediately follow this shape entry
        pnts_pos = pnts_size = None
        trig_pos = trig_size = None
        for e in entries[idx+1:]:
            if e['tag'] != '<-->':
                break
            if e['subtag'] == 'pnts':
                pnts_pos, pnts_size = e['pos'], e['size']
            elif e['subtag'] == 'trig':
                trig_pos, trig_size = e['pos'], e['size']

        if pnts_pos is None or trig_pos is None:
            print(f'  WARNING: missing pnts or trig data, skipping')
            continue

        vertices  = parse_vertices(data, pnts_pos, pc)
        triangles = parse_triangles(data, trig_pos, tc)

        safe = sanitize_name(name)
        base_fn = f'{base_name}_{sid}_{safe}'
        if base_fn in used_names:
            used_names[base_fn] += 1
            filename = f'{base_fn}_{used_names[base_fn]}.obj'
        else:
            used_names[base_fn] = 0
            filename = f'{base_fn}.obj'
        write_obj(filename, shape_info, vertices, triangles)
        shape_count += 1

    print(f'\nExtracted {shape_count} shape(s) from {metafile_path}')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f'Usage: {sys.argv[0]} <metafile>', file=sys.stderr)
        sys.exit(1)
    extract_shapes(sys.argv[1])
