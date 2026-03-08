#!/usr/bin/env python3
"""
extract_interface_sounds.py — Extract PICT images and snd sounds from
WaterRace Data/Interface.rsrc_fork and WaterRace Data/Sounds.rsrc_fork.

Saves:
  _OSX_PORT_/Interface/  — one PNG or JPEG per PICT resource (named by resource ID)
  _OSX_PORT_/Sounds/     — one WAV per snd resource (named by resource name or ID)

Usage:
    python3 extract_interface_sounds.py
  or:
    python3 extract_interface_sounds.py <path-to-WaterRace-Data-folder>
"""

import os
import struct
import sys
import zlib

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

def find_data_folder(argv):
    if len(argv) > 1:
        return argv[1]
    # Auto-detect relative to script location
    candidates = [
        os.path.join(os.path.dirname(SCRIPT_DIR), 'WaterRace Data'),
        os.path.join(SCRIPT_DIR, '..', 'WaterRace Data'),
    ]
    for c in candidates:
        if os.path.isdir(c):
            return os.path.normpath(c)
    return None


# ---------------------------------------------------------------------------
# Mac OS Resource Fork parser
# ---------------------------------------------------------------------------

def parse_resource_fork(data: bytes):
    """Parse a Mac OS resource fork. Returns a list of resource dicts:
      {'type': bytes4, 'id': int, 'name': str, 'data': bytes}
    """
    if len(data) < 16:
        raise ValueError("Resource fork too small")

    data_offset, map_offset, data_len, map_len = struct.unpack_from('>LLLL', data, 0)

    # Resource map
    type_list_off = struct.unpack_from('>H', data, map_offset + 24)[0]
    name_list_off = struct.unpack_from('>H', data, map_offset + 26)[0]

    tl_start = map_offset + type_list_off
    num_types = struct.unpack_from('>H', data, tl_start)[0] + 1

    resources = []

    for i in range(num_types):
        te_off = tl_start + 2 + i * 8
        rtype = data[te_off:te_off + 4]
        count = struct.unpack_from('>H', data, te_off + 4)[0] + 1
        ref_list_rel = struct.unpack_from('>H', data, te_off + 6)[0]
        rl_start = tl_start + ref_list_rel  # ref list offset is from tl_start

        for j in range(count):
            re_off = rl_start + j * 12
            res_id = struct.unpack_from('>h', data, re_off)[0]
            name_off_val = struct.unpack_from('>H', data, re_off + 2)[0]

            # 3-byte data offset packed in bytes 4-7 (high byte = attributes)
            data_off_raw = struct.unpack_from('>I', data, re_off + 4)[0]
            data_off_in_section = data_off_raw & 0x00FFFFFF

            abs_off = data_offset + data_off_in_section
            res_len = struct.unpack_from('>L', data, abs_off)[0]
            res_data = data[abs_off + 4: abs_off + 4 + res_len]

            # Name
            if name_off_val != 0xFFFF:
                name_abs = map_offset + name_list_off + name_off_val
                nlen = data[name_abs]
                name = data[name_abs + 1: name_abs + 1 + nlen].decode('mac_roman', errors='replace')
            else:
                name = ''

            resources.append({
                'type': rtype,
                'id': res_id,
                'name': name,
                'data': res_data,
            })

    return resources


# ---------------------------------------------------------------------------
# PNG writer (no external dependencies)
# ---------------------------------------------------------------------------

def _png_chunk(tag: bytes, payload: bytes) -> bytes:
    crc = zlib.crc32(tag + payload) & 0xFFFFFFFF
    return struct.pack('>I', len(payload)) + tag + payload + struct.pack('>I', crc)


def write_png(path: str, width: int, height: int, pixels: bytes, has_alpha: bool):
    """Write a minimal PNG file. pixels: raw RGB or RGBA bytes, row-major."""
    color_type = 6 if has_alpha else 2
    channels = 4 if has_alpha else 3

    raw = bytearray()
    row_stride = width * channels
    for y in range(height):
        raw.append(0)  # filter: None
        raw.extend(pixels[y * row_stride: (y + 1) * row_stride])

    ihdr = struct.pack('>IIBBBBB', width, height, 8, color_type, 0, 0, 0)
    idat = zlib.compress(bytes(raw), 6)

    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n')
        f.write(_png_chunk(b'IHDR', ihdr))
        f.write(_png_chunk(b'IDAT', idat))
        f.write(_png_chunk(b'IEND', b''))


