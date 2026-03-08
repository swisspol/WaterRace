#!/usr/bin/env python3
"""
metafile_to_gltf.py — Convert a WaterRace MetaFile to a .glb (binary glTF 2.0).

Usage:
    python3 metafile_to_gltf.py <path-to-metafile>

Output: <same-path>.glb  — single self-contained file with meshes + textures.

Reuses:
  metafile_to_obj.py  — tag-table parser, shape/vertex/triangle parsers
  decode_pict.py      — PICT decoder and PNG chunk builder
  extract_textures.py — QuickTime JPEG extractor
"""

import struct, sys, os, json, zlib

from metafile_to_obj import (
    parse_tag_table, parse_shape_header, parse_vertices, parse_triangles,
)
from decode_pict import decode_pict, _png_chunk
from extract_textures import _extract_jpeg_if_compressed_quicktime


# ── Helpers ──────────────────────────────────────────────────────────────────

def parse_normals(data, pos, count):
    """Parse a normals list (Vector = 3 big-endian floats each)."""
    normals = []
    for i in range(count):
        off = pos + i * 12
        normals.append(struct.unpack_from('>fff', data, off))
    return normals


def encode_png_bytes(width, height, pixels, has_alpha):
    """Encode raw pixel data to a PNG byte string (no file I/O)."""
    color_type = 6 if has_alpha else 2
    channels   = 4 if has_alpha else 3
    row_stride = width * channels
    raw = bytearray()
    for y in range(height):
        raw.append(0)  # filter: None
        raw.extend(pixels[y * row_stride:(y + 1) * row_stride])
    ihdr = struct.pack('>IIBBBBB', width, height, 8, color_type, 0, 0, 0)
    idat = zlib.compress(bytes(raw), 6)
    return (b'\x89PNG\r\n\x1a\n'
            + _png_chunk(b'IHDR', ihdr)
            + _png_chunk(b'IDAT', idat)
            + _png_chunk(b'IEND', b''))


def pict_to_image(pict_data):
    """Return (mime_type, image_bytes, has_alpha) for a PICT blob.
    Tries JPEG (CompressedQuickTime) first, falls back to PICT→PNG."""
    jpeg = _extract_jpeg_if_compressed_quicktime(pict_data)
    if jpeg is not None:
        return 'image/jpeg', jpeg, False
    pixels, w, h, has_alpha = decode_pict(pict_data)
    return 'image/png', encode_png_bytes(w, h, pixels, has_alpha), has_alpha


# ── GLB builder ──────────────────────────────────────────────────────────────

ARRAY_BUFFER         = 34962
ELEMENT_ARRAY_BUFFER = 34963
FLOAT                = 5126
UNSIGNED_INT         = 5125


