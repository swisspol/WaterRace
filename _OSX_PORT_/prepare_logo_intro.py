#!/usr/bin/env python3
"""
prepare_logo_intro.py — Convert the Logo Intro MetaFile into Metal-ready binary assets.

Outputs into _OSX_PORT_/MacOS/LogoAssets/:
  logo.bin          — header + shape geometry + animation keyframes (little-endian)
  logo_<id>.jpg     — JPEG textures

Binary format:
  [Header, 32 bytes]
    uint32  script_flags
    uint32  script_length_cs   (duration in centiseconds, e.g. 850 = 8.5s)
    uint32  shape_count        (geometric shapes only, shading < 16)
    uint32  anim_count         (kType_Animation 'anim' only)
    float32 scene_cx, cy, cz  (center of keyframe-0 positions across all animations)
    float32 scene_radius       (approx bounding radius for camera placement)

  [Shapes, shape_count entries, each variable-size]
    uint8[4]  shape_id        (OSType, big-endian bytes as-is)
    uint8[4]  texture_id      (OSType, big-endian bytes as-is)
    uint32    vertex_count
    uint32    index_count     (= triangle_count * 3)
    [vertex_count × 32 bytes]  float3 pos (local), float3 norm (local), float2 uv
    [index_count  × 4 bytes]   uint32 index

  [Animations, anim_count entries, each variable-size]
    uint8[4]  target_shape_id  (OSType matching a shape_id above)
    uint32    anim_flags
    int32     end_time_cs
    uint32    event_count
    [event_count × 32 bytes]
      int32   time_cs
      float32 qw, qx, qy, qz  (quaternion: WR Quaternion {w,x,y,z})
      float32 px, py, pz       (position)

Quaternion convention:
  WR Event stores: rotateX=quat.w, rotateY=quat.x, rotateZ=quat.y, rotateW=quat.z
  (because code casts &event->rotateX to QuaternionPtr which has fields {w,x,y,z})
"""

import struct, sys, os, math

HERE      = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from metafile_to_obj import (parse_tag_table, parse_shape_header,
                              parse_vertices, parse_triangles)
from extract_textures import _extract_jpeg_if_compressed_quicktime

LOGO_PATH = os.path.join(HERE, '..', 'WaterRace Data', 'Core', 'Logo Intro')
OUT_DIR   = os.path.join(HERE, 'MacOS', 'LogoAssets')

kShader_FXOffset = 16   # shading >= 16 → FX shape (lens, light, sprite, …) – skip
kType_Animation  = 'anim'


# ── Helpers ──────────────────────────────────────────────────────────────────

def parse_normals(data, pos, count):
    return [struct.unpack_from('>fff', data, pos + i*12) for i in range(count)]


def compute_normals(positions, triangles):
    acc = [[0.0, 0.0, 0.0] for _ in range(len(positions))]
    for a, b, c in triangles:
        ax, ay, az = positions[a]
        bx, by, bz = positions[b]
        cx, cy, cz = positions[c]
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


def ostype_str(b: bytes) -> str:
    try:
        return b.decode('mac_roman')
    except Exception:
        return b.hex()


# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    with open(LOGO_PATH, 'rb') as fh:
        data = fh.read()

    entries = parse_tag_table(data)
    print(f"Logo Intro MetaFile: {len(entries)} tag entries\n")

    # Dump tag table for debugging
    for i, e in enumerate(entries):
        print(f"  [{i:3d}] depth={e['depth']} tag={e['tag']!r} "
              f"sub={e['subtag']!r} id={e['id']!r} "
              f"size={e['size']}  pos={e['pos']}")
    print()

    # ── Script + Animations ────────────────────────────────────────────────
    script_flags     = 0
    script_length_cs = 0
    animations       = []          # list of dicts

    for idx, entry in enumerate(entries):
        if entry['tag'] != 'scrp':
            continue

        # Script struct at entry['pos'] (192 bytes):
        #   Str31 name  (32)   + flags(4) + id(4) + type(4) + skeletonID(4)
        #   + startTime(4) + length(4) + loopTime(4) + animCount(4) + ptrs(128)
        sb = data[entry['pos']:entry['pos'] + 192]
        script_flags      = struct.unpack_from('>I', sb, 32)[0]
        script_length_cs  = struct.unpack_from('>i', sb, 52)[0]
        anim_count_hdr    = struct.unpack_from('>I', sb, 60)[0]
        name_len = sb[0]
        name     = sb[1:1+name_len].decode('mac_roman', errors='replace')
        stype    = sb[40:44].decode('mac_roman', errors='replace')
        print(f"Script: name={name!r} type={stype!r} "
              f"flags=0x{script_flags:08x} length={script_length_cs}cs "
              f"anim_count={anim_count_hdr}")

        # Children: '<-->' entries with subtag 'anim', depth = entry['depth']+1
        child_depth = entry['depth'] + 1
        for e in entries[idx+1:]:
            if e['depth'] < child_depth:
                break                     # left script scope
            if e['tag'] != '<-->' or e['subtag'] != 'anim':
                continue

            ab = data[e['pos']:e['pos'] + e['size']]
            if len(ab) < 24:
                print(f"  WARNING: anim entry too small ({len(ab)}B)")
                continue

            # CinematicAnimation fixed header (24 bytes):
            #   OSType shapeID(4) + ulong flags(4) + OSType type(4) + long endTime(4)
            #   + Boolean running(1) + pad(3) + ulong eventCount(4)
            shape_id_b  = ab[0:4]
            anim_flags  = struct.unpack_from('>I', ab, 4)[0]
            type_b      = ab[8:12]
            end_time    = struct.unpack_from('>i', ab, 12)[0]
            event_count = struct.unpack_from('>I', ab, 20)[0]
            type_str    = ostype_str(type_b)
            sid_str     = ostype_str(shape_id_b)

            print(f"  Anim: target={sid_str!r} type={type_str!r} "
                  f"endTime={end_time}cs events={event_count}")

            evs = []
            for j in range(event_count):
                off = 24 + j*32
                if off + 32 > len(ab):
                    print(f"    WARNING: event {j} out of bounds")
                    break
                # int32 time, float rotX(=qw), rotY(=qx), rotZ(=qy), rotW(=qz),
                # float px, py, pz
                time_cs, rotX, rotY, rotZ, rotW, px, py, pz = \
                    struct.unpack_from('>i4f3f', ab, off)
                # WR cast: &event->rotateX → QuaternionPtr{w,x,y,z}
                # so rotX=qw, rotY=qx, rotZ=qy, rotW=qz
                qw, qx, qy, qz = rotX, rotY, rotZ, rotW
                print(f"    ev[{j}] t={time_cs}cs "
                      f"q=({qw:.3f},{qx:.3f},{qy:.3f},{qz:.3f}) "
                      f"pos=({px:.1f},{py:.1f},{pz:.1f})")
                evs.append(dict(time_cs=time_cs,
                                qw=qw, qx=qx, qy=qy, qz=qz,
                                px=px, py=py, pz=pz))

            animations.append(dict(
                target_id=shape_id_b,
                flags=anim_flags,
                type=type_str,
                end_time=end_time,
                events=evs,
            ))

        break   # only the first scrp entry

    # ── Shapes ────────────────────────────────────────────────────────────
    shapes = []
    for idx, entry in enumerate(entries):
        if entry['tag'] != 'shap':
            continue

        info    = parse_shape_header(data, entry['pos'])
        pc      = info['point_count']
        tc      = info['tri_count']
        shading = info['shading']
        sid_str = info['id']
        name    = info['name']
        tex_str = info['texture']

        print(f"\nShape [{sid_str!r}] {name!r}: "
              f"pts={pc} tris={tc} shading={shading} tex={tex_str!r}")

        if shading >= kShader_FXOffset:
            print(f"  → Skip: FX shape (shading={shading})")
            continue
        if pc == 0 or tc == 0:
            print(f"  → Skip: no geometry")
            continue

        # Geometry children
        pnts_e = trig_e = nrls_e = None
        child_depth = entry['depth'] + 1
        for e in entries[idx+1:]:
            if e['depth'] < child_depth:
                break
            if e['tag'] != '<-->':
                continue
            if e['subtag'] == 'pnts': pnts_e = e
            elif e['subtag'] == 'trig': trig_e = e
            elif e['subtag'] == 'nrls': nrls_e = e

        if pnts_e is None or trig_e is None:
            print(f"  → Skip: missing pnts/trig")
            continue

        raw_verts = parse_vertices(data, pnts_e['pos'], pc)
        triangles = parse_triangles(data, trig_e['pos'], tc)

        # Keep positions in LOCAL space (animation drives world transform)
        positions = [(v[0], v[1], v[2]) for v in raw_verts]

        if nrls_e and nrls_e['size'] >= pc*12:
            normals = parse_normals(data, nrls_e['pos'], pc)
            print(f"  Using stored normals")
        else:
            normals = compute_normals(positions, triangles)
            print(f"  Computed normals")

        # UV: flip V (WR V=0-at-bottom → Metal V=0-at-top)
        uvs = [(v[3], 1.0 - v[4]) for v in raw_verts]

        # Initial model matrix (WR row-major → Metal column-major 4x4)
        # WR matrix rows: x={m00,m01,m02}, y={m10,m11,m12}, z={m20,m21,m22}, w=translation
        # Metal: col i = WR row i (viewed as column vector) → transpose
        m = info['matrix']  # 4 rows of 3 floats each
        # Write as 16 floats: col0, col1, col2, col3
        # col0 = (m[0][0], m[0][1], m[0][2], 0)  ← WR row x
        # col1 = (m[1][0], m[1][1], m[1][2], 0)  ← WR row y
        # col2 = (m[2][0], m[2][1], m[2][2], 0)  ← WR row z
        # col3 = (m[3][0], m[3][1], m[3][2], 1)  ← translation
        init_mat = [
            m[0][0], m[0][1], m[0][2], 0.0,   # col0
            m[1][0], m[1][1], m[1][2], 0.0,   # col1
            m[2][0], m[2][1], m[2][2], 0.0,   # col2
            m[3][0], m[3][1], m[3][2], 1.0,   # col3
        ]
        print(f"  Initial pos: ({m[3][0]:.2f}, {m[3][1]:.2f}, {m[3][2]:.2f})")

        # Raw OSType bytes (big-endian, 4 bytes each)
        sid_bytes = sid_str.encode('mac_roman')[:4].ljust(4, b'\x00')
        tex_bytes = tex_str.encode('mac_roman')[:4].ljust(4, b'\x00')

        shapes.append(dict(
            shape_id=sid_bytes,
            texture_id=tex_bytes,
            initial_matrix=init_mat,
            positions=positions,
            normals=normals,
            uvs=uvs,
            triangles=triangles,
        ))
        print(f"  → Added: {pc} verts, {tc} tris")

    # ── Only keep kType_Animation animations ──────────────────────────────
    anim_anims = [a for a in animations if a['type'] == kType_Animation]
    print(f"\nSummary: {len(shapes)} renderable shapes, "
          f"{len(anim_anims)} kType_Animation animations")

    # ── Scene bounds from FINAL keyframe positions of non-camera shapes ──────
    # Use the last keyframe (resting position of letters) for camera setup.
    final_pos = []
    for a in anim_anims:
        target_str = ostype_str(a['target_id']).strip()
        if target_str == 'camr':
            continue   # camera path — not a scene object
        if a['events']:
            ev = a['events'][-1]   # last keyframe = final resting position
            final_pos.append((ev['px'], ev['py'], ev['pz']))

    if final_pos:
        cx = sum(p[0] for p in final_pos) / len(final_pos)
        cy = sum(p[1] for p in final_pos) / len(final_pos)
        cz = sum(p[2] for p in final_pos) / len(final_pos)
        rmax = max(math.sqrt((p[0]-cx)**2 + (p[1]-cy)**2 + (p[2]-cz)**2)
                   for p in final_pos)
        scene_radius = max(rmax * 1.2, 1.0)   # 20% margin, at least 1 unit
    else:
        cx, cy, cz, scene_radius = 0.0, 0.0, 0.0, 5.0

    print(f"Scene centre: ({cx:.1f}, {cy:.1f}, {cz:.1f})  radius: {scene_radius:.1f}")

    # ── Write logo.bin ────────────────────────────────────────────────────
    out = os.path.join(OUT_DIR, 'logo.bin')
    with open(out, 'wb') as fh:
        # Header (32 bytes)
        fh.write(struct.pack('<IIII', script_flags, script_length_cs,
                             len(shapes), len(anim_anims)))
        fh.write(struct.pack('<4f', cx, cy, cz, scene_radius))

        # Shapes
        for sh in shapes:
            pc2 = len(sh['positions'])
            ic  = len(sh['triangles']) * 3
            fh.write(sh['shape_id'])    # 4
            fh.write(sh['texture_id'])  # 4
            fh.write(struct.pack('<II', pc2, ic))
            fh.write(struct.pack('<16f', *sh['initial_matrix']))  # 64 bytes
            for i in range(pc2):
                x,  y,  z  = sh['positions'][i]
                nx, ny, nz = sh['normals'][i]
                u,  v      = sh['uvs'][i]
                fh.write(struct.pack('<8f', x, y, z, nx, ny, nz, u, v))
            for a2, b2, c2 in sh['triangles']:
                fh.write(struct.pack('<III', a2, b2, c2))

        # Animations
        for a in anim_anims:
            fh.write(a['target_id'])    # 4
            fh.write(struct.pack('<Ii', a['flags'], a['end_time']))
            fh.write(struct.pack('<I', len(a['events'])))
            for ev in a['events']:
                fh.write(struct.pack('<i4f3f',
                                     ev['time_cs'],
                                     ev['qw'], ev['qx'], ev['qy'], ev['qz'],
                                     ev['px'], ev['py'], ev['pz']))

    print(f"\nWrote {out}  ({os.path.getsize(out):,} bytes)")

    # ── Extract Textures ──────────────────────────────────────────────────
    for i, e in enumerate(entries):
        if e['tag'] != 'cptx' or i+1 >= len(entries):
            continue
        nxt = entries[i+1]
        if nxt['tag'] != '<-->' or nxt['subtag'] != 'txim':
            continue

        tex_id  = e['id'].strip()
        pict    = data[nxt['pos']:nxt['pos'] + nxt['size']]
        jpeg    = _extract_jpeg_if_compressed_quicktime(pict)
        if jpeg:
            out_tex = os.path.join(OUT_DIR, f'logo_{tex_id}.jpg')
            with open(out_tex, 'wb') as fh:
                fh.write(jpeg)
            print(f"Texture {tex_id!r}: {out_tex}  ({len(jpeg):,} bytes)")
        else:
            print(f"WARNING: texture {tex_id!r} is not JPEG-in-QT, skipping")


if __name__ == '__main__':
    main()