# ---------------------------------------------------------------------------
# PackBits decompression (used by DirectBitsRect PICT)
# ---------------------------------------------------------------------------

def unpack_bits(data: bytes, expected_len: int, word_mode: bool = False) -> bytes:
    """Decompress PackBits data. word_mode=True for 16-bit pixel rows."""
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
# PICT decoder
# ---------------------------------------------------------------------------

def _parse_pixmap(data: bytes, offset: int):
    """Parse a 50-byte PixMap record. Returns (info, new_offset)."""
    fmt = '>LHhhhhhhLLLhhhhlLL'
    fields = struct.unpack_from(fmt, data, offset)
    info = {
        'row_bytes': fields[1] & 0x7FFF,
        'top': fields[2], 'left': fields[3], 'bottom': fields[4], 'right': fields[5],
        'pack_type': fields[7],
        'pixel_size': fields[12],
        'cmp_count': fields[13],
        'cmp_size': fields[14],
    }
    return info, offset + struct.calcsize(fmt)


def _decode_direct_bits_rect(data: bytes, offset: int, pixel_data_start: int = None):
    """Decode DirectBitsRect (0x009A) or DirectBitsRgn (0x009B).
    pixel_data_start: if given, absolute offset to the first compressed row
    (used for 0x009B to skip the region that comes after srcRect/dstRect/mode).
    Returns (pixels, w, h, has_alpha)."""
    pm, after_pm = _parse_pixmap(data, offset)

    row_bytes = pm['row_bytes']
    width  = pm['right'] - pm['left']
    height = pm['bottom'] - pm['top']
    pack_type  = pm['pack_type']
    pixel_size = pm['pixel_size']
    cmp_count  = pm['cmp_count']

    if pixel_data_start is not None:
        offset = pixel_data_start
    else:
        # Skip srcRect(8) + dstRect(8) + mode(2)
        offset = after_pm + 18

    long_prefix = row_bytes > 250

    if pack_type == 4 and pixel_size == 32 and cmp_count in (3, 4):
        uncompressed_row = width * cmp_count
        rows = []
        for _ in range(height):
            if long_prefix:
                byte_count = struct.unpack_from('>H', data, offset)[0]; offset += 2
            else:
                byte_count = data[offset]; offset += 1
            row_raw = unpack_bits(data[offset:offset + byte_count], uncompressed_row)
            offset += byte_count
            if cmp_count == 3:
                rows.append((None, row_raw[0:width], row_raw[width:2*width], row_raw[2*width:3*width]))
            else:
                rows.append((row_raw[0:width], row_raw[width:2*width], row_raw[2*width:3*width], row_raw[3*width:4*width]))

        has_alpha = cmp_count == 4 and any(
            a != 0 for A, _, _, _ in rows if A is not None for a in A
        )
        pixels = bytearray()
        for A, R, G, B in rows:
            if has_alpha:
                for x in range(width):
                    pixels.extend((R[x], G[x], B[x], A[x]))
            else:
                for x in range(width):
                    pixels.extend((R[x], G[x], B[x]))
        return bytes(pixels), width, height, has_alpha

    elif pack_type == 3 and pixel_size == 16 and cmp_count == 3:
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
            f"Unsupported DirectBitsRect: packType={pack_type} "
            f"pixelSize={pixel_size} cmpCount={cmp_count}"
        )