def build_glb(shapes_data, textures_map):
    """Build a GLB binary from extracted shapes and textures.

    shapes_data: list of dicts with keys info, vertices, triangles, normals
    textures_map: dict ostype_str -> (mime_type, image_bytes)
    """
    bin_buf      = bytearray()
    buffer_views = []
    accessors    = []
    meshes       = []
    nodes        = []
    materials    = []
    textures     = []
    images       = []
    samplers     = [{"magFilter": 9729, "minFilter": 9987,
                     "wrapS": 10497, "wrapT": 10497}]

    def add_bv(data_bytes, target=None):
        # 4-byte align
        pad = (-len(bin_buf)) % 4
        bin_buf.extend(b'\x00' * pad)
        offset = len(bin_buf)
        bin_buf.extend(data_bytes)
        bv = {"buffer": 0, "byteOffset": offset, "byteLength": len(data_bytes)}
        if target:
            bv["target"] = target
        buffer_views.append(bv)
        return len(buffer_views) - 1

    # ── Textures ──────────────────────────────────────────────────────────────
    mat_index = {}   # ostype_str -> material index

    for tex_id, (mime, img_bytes, has_alpha) in textures_map.items():
        img_idx = len(images)
        images.append({"bufferView": add_bv(img_bytes), "mimeType": mime})
        tex_idx = len(textures)
        textures.append({"source": img_idx, "sampler": 0})
        mat_idx = len(materials)
        mat = {
            "name": tex_id,
            "pbrMetallicRoughness": {
                "baseColorTexture": {"index": tex_idx},
                "metallicFactor": 0.0,
                "roughnessFactor": 1.0,
            },
            "doubleSided": True,
        }
        if has_alpha:
            mat["alphaMode"] = "BLEND"
        materials.append(mat)
        mat_index[tex_id] = mat_idx

    default_mat = len(materials)
    materials.append({
        "name": "default",
        "pbrMetallicRoughness": {"metallicFactor": 0.0, "roughnessFactor": 1.0},
        "doubleSided": True,
    })

    # ── Meshes ────────────────────────────────────────────────────────────────
    for sd in shapes_data:
        info      = sd['info']
        vertices  = sd['vertices']
        triangles = sd['triangles']
        normals   = sd.get('normals')
        n  = len(vertices)
        tc = len(triangles)

        # Positions (little-endian float32 XYZ)
        pos_bytes = bytearray(n * 12)
        pos_min = [float('inf')] * 3
        pos_max = [float('-inf')] * 3
        for i, (x, y, z, u, v) in enumerate(vertices):
            struct.pack_into('<fff', pos_bytes, i * 12, x, y, z)
            if x < pos_min[0]: pos_min[0] = x
            if y < pos_min[1]: pos_min[1] = y
            if z < pos_min[2]: pos_min[2] = z
            if x > pos_max[0]: pos_max[0] = x
            if y > pos_max[1]: pos_max[1] = y
            if z > pos_max[2]: pos_max[2] = z

        # Tex-coords — flip V: QuickDraw 3D has V=0 at bottom, glTF has V=0 at top
        uv_bytes = bytearray(n * 8)
        for i, (x, y, z, u, v) in enumerate(vertices):
            struct.pack_into('<ff', uv_bytes, i * 8, u, 1.0 - v)

        # Indices (little-endian uint32)
        idx_bytes = bytearray(tc * 12)
        for i, (a, b, c) in enumerate(triangles):
            struct.pack_into('<III', idx_bytes, i * 12, a, b, c)

        bv_pos = add_bv(bytes(pos_bytes), ARRAY_BUFFER)
        bv_uv  = add_bv(bytes(uv_bytes),  ARRAY_BUFFER)
        bv_idx = add_bv(bytes(idx_bytes), ELEMENT_ARRAY_BUFFER)

        acc_pos = len(accessors)
        accessors.append({
            "bufferView": bv_pos, "byteOffset": 0,
            "componentType": FLOAT, "count": n, "type": "VEC3",
            "min": pos_min, "max": pos_max,
        })
        acc_uv = len(accessors)
        accessors.append({
            "bufferView": bv_uv, "byteOffset": 0,
            "componentType": FLOAT, "count": n, "type": "VEC2",
        })
        acc_idx = len(accessors)
        accessors.append({
            "bufferView": bv_idx, "byteOffset": 0,
            "componentType": UNSIGNED_INT, "count": tc * 3, "type": "SCALAR",
        })

        attrs = {"POSITION": acc_pos, "TEXCOORD_0": acc_uv}

        if normals:
            # Skip normals if any are zero-length (glTF requires unit vectors)
            has_zero = any(nx*nx + ny*ny + nz*nz == 0.0 for (nx, ny, nz) in normals)
            if not has_zero:
                nrm_bytes = bytearray(len(normals) * 12)
                for i, (nx, ny, nz) in enumerate(normals):
                    struct.pack_into('<fff', nrm_bytes, i * 12, nx, ny, nz)
                bv_nrm = add_bv(bytes(nrm_bytes), ARRAY_BUFFER)
                acc_nrm = len(accessors)
                accessors.append({
                    "bufferView": bv_nrm, "byteOffset": 0,
                    "componentType": FLOAT, "count": len(normals), "type": "VEC3",
                })
                attrs["NORMAL"] = acc_nrm

        mesh_idx = len(meshes)
        meshes.append({
            "name": info['name'],
            "primitives": [{
                "attributes": attrs,
                "indices": acc_idx,
                "material": mat_index.get(info['texture'], default_mat),
            }],
        })

        # Node matrix: WaterRace row-vectors → glTF column-major
        # WaterRace Matrix rows: x=(x.x,x.y,x.z), y=..., z=..., w=(tx,ty,tz)
        # glTF column-major: [x.x,x.y,x.z,0, y.x,y.y,y.z,0, z.x,z.y,z.z,0, tx,ty,tz,1]
        mx, my, mz, mw = info['matrix']
        mat16 = [
            mx[0], mx[1], mx[2], 0.0,
            my[0], my[1], my[2], 0.0,
            mz[0], mz[1], mz[2], 0.0,
            mw[0], mw[1], mw[2], 1.0,
        ]
        identity = [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]
        node = {"name": info['name'], "mesh": mesh_idx}
        if mat16 != identity:
            node["matrix"] = mat16
        nodes.append(node)

    # ── glTF JSON ─────────────────────────────────────────────────────────────
    gltf = {
        "asset": {"version": "2.0", "generator": "metafile_to_gltf/WaterRace"},
        "scene": 0,
        "scenes": [{"name": "Scene", "nodes": list(range(len(nodes)))}],
        "nodes":        nodes,
        "meshes":       meshes,
        "materials":    materials,
        "accessors":    accessors,
        "bufferViews":  buffer_views,
        "buffers":      [{"byteLength": len(bin_buf)}],
    }
    if samplers: gltf["samplers"] = samplers
    if textures: gltf["textures"] = textures
    if images:   gltf["images"]   = images

    return _pack_glb(gltf, bytes(bin_buf))


