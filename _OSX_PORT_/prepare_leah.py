#!/usr/bin/env python3
"""
prepare_leah.py — Convert the Leah MetaFile into Metal-ready binary assets.

Outputs into _OSX_PORT_/MacOS/LeahAssets/:
  leah.bin          — header + vertex buffer + index buffer (little-endian)
  leah_<id>.jpg     — JPEG texture

Binary format:
  uint32  vertex_count
  uint32  index_count        (triangle_count * 3)
  float32 bbox[6]            (min_x, min_y, min_z, max_x, max_y, max_z)
  vertex_count × 32 bytes:
      float32  x, y, z       (position, transformed by shape local matrix)
      float32  nx, ny, nz    (normal,   transformed by shape rotation)
      float32  u, v          (UV, v flipped for Metal: 0=top)
  index_count × uint32       (triangle indices, little-endian)
"""

import struct, sys, os, math

HERE  = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from metafile_to_obj import parse_tag_table, parse_shape_header, parse_vertices, parse_triangles
from extract_textures import _extract_jpeg_if_compressed_quicktime

LEAH_PATH = os.path.join(HERE, '..', 'WaterRace Data', 'Core', 'Leah')
OUT_DIR   = os.path.join(HERE, 'MacOS', 'LeahAssets')


def parse_normals(data, pos, count):
    return [struct.unpack_from('>fff', data, pos + i * 12) for i in range(count)]


def transform_point(v, m):
    """Row-vector × WaterRace matrix → transformed point."""
    x = v[0]*m[0][0] + v[1]*m[1][0] + v[2]*m[2][0] + m[3][0]
    y = v[0]*m[0][1] + v[1]*m[1][1] + v[2]*m[2][1] + m[3][1]
    z = v[0]*m[0][2] + v[1]*m[1][2] + v[2]*m[2][2] + m[3][2]
    return (x, y, z)


def transform_normal(n, m):
    """Rotate normal by WaterRace matrix (no translation), then normalize."""
    x = n[0]*m[0][0] + n[1]*m[1][0] + n[2]*m[2][0]
    y = n[0]*m[0][1] + n[1]*m[1][1] + n[2]*m[2][1]
    z = n[0]*m[0][2] + n[1]*m[1][2] + n[2]*m[2][2]
    l = math.sqrt(x*x + y*y + z*z)
    return (x/l, y/l, z/l) if l > 1e-6 else (0.0, 1.0, 0.0)


def compute_normals(vertices, triangles):
    """Per-vertex normals computed by averaging adjacent face normals."""
    acc = [[0.0, 0.0, 0.0] for _ in range(len(vertices))]
    for a, b, c in triangles:
        ax, ay, az = vertices[a]
        bx, by, bz = vertices[b]
        cx, cy, cz = vertices[c]
        ux, uy, uz = bx-ax, by-ay, bz-az
        vx, vy, vz = cx-ax, cy-ay, cz-az
        nx, ny, nz = uy*vz-uz*vy, uz*vx-ux*vz, ux*vy-uy*vx
        for i in (a, b, c):
            acc[i][0] += nx; acc[i][1] += ny; acc[i][2] += nz
    result = []
    for x, y, z in acc:
        l = math.sqrt(x*x + y*y + z*z)
        result.append((x/l, y/l, z/l) if l > 1e-6 else (0.0, 1.0, 0.0))
    return result


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    with open(LEAH_PATH, 'rb') as f:
        data = f.read()

    entries = parse_tag_table(data)
    print(f"Leah MetaFile: {len(entries)} tag entries")

    # ── Geometry ──────────────────────────────────────────────────────────────
    for idx, entry in enumerate(entries):
        if entry['tag'] != 'shap':
            continue

        info   = parse_shape_header(data, entry['pos'])
        pc, tc = info['point_count'], info['tri_count']
        matrix = info['matrix']
        print(f"Shape {info['name']!r}: {pc} verts, {tc} tris, texture={info['texture']!r}")

        pnts_e = trig_e = nrls_e = None
        for e in entries[idx+1:]:
            if e['tag'] != '<-->': break
            if e['subtag'] == 'pnts': pnts_e = e
            elif e['subtag'] == 'trig': trig_e = e
            elif e['subtag'] == 'nrls': nrls_e = e

        if pnts_e is None or trig_e is None:
            print("  ERROR: missing geometry"); continue

        raw_verts  = parse_vertices(data, pnts_e['pos'], pc)
        triangles  = parse_triangles(data, trig_e['pos'], tc)

        # Transform positions by shape local matrix
        positions = [transform_point(v[:3], matrix) for v in raw_verts]

        # Normals
        if nrls_e and nrls_e['size'] >= pc * 12:
            raw_n   = parse_normals(data, nrls_e['pos'], pc)
            normals = [transform_normal(n, matrix) for n in raw_n]
            print(f"  Using stored normals")
        else:
            normals = compute_normals(positions, triangles)
            print(f"  Computed normals")

        # Bounding box
        xs, ys, zs = zip(*positions)
        bbox = (min(xs), min(ys), min(zs), max(xs), max(ys), max(zs))
        cx = (bbox[0]+bbox[3])/2; cy = (bbox[1]+bbox[4])/2; cz = (bbox[2]+bbox[5])/2
        diam = max(bbox[3]-bbox[0], bbox[4]-bbox[1], bbox[5]-bbox[2])
        print(f"  BBox: ({bbox[0]:.1f},{bbox[1]:.1f},{bbox[2]:.1f}) "
              f"→ ({bbox[3]:.1f},{bbox[4]:.1f},{bbox[5]:.1f})")
        print(f"  Center: ({cx:.1f},{cy:.1f},{cz:.1f}), diameter: {diam:.1f}")

        # UV: flip V to match Metal/glTF convention (V=0 at top).
        # WaterRace/QuickDraw3D has V=0 at bottom; Metal texture sampling
        # has V=0 at top. The GLB exporter does the same flip.
        uvs = [(v[3], 1.0 - v[4]) for v in raw_verts]

        # Write binary
        out = os.path.join(OUT_DIR, 'leah.bin')
        with open(out, 'wb') as f:
            f.write(struct.pack('<II', pc, tc * 3))
            f.write(struct.pack('<6f', *bbox))
            for i in range(pc):
                x,  y,  z  = positions[i]
                nx, ny, nz = normals[i]
                u,  v      = uvs[i]
                f.write(struct.pack('<8f', x, y, z, nx, ny, nz, u, v))
            for a, b, c in triangles:
                f.write(struct.pack('<III', a, b, c))
        print(f"  Wrote {out}  ({os.path.getsize(out):,} bytes)")
        break  # single shape

    # ── Texture ───────────────────────────────────────────────────────────────
    for i, e in enumerate(entries):
        if e['tag'] != 'cptx' or i+1 >= len(entries): continue
        nxt = entries[i+1]
        if nxt['tag'] != '<-->' or nxt['subtag'] != 'txim': continue

        tex_id = e['id'].strip()
        pict   = data[nxt['pos']:nxt['pos']+nxt['size']]
        jpeg   = _extract_jpeg_if_compressed_quicktime(pict)
        if jpeg:
            out = os.path.join(OUT_DIR, f'leah_{tex_id}.jpg')
            with open(out, 'wb') as f:
                f.write(jpeg)
            print(f"Texture {tex_id!r}: {out}  ({len(jpeg):,} bytes)")
        else:
            print(f"WARNING: texture {tex_id!r} is not JPEG-in-QT, skipping")


if __name__ == '__main__':
    main()