def _decode_pack_bits_rect(data: bytes, offset: int):
    """Decode PackBitsRect (0x0098) or PackBitsRgn (0x0099, region already removed from data).
    offset points to the rowBytes field. Returns (pixels, w, h, has_alpha=False).

    PackBitsRect layout after the opcode:
      rowBytes(2) [bit15=1 means PixMap, else BitMap]
      If PixMap (no baseAddr in this opcode format):
        bounds(8) pmVersion(2) packType(2) packSize(4) hRes(4) vRes(4)
        pixelType(2) pixelSize(2) cmpCount(2) cmpSize(2) planeBytes(4) pmTable(4) pmReserved(4)
        = 44 bytes of PixMap body after rowBytes
        ColorTable: ctSeed(4) ctFlags(2) ctSize(2) + (ctSize+1)*{value(2) r(2) g(2) b(2)}
      srcRect(8) dstRect(8) mode(2) = 18 bytes
      Compressed pixel rows
    """
    row_bytes_raw = struct.unpack_from('>H', data, offset)[0]
    is_pixmap = bool(row_bytes_raw & 0x8000)
    row_bytes = row_bytes_raw & 0x7FFF
    offset += 2

    if is_pixmap:
        # PixMap body (44 bytes, no baseAddr):
        # bounds: top(2) left(2) bottom(2) right(2)          @ +0
        # pmVersion(2) packType(2) packSize(4)                @ +8
        # hRes(4) vRes(4)                                     @ +16
        # pixelType(2) pixelSize(2) cmpCount(2) cmpSize(2)   @ +24
        # planeBytes(4) pmTable(4) pmReserved(4)              @ +32
        top        = struct.unpack_from('>h', data, offset + 0)[0]
        left       = struct.unpack_from('>h', data, offset + 2)[0]
        bottom     = struct.unpack_from('>h', data, offset + 4)[0]
        right      = struct.unpack_from('>h', data, offset + 6)[0]
        pixel_size = struct.unpack_from('>h', data, offset + 26)[0]
        offset += 44  # skip entire PixMap body

        # ColorTable: ctSeed(4) ctFlags(2) ctSize(2) then (ctSize+1) entries of 8 bytes
        offset += 4 + 2  # ctSeed, ctFlags
        ct_size = struct.unpack_from('>H', data, offset)[0]; offset += 2  # entries - 1
        palette = {}
        for _ in range(ct_size + 1):
            value = struct.unpack_from('>H', data, offset)[0]; offset += 2
            r = struct.unpack_from('>H', data, offset)[0] >> 8; offset += 2
            g = struct.unpack_from('>H', data, offset)[0] >> 8; offset += 2
            b = struct.unpack_from('>H', data, offset)[0] >> 8; offset += 2
            palette[value & 0xFF] = (r, g, b)
    else:
        # BitMap: bounds only (8 bytes), no pixel_size/palette fields
        top    = struct.unpack_from('>h', data, offset + 0)[0]
        left   = struct.unpack_from('>h', data, offset + 2)[0]
        bottom = struct.unpack_from('>h', data, offset + 4)[0]
        right  = struct.unpack_from('>h', data, offset + 6)[0]
        pixel_size = 1
        offset += 8
        palette = {0: (255, 255, 255), 1: (0, 0, 0)}

    width  = right - left
    height = bottom - top

    # srcRect(8) + dstRect(8) + mode(2)
    offset += 18

    long_prefix = row_bytes > 250
    pixels = bytearray()

    for _ in range(height):
        if long_prefix:
            byte_count = struct.unpack_from('>H', data, offset)[0]; offset += 2
        else:
            byte_count = data[offset]; offset += 1
        row_raw = unpack_bits(data[offset:offset + byte_count], row_bytes)
        offset += byte_count

        if pixel_size == 8:
            for x in range(width):
                idx = row_raw[x]
                r, g, b = palette.get(idx, (0, 0, 0))
                pixels.extend((r, g, b))
        elif pixel_size == 4:
            for x in range(width):
                if x % 2 == 0:
                    idx = (row_raw[x // 2] >> 4) & 0x0F
                else:
                    idx = row_raw[x // 2] & 0x0F
                r, g, b = palette.get(idx, (0, 0, 0))
                pixels.extend((r, g, b))
        else:
            # 1-bit: 8 pixels per byte
            for x in range(width):
                bit = (row_raw[x // 8] >> (7 - (x % 8))) & 1
                r, g, b = palette.get(bit, (0, 0, 0))
                pixels.extend((r, g, b))

    return bytes(pixels), width, height, False


def _extract_jpeg_from_compressed_quicktime(pict_data: bytes):
    """If the PICT contains a CompressedQuickTime (0x8200) opcode with JPEG,
    extract and return the JPEG bytes. Otherwise return None."""
    if len(pict_data) < 12:
        return None
    offset = 10  # skip picSize(2) + picFrame(8)
    while offset < len(pict_data) - 2:
        try:
            op = struct.unpack_from('>H', pict_data, offset)[0]
        except struct.error:
            return None
        offset += 2
        if op == 0x0011:
            offset += 2
        elif op == 0x0C00:
            offset += 24
        elif op in (0x001E, 0x00FF):
            pass
        elif op == 0x0001:
            sz = struct.unpack_from('>H', pict_data, offset)[0]; offset += sz
        elif op == 0x00A1:
            offset += 2
            sz = struct.unpack_from('>H', pict_data, offset)[0]; offset += 2 + sz
        elif op == 0x8200:
            qt_size = struct.unpack_from('>L', pict_data, offset)[0]
            qt_data = pict_data[offset + 4: offset + 4 + qt_size]
            soi = qt_data.find(b'\xff\xd8')
            eoi = qt_data.rfind(b'\xff\xd9')
            if soi >= 0 and eoi > soi:
                return qt_data[soi: eoi + 2]
            return None
        elif op == 0x009A:
            return None  # DirectBitsRect — not a JPEG
        else:
            return None
    return None


def decode_pict(pict_data: bytes):
    """Decode a PICT v2 resource. Returns (pixels, width, height, has_alpha)."""
    if len(pict_data) < 16:
        raise ValueError("PICT data too small")

    offset = 10  # skip picSize(2) + picFrame(8)
    while offset < len(pict_data) - 2:
        op = struct.unpack_from('>H', pict_data, offset)[0]
        offset += 2

        if op == 0x0011:
            offset += 2
        elif op == 0x0C00:
            offset += 24
        elif op in (0x001E,):
            pass
        elif op == 0x0001:
            sz = struct.unpack_from('>H', pict_data, offset)[0]; offset += sz
        elif op == 0x00A1:
            offset += 2
            sz = struct.unpack_from('>H', pict_data, offset)[0]; offset += 2 + sz
        elif op == 0x009A:
            return _decode_direct_bits_rect(pict_data, offset)
        elif op == 0x009B:
            # DirectBitsRgn: same as DirectBitsRect but with a clipping region prepended
            # after PixMap(50) + srcRect(8) + dstRect(8) + mode(2) = 68 bytes, then region
            rgn_offset = offset + 68
            rgn_size = struct.unpack_from('>H', pict_data, rgn_offset)[0]
            # Skip the region entirely — decode the pixel data that follows
            pixel_offset = rgn_offset + rgn_size
            return _decode_direct_bits_rect(pict_data, offset, pixel_data_start=pixel_offset)
        elif op == 0x0098:
            # PackBitsRect: palette-indexed image
            return _decode_pack_bits_rect(pict_data, offset)
        elif op == 0x0099:
            # PackBitsRgn: palette-indexed with a clipping region
            # rowBytes at offset, then PixMap/BitMap header, then ColorTable,
            # then srcRect(8)+dstRect(8)+mode(2), then region, then pixel data.
            # Easiest: find and skip the region, then parse like PackBitsRect.
            # Region is at: offset + 2 (rowBytes already at offset) + rest of header + CT + 18
            # For PixMap (rowBytes bit15=1): header = 2 (rb) + 44 (pm body) + CT
            #   ColorTable size: 4+2+2 + (ctSize+1)*8
            rb_raw = struct.unpack_from('>H', pict_data, offset)[0]
            is_pm = bool(rb_raw & 0x8000)
            if is_pm:
                # PixMap body without rowBytes = 44 bytes
                ct_start = offset + 2 + 44
                ct_size_val = struct.unpack_from('>H', pict_data, ct_start + 6)[0]
                ct_total = 8 + (ct_size_val + 1) * 8
                rgn_start = ct_start + ct_total + 18  # +srcRect+dstRect+mode
            else:
                rgn_start = offset + 2 + 8 + 18   # BitMap bounds + srcRect+dstRect+mode
            rgn_size = struct.unpack_from('>H', pict_data, rgn_start)[0]
            # Build a modified pict_data with region removed, then call PackBitsRect decoder
            patched = pict_data[:rgn_start] + pict_data[rgn_start + rgn_size:]
            return _decode_pack_bits_rect(patched, offset)
        elif op == 0x8200:
            raise ValueError("CompressedQuickTime — use _extract_jpeg_from_compressed_quicktime")
        elif op == 0x00FF:
            raise ValueError("EndOfPicture without image data")
        else:
            raise ValueError(f"Unknown PICT opcode {op:#06x} at offset {offset - 2:#x}")

    raise ValueError("End of data without image")


def save_pict_resource(res_id: int, pict_data: bytes, out_dir: str) -> str:
    """Convert a PICT resource to PNG or JPEG and save to out_dir.
    Returns a short status string for logging."""
    # Try JPEG extraction first
    jpeg = _extract_jpeg_from_compressed_quicktime(pict_data)
    if jpeg is not None:
        path = os.path.join(out_dir, f'PICT_{res_id}.jpg')
        with open(path, 'wb') as f:
            f.write(jpeg)
        return f'JPEG ({len(jpeg)} bytes) → {os.path.basename(path)}'

    # Try DirectBitsRect decode → PNG
    try:
        pixels, w, h, has_alpha = decode_pict(pict_data)
        path = os.path.join(out_dir, f'PICT_{res_id}.png')
        write_png(path, w, h, pixels, has_alpha)
        mode = 'RGBA' if has_alpha else 'RGB'
        return f'{w}×{h} {mode} PNG → {os.path.basename(path)}'
    except ValueError as e:
        return f'SKIP: {e}'


# ---------------------------------------------------------------------------
# WAV writer
# ---------------------------------------------------------------------------

def write_wav(path: str, sample_rate: int, num_channels: int, sample_width_bytes: int, pcm_data: bytes):
    """Write a minimal WAV file. pcm_data must be little-endian signed PCM."""
    data_len = len(pcm_data)
    with open(path, 'wb') as f:
        # RIFF header
        f.write(b'RIFF')
        f.write(struct.pack('<I', 36 + data_len))
        f.write(b'WAVE')
        # fmt chunk
        f.write(b'fmt ')
        f.write(struct.pack('<I', 16))               # chunk size
        f.write(struct.pack('<H', 1))                # PCM
        f.write(struct.pack('<H', num_channels))
        f.write(struct.pack('<I', sample_rate))
        f.write(struct.pack('<I', sample_rate * num_channels * sample_width_bytes))  # byte rate
        f.write(struct.pack('<H', num_channels * sample_width_bytes))               # block align
        f.write(struct.pack('<H', sample_width_bytes * 8))                          # bits per sample
        # data chunk
        f.write(b'data')
        f.write(struct.pack('<I', data_len))
        f.write(pcm_data)


# ---------------------------------------------------------------------------
# snd resource decoder
# ---------------------------------------------------------------------------

def _find_buffer_cmd(snd_data: bytes, snd_format: int):
    """Locate the bufferCmd (0x8051) in a snd resource.
    Returns the offset within snd_data to the sound header, or None."""
    if snd_format == 1:
        # Format 1: numSynth at offset 2
        num_synth = struct.unpack_from('>H', snd_data, 2)[0]
        cmd_offset = 4 + num_synth * 6  # each synth entry = 2+4 bytes
        num_cmds = struct.unpack_from('>H', snd_data, cmd_offset)[0]
        cmd_offset += 2
    elif snd_format == 2:
        # Format 2: refCount at offset 2, numCmds at offset 4
        num_cmds = struct.unpack_from('>H', snd_data, 4)[0]
        cmd_offset = 6
    else:
        return None

    for _ in range(num_cmds):
        cmd = struct.unpack_from('>H', snd_data, cmd_offset)[0]
        p2  = struct.unpack_from('>L', snd_data, cmd_offset + 4)[0]
        if cmd == 0x8051:  # bufferCmd with data-offset flag
            return p2
        cmd_offset += 8
    return None


def decode_snd_to_wav(snd_data: bytes) -> bytes:
    """Decode a Mac OS 'snd ' resource to WAV bytes.
    Supports: Standard header (8-bit unsigned), Extended header (8 or 16-bit signed).
    Raises ValueError for unsupported formats."""
    if len(snd_data) < 4:
        raise ValueError("snd resource too small")

    snd_format = struct.unpack_from('>H', snd_data, 0)[0]
    if snd_format not in (1, 2):
        raise ValueError(f"Unknown snd format {snd_format}")

    hdr_offset = _find_buffer_cmd(snd_data, snd_format)
    if hdr_offset is None:
        raise ValueError("No bufferCmd found in snd resource")

    # Sound header starts at hdr_offset within snd_data
    encode = snd_data[hdr_offset + 20]

    if encode == 0x00:
        # Standard sound header: 8-bit unsigned mono
        sample_count = struct.unpack_from('>L', snd_data, hdr_offset + 4)[0]
        sr_fixed     = struct.unpack_from('>L', snd_data, hdr_offset + 8)[0]
        sample_rate  = max(1, round(sr_fixed / 65536.0))
        sample_data  = snd_data[hdr_offset + 22: hdr_offset + 22 + sample_count]
        # 8-bit Mac sound is unsigned; WAV 8-bit PCM is also unsigned — pass through
        import io, struct as st
        buf = io.BytesIO()
        write_wav.__wrapped__ = None  # not wrapped; just build bytes
        # Build WAV in memory
        data_len = len(sample_data)
        wav = bytearray()
        wav += b'RIFF'
        wav += st.pack('<I', 36 + data_len)
        wav += b'WAVE'
        wav += b'fmt '
        wav += st.pack('<I', 16)
        wav += st.pack('<H', 1)       # PCM
        wav += st.pack('<H', 1)       # mono
        wav += st.pack('<I', sample_rate)
        wav += st.pack('<I', sample_rate)
        wav += st.pack('<H', 1)       # block align
        wav += st.pack('<H', 8)       # 8-bit
        wav += b'data'
        wav += st.pack('<I', data_len)
        wav += sample_data
        return bytes(wav)

    elif encode == 0xFF:
        # Extended sound header: multi-channel, variable bit depth
        num_channels = struct.unpack_from('>L', snd_data, hdr_offset + 4)[0]
        sr_fixed     = struct.unpack_from('>L', snd_data, hdr_offset + 8)[0]
        sample_rate  = max(1, round(sr_fixed / 65536.0))
        num_frames   = struct.unpack_from('>L', snd_data, hdr_offset + 22)[0]
        sample_size  = struct.unpack_from('>H', snd_data, hdr_offset + 48)[0]  # bits per sample
        sample_bytes = sample_size // 8
        sample_start = hdr_offset + 64
        total_samples = num_frames * num_channels
        raw = snd_data[sample_start: sample_start + total_samples * sample_bytes]

        if sample_size == 8:
            # 8-bit: Mac stores as signed, WAV 8-bit expects unsigned → shift
            pcm = bytes((b + 128) & 0xFF for b in raw)
            width = 1
        elif sample_size == 16:
            # 16-bit big-endian signed → little-endian signed
            n = len(raw) // 2
            pcm = struct.pack('<' + 'h' * n, *struct.unpack_from('>' + 'h' * n, raw))
            width = 2
        else:
            raise ValueError(f"Unsupported sample size {sample_size} bits in extended header")

        data_len = len(pcm)
        wav = bytearray()
        wav += b'RIFF'
        wav += struct.pack('<I', 36 + data_len)
        wav += b'WAVE'
        wav += b'fmt '
        wav += struct.pack('<I', 16)
        wav += struct.pack('<H', 1)            # PCM
        wav += struct.pack('<H', num_channels)
        wav += struct.pack('<I', sample_rate)
        wav += struct.pack('<I', sample_rate * num_channels * width)
        wav += struct.pack('<H', num_channels * width)
        wav += struct.pack('<H', sample_size)
        wav += b'data'
        wav += struct.pack('<I', data_len)
        wav += pcm
        return bytes(wav)

    elif encode == 0xFE:
        # Compressed sound header
        fmt_tag = snd_data[hdr_offset + 40: hdr_offset + 44]
        raise ValueError(
            f"Compressed snd (encode=0xFE, codec={fmt_tag!r}) not supported"
        )
    else:
        raise ValueError(f"Unknown sound header encode byte {encode:#04x}")


def save_snd_resource(res: dict, out_dir: str) -> str:
    """Convert a snd resource to WAV and save to out_dir.
    Returns a short status string."""
    name = res['name'] or f"snd_{res['id']}"
    safe_name = ''.join(c if c.isalnum() or c in ' -_' else '_' for c in name).strip()
    filename = f"{safe_name}.wav"
    path = os.path.join(out_dir, filename)

    try:
        wav_data = decode_snd_to_wav(res['data'])
        with open(path, 'wb') as f:
            f.write(wav_data)
        return f'WAV ({len(wav_data)} bytes) → {filename}'
    except ValueError as e:
        return f'SKIP: {e}'


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    data_folder = find_data_folder(sys.argv)
    if data_folder is None:
        print("Error: could not find 'WaterRace Data' folder.", file=sys.stderr)
        print("Usage: python3 extract_interface_sounds.py [path-to-WaterRace-Data]", file=sys.stderr)
        sys.exit(1)

    interface_rsrc = os.path.join(data_folder, 'Interface.rsrc_fork')
    sounds_rsrc    = os.path.join(data_folder, 'Sounds.rsrc_fork')

    for p in (interface_rsrc, sounds_rsrc):
        if not os.path.isfile(p):
            print(f"Error: file not found: {p!r}", file=sys.stderr)
            sys.exit(1)

    import shutil
    interface_out = os.path.join(SCRIPT_DIR, 'Interface')
    sounds_out    = os.path.join(SCRIPT_DIR, 'Sounds')
    for d in (interface_out, sounds_out):
        if os.path.exists(d):
            shutil.rmtree(d)
        os.makedirs(d)

    # ---- Interface: extract PICT resources ----
    print(f"Reading {interface_rsrc} …")
    with open(interface_rsrc, 'rb') as f:
        iface_data = f.read()

    iface_resources = parse_resource_fork(iface_data)
    picts = [r for r in iface_resources if r['type'] == b'PICT']
    print(f"  Found {len(picts)} PICT resources → {interface_out}")

    ok_count = skip_count = 0
    for r in sorted(picts, key=lambda x: x['id']):
        status = save_pict_resource(r['id'], r['data'], interface_out)
        if status.startswith('SKIP'):
            skip_count += 1
            print(f"  PICT {r['id']:5d}: {status}")
        else:
            ok_count += 1
            print(f"  PICT {r['id']:5d}: {status}")

    print(f"  Interface: {ok_count} saved, {skip_count} skipped\n")

    # ---- Sounds: extract snd resources ----
    print(f"Reading {sounds_rsrc} …")
    with open(sounds_rsrc, 'rb') as f:
        sounds_data = f.read()

    sound_resources = parse_resource_fork(sounds_data)
    snds = [r for r in sound_resources if r['type'] == b'snd ']
    print(f"  Found {len(snds)} snd resources → {sounds_out}")

    ok_count = skip_count = 0
    for r in sorted(snds, key=lambda x: x['id']):
        status = save_snd_resource(r, sounds_out)
        if status.startswith('SKIP'):
            skip_count += 1
            print(f"  snd {r['id']:5d} {repr(r['name'] or ''):30s}: {status}")
        else:
            ok_count += 1
            print(f"  snd {r['id']:5d} {repr(r['name'] or ''):30s}: {status}")

    print(f"  Sounds: {ok_count} saved, {skip_count} skipped")


if __name__ == '__main__':
    main()