def _pack_glb(gltf_json, bin_data):
    """Pack glTF dict + binary chunk into GLB bytes."""
    json_bytes = json.dumps(gltf_json, separators=(',', ':')).encode('utf-8')
    # Pad to 4-byte boundary (JSON: spaces, binary: zeros)
    pad_j = (-len(json_bytes)) % 4
    json_bytes += b' ' * pad_j
    pad_b = (-len(bin_data)) % 4
    bin_data += b'\x00' * pad_b

    total = 12 + 8 + len(json_bytes) + 8 + len(bin_data)
    glb = bytearray()
    glb += struct.pack('<III', 0x46546C67, 2, total)          # header
    glb += struct.pack('<II',  len(json_bytes), 0x4E4F534A)   # JSON chunk
    glb += json_bytes
    glb += struct.pack('<II',  len(bin_data),   0x004E4942)   # BIN chunk
    glb += bin_data
    return bytes(glb)


# ── Matrix math ──────────────────────────────────────────────────────────────

# WaterRace Matrix = list of 4 rows, each a 3-tuple (x,y,z).
# Rows 0-2 are the basis vectors; row 3 is the translation.
# Convention: row-vector * matrix  (v' = v * M)

IDENTITY_WM = [(1.,0.,0.), (0.,1.,0.), (0.,0.,1.), (0.,0.,0.)]

def wm_multiply(A, B):
    """Concatenate two WaterRace matrices: apply A first, then B.
    Equivalent to Matrix_Cat(&A, &B, &result) in the engine."""
    a = [[A[r][c] for c in range(3)] + [0. if r < 3 else 1.]  for r in range(4)]
    b = [[B[r][c] for c in range(3)] + [0. if r < 3 else 1.]  for r in range(4)]
    c = [[sum(a[i][k] * b[k][j] for k in range(4)) for j in range(4)]
         for i in range(4)]
    return [(c[r][0], c[r][1], c[r][2]) for r in range(4)]


# ── Shape geometry helpers ────────────────────────────────────────────────────

def _collect_shape_children(entries, shap_idx):
    """Return (pnts_e, trig_e, nrls_e) immediately following a shap entry."""
    pnts_e = trig_e = nrls_e = None
    for e in entries[shap_idx + 1:]:
        if e['tag'] != '<-->':
            break
        if   e['subtag'] == 'pnts': pnts_e = e
        elif e['subtag'] == 'trig': trig_e = e
        elif e['subtag'] == 'nrls': nrls_e = e
    return pnts_e, trig_e, nrls_e


def _load_shape(data, entries, shap_idx, override_matrix=None):
    """Parse one shap entry and its geometry children.
    Returns a shapes_data dict, or None if geometry is missing."""
    entry = entries[shap_idx]
    info  = parse_shape_header(data, entry['pos'])
    pc, tc = info['point_count'], info['tri_count']

    pnts_e, trig_e, nrls_e = _collect_shape_children(entries, shap_idx)
    if pnts_e is None or trig_e is None:
        return None

    if override_matrix is not None:
        info = dict(info)          # shallow copy so we don't mutate shared data
        info['matrix'] = override_matrix

    return {
        'info':      info,
        'vertices':  parse_vertices(data, pnts_e['pos'], pc),
        'triangles': parse_triangles(data, trig_e['pos'], tc),
        'normals':   parse_normals(data, nrls_e['pos'], pc) if nrls_e else None,
    }


# ── Terrain mesh geometry helper ──────────────────────────────────────────────

# TerrainMesh / SeaMesh struct offsets (PPC, 4-byte aligned):
#   0   Str31 name (32)
#  32   OSType id (4)
#  36   ulong flags (4)
#  40   Matrix pos (48)
#  88   float rotX/Y/Z (12)
# 100   float scaleX/Y/Z (12)
# 112   long pointCount (4)
# 116   VertexPtr pointList (4)
# 120   LightningPtr lightningList (4)
# 124   long triangleCount (4)
# 128   TriFacePtr triangleList (4)
# 132   Vector boundingBox[8] (96)
# 228   long clipping (4)
# 232   OSType texture (4)
# 236   float diffuse (4)
# 240   VectorPtr normalList  [tmsh]  /  float alpha [smsh]  (4)
# 244   TQATexture* texturePtr (4)
# 248   char unusedArray[252] (252)
# 500   ulong referenceCount (4)    ← same offset for both tmsh and smsh
# 504   ModelReference referenceList[128] (96 bytes each)

# ModelReference struct offsets (96 bytes):
#   0   OSType id (4)
#   4   ulong flags (4)
#   8   long visibility (4)
#  12   Matrix pos (48)   ← world-space placement matrix
#  60   float rotX/Y/Z (12)
#  72   float scale (4)
#  76   float inertia (4)
#  80   long unused[4] (16)

MODEL_REF_SIZE    = 96
MODEL_REF_POS_OFF = 12   # Matrix pos within ModelReference

# tmsh (TerrainMesh) ends unusedArray at offset 248+252=500, refcount at 500
# smsh (SeaMesh) has an extra 'lastUpdateTime' field, refcount at 504
MESH_REFCOUNT_OFF = {12792: 500, 12796: 504}
MESH_REFLIST_OFF  = {12792: 504, 12796: 508}


def _parse_mesh_references(mesh_bytes):
    """Return list of (ref_id_str, wm_matrix) from a tmsh/smsh struct blob."""
    size = len(mesh_bytes)
    rc_off = MESH_REFCOUNT_OFF.get(size, 500)
    rl_off = MESH_REFLIST_OFF.get(size, 504)
    ref_count = struct.unpack_from('>I', mesh_bytes, rc_off)[0]
    if ref_count > 128:   # sanity guard
        return []
    refs = []
    for i in range(ref_count):
        off = rl_off + i * MODEL_REF_SIZE
        ref_id = mesh_bytes[off:off+4].decode('mac_roman', errors='replace')
        mat_off = off + MODEL_REF_POS_OFF
        matrix = [struct.unpack_from('>fff', mesh_bytes, mat_off + r * 12)
                  for r in range(4)]
        refs.append((ref_id, matrix))
    return refs


def _parse_mesh_geometry(data, entries, mesh_entry_idx):
    """Return (pointCount, triangleCount, texture, pnts_e, trig_e, nrls_e)
    for a tmsh/smsh entry, reading pointCount/triangleCount/texture from the
    struct and geometry child tags from the tag table."""
    e = entries[mesh_entry_idx]
    d = data[e['pos']:e['pos'] + e['size']]
    pc  = struct.unpack_from('>i', d, 112)[0]
    tc  = struct.unpack_from('>i', d, 124)[0]
    tex = d[232:236].decode('mac_roman', errors='replace')

    pnts_e = trig_e = nrls_e = None
    for ce in entries[mesh_entry_idx + 1:]:
        if ce['tag'] != '<-->':
            break
        if   ce['subtag'] == 'pnts': pnts_e = ce
        elif ce['subtag'] == 'trig': trig_e = ce
        elif ce['subtag'] == 'nrls': nrls_e = ce

    return pc, tc, tex, pnts_e, trig_e, nrls_e


# ── Main ─────────────────────────────────────────────────────────────────────

# CompressedTexture struct layout (big-endian PPC, natural alignment):
#   0  OSType         name           (4)
#   4  unsigned long  flags          (4)
#   8  unsigned short width          (2)
#  10  unsigned short height         (2)
#  12  unsigned short depth          (2)
#  14  [2 pad to align PicHandle]
#  16  PicHandle      image          (4)  ← heap ptr, meaningless on disk
#  20  unsigned long  uncompressedSize (4)
#  24  short          alphaDepth     (2)  0=none, 1=1-bit, 8=8-bit
#  26  unsigned char  alphaLayerData[]   ← width*height bytes if alphaDepth≠0

def _parse_cptx_alpha(cptx_data):
    """Return (width, height, alpha_depth, alpha_bytes_or_None) from cptx entry data."""
    if len(cptx_data) < 26:
        return 0, 0, 0, None
    width        = struct.unpack_from('>H', cptx_data, 8)[0]
    height       = struct.unpack_from('>H', cptx_data, 10)[0]
    alpha_depth  = struct.unpack_from('>h', cptx_data, 24)[0]
    if alpha_depth != 0 and width > 0 and height > 0:
        expected = width * height
        if len(cptx_data) >= 26 + expected:
            return width, height, alpha_depth, bytes(cptx_data[26:26 + expected])
    return width, height, alpha_depth, None


def _apply_alpha_layer(rgb_bytes, width, height, alpha_data, alpha_depth):
    """Merge alpha_data into an RGB image, returning RGBA PNG bytes."""
    from PIL import Image
    import io
    img = Image.frombytes('RGB', (width, height), rgb_bytes)
    alpha_img = Image.new('L', (width, height))
    for idx in range(width * height):
        a = alpha_data[idx]
        alpha_img.putpixel((idx % width, idx // width),
                           255 if (alpha_depth == 1 and a > 127) else a)
    img.putalpha(alpha_img)
    buf = io.BytesIO()
    img.save(buf, format='PNG')
    return buf.getvalue()


def _decode_jpeg_to_rgb(jpeg_bytes, width, height):
    """Decode JPEG bytes, resize to (width, height), return raw RGB bytes."""
    from PIL import Image
    import io
    img = Image.open(io.BytesIO(jpeg_bytes)).convert('RGB')
    if img.size != (width, height):
        img = img.resize((width, height), Image.LANCZOS)
    return img.tobytes()


def _collect_textures(data, entries):
    textures_map = {}
    for i, e in enumerate(entries):
        if e['tag'] != 'cptx' or i + 1 >= len(entries):
            continue
        nxt = entries[i + 1]
        if nxt['tag'] != '<-->' or nxt['subtag'] != 'txim':
            continue
        tex_id = e['id']
        pict = data[nxt['pos']:nxt['pos'] + nxt['size']]

        # Parse alpha info from the CompressedTexture struct in the cptx entry
        cptx_data = data[e['pos']:e['pos'] + e['size']]
        tex_w, tex_h, alpha_depth, alpha_data = _parse_cptx_alpha(cptx_data)

        try:
            mime, img, has_alpha = pict_to_image(pict)

            # For JPEG textures with a separate alpha layer, decode and merge
            if not has_alpha and alpha_depth != 0 and alpha_data is not None:
                try:
                    rgb = _decode_jpeg_to_rgb(img, tex_w, tex_h)
                    img = _apply_alpha_layer(rgb, tex_w, tex_h, alpha_data, alpha_depth)
                    mime, has_alpha = 'image/png', True
                except Exception as ae:
                    print(f'  WARNING: could not apply alpha to {tex_id!r}: {ae}')

            textures_map[tex_id] = (mime, img, has_alpha)
            print(f'  Texture {tex_id!r}: {mime} ({len(img):,} bytes)'
                  + (' [alpha]' if has_alpha else ''))
        except Exception as exc:
            print(f'  WARNING: could not decode texture {tex_id!r}: {exc}')
    return textures_map


def _extract_mega_shapes(data, entries):
    """For ship/character files: collect all shap entries with their own matrices."""
    shapes_data = []
    for idx, entry in enumerate(entries):
        if entry['tag'] != 'shap':
            continue
        sd = _load_shape(data, entries, idx)
        if sd is None:
            info = parse_shape_header(data, entry['pos'])
            print(f'  Skipping shape {info["name"]!r}: no geometry')
            continue
        info = sd['info']
        print(f'  Shape {info["name"]!r}: {info["point_count"]} verts, '
              f'{info["tri_count"]} tris, texture={info["texture"]!r}')
        shapes_data.append(sd)
    return shapes_data


def _extract_terrain_shapes(data, entries):
    """For level/terrain files: place mega objects at their tmsh/smsh reference
    positions, and also include the terrain-mesh ground geometry."""
    shapes_data = []

    # ── Pass 1: build mega_id → [shape_data_template, ...] ───────────────────
    # Shape matrices here are LOCAL to the MegaObject.
    mega_shapes = {}   # mega_id -> list of shape dicts (geometry + local matrix)
    for idx, entry in enumerate(entries):
        if entry['tag'] != 'mega':
            continue
        mega_id    = entry['id']
        mega_depth = entry['depth']
        local_list = []
        for cidx in range(idx + 1, len(entries)):
            ce = entries[cidx]
            if ce['depth'] <= mega_depth:
                break
            if ce['tag'] != 'shap':
                continue
            sd = _load_shape(data, entries, cidx)
            if sd is not None:
                local_list.append(sd)
        mega_shapes[mega_id] = local_list

    # ── Pass 2: walk tmsh/smsh entries ───────────────────────────────────────
    for idx, entry in enumerate(entries):
        if entry['tag'] not in ('tmsh', 'smsh'):
            continue

        mesh_bytes = data[entry['pos']:entry['pos'] + entry['size']]

        # 2a. Terrain-mesh ground geometry (vertices already in world space)
        pc, tc, tex, pnts_e, trig_e, nrls_e = _parse_mesh_geometry(
            data, entries, idx)

        if pc > 0 and tc > 0 and pnts_e is not None and trig_e is not None:
            nlen = mesh_bytes[0]
            patch_name = mesh_bytes[1:1+nlen].decode('mac_roman', errors='replace')
            # Build a synthetic shape_info for the terrain patch
            fake_info = {
                'name':        patch_name or entry['tag'],
                'id':          entry['id'],
                'point_count': pc,
                'tri_count':   tc,
                'texture':     tex,
                'matrix':      IDENTITY_WM,   # verts are in world space
                'normal_mode': 0,
                'flags':       0,
                'translation': (0., 0., 0.),
                'link':        -1,
                'shading':     0,
            }
            verts = parse_vertices(data, pnts_e['pos'], pc)
            tris  = parse_triangles(data, trig_e['pos'], tc)
            nrls  = (parse_normals(data, nrls_e['pos'], pc)
                     if nrls_e and nrls_e['size'] > 0 else None)
            print(f'  Terrain patch {patch_name!r}: {pc} verts, {tc} tris,'
                  f' texture={tex!r}')
            shapes_data.append({
                'info': fake_info, 'vertices': verts,
                'triangles': tris, 'normals': nrls,
            })

        # 2b. Placed MegaObject instances
        refs = _parse_mesh_references(mesh_bytes)
        if refs:
            ref_ids = [r[0] for r in refs]
            print(f'  Mesh refs ({len(refs)}): {ref_ids[:6]}'
                  f'{"…" if len(refs) > 6 else ""}')
        for ref_id, ref_matrix in refs:
            for local_sd in mega_shapes.get(ref_id, []):
                # Combined world transform: shape-local matrix × reference matrix
                new_info = dict(local_sd['info'])
                new_info['matrix'] = wm_multiply(local_sd['info']['matrix'],
                                                  ref_matrix)
                shapes_data.append({
                    'info':      new_info,
                    'vertices':  local_sd['vertices'],
                    'triangles': local_sd['triangles'],
                    'normals':   local_sd['normals'],
                })

    return shapes_data


def convert(metafile_path):
    with open(metafile_path, 'rb') as f:
        data = f.read()

    # Too small to be a MetaFile header — not a 3D asset, skip silently.
    if len(data) < 16:
        print(f'Skipping {os.path.basename(metafile_path)}: not a MetaFile '
              f'({len(data)} bytes)', file=sys.stderr)
        sys.exit(0)

    try:
        entries = parse_tag_table(data)
    except Exception as exc:
        print(f'Error: could not parse tag table in '
              f'{os.path.basename(metafile_path)}: {exc}', file=sys.stderr)
        sys.exit(1)

    # ── 1. Textures ───────────────────────────────────────────────────────────
    textures_map = _collect_textures(data, entries)

    # ── 2. Shapes — detect terrain vs. object file ────────────────────────────
    is_terrain = entries and entries[0]['tag'] == 'terr'
    if is_terrain:
        print(f'  (terrain/level file — resolving ModelReference placements)')
        shapes_data = _extract_terrain_shapes(data, entries)
    else:
        shapes_data = _extract_mega_shapes(data, entries)

    if not shapes_data:
        print(f'No shapes found in {os.path.basename(metafile_path)} '
              f'— skipping.', file=sys.stderr)
        sys.exit(0)

    # ── 3. Build & write GLB ──────────────────────────────────────────────────
    try:
        glb = build_glb(shapes_data, textures_map)
    except Exception as exc:
        print(f'Error: GLB build failed for '
              f'{os.path.basename(metafile_path)}: {exc}', file=sys.stderr)
        sys.exit(1)

    base_name = os.path.splitext(os.path.basename(metafile_path))[0]
    out = base_name + '.glb'
    with open(out, 'wb') as f:
        f.write(glb)
    print(f'\nWrote {len(glb):,} bytes → {out}')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f'Usage: {sys.argv[0]} <metafile>', file=sys.stderr)
        sys.exit(1)
    convert(sys.argv[1])
