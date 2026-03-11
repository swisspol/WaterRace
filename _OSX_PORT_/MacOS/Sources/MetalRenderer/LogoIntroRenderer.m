#import "MetalRenderer.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <CoreVideo/CVDisplayLink.h>
#import <ImageIO/ImageIO.h>
#import <simd/simd.h>
#import <math.h>

// ── Shaders ───────────────────────────────────────────────────────────────────
// Vertex layout (32 bytes, same as LeahRenderer):
//   float3 position  @ offset  0
//   float3 normal    @ offset 12
//   float2 uv        @ offset 24

static NSString * const kLogoShaders =
    @"#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct Vin  { float3 pos [[attribute(0)]]; float3 nrm [[attribute(1)]]; float2 uv [[attribute(2)]]; };\n"
    "struct Vout { float4 pos [[position]]; float clip_dist [[clip_distance]]; float3 nrm; float2 uv; };\n"
    "struct Uni  { float4x4 model; float4x4 vp; };\n"
    "vertex Vout logo_vert(Vin in [[stage_in]], constant Uni &u [[buffer(1)]]) {\n"
    "    float4 wPos = u.model * float4(in.pos, 1.0);\n"
    "    float4 cp = u.vp * wPos;\n"
    "    Vout o;\n"
    "    o.pos = cp;\n"
    "    o.clip_dist = cp.z;\n"  // negative → closer than near plane → clipped
    "    float3x3 rot = float3x3(u.model[0].xyz, u.model[1].xyz, u.model[2].xyz);\n"
    "    o.nrm = rot * in.nrm;\n"
    "    o.uv  = in.uv;\n"
    "    return o;\n"
    "}\n"
    "fragment float4 logo_frag(Vout in [[stage_in]],\n"
    "                          texture2d<float> tex [[texture(0)]],\n"
    "                          constant Uni &u [[buffer(1)]]) {\n"
    "    constexpr sampler s(filter::linear, address::clamp_to_edge);\n"
    "    float4 c = tex.sample(s, in.uv);\n"
    "    float3 L = normalize(float3(0.6, 1.0, 0.8));\n"
    "    float  d = max(0.0f, dot(normalize(in.nrm), L));\n"
    "    return float4(c.rgb * (0.35f + 0.65f * d), c.a);\n"
    "}\n";

// ── Uniform struct ─────────────────────────────────────────────────────────────
typedef struct { simd_float4x4 model; simd_float4x4 vp; } LogoUni;

// ── Animation keyframe (32 bytes, matches logo.bin layout) ────────────────────
typedef struct {
    int32_t  time_cs;
    float    qw, qx, qy, qz;
    float    px, py, pz;
} __attribute__((packed)) LogoKF;

// ── Per-shape CPU data ─────────────────────────────────────────────────────────
typedef struct {
    uint8_t       shape_id[4];
    uint8_t       tex_id[4];
    uint32_t      index_count;
    int32_t       tex_idx;         // index into _textures, -1 → white fallback
    simd_float4x4 init_mat;        // from logo.bin (used if no animation)
    simd_float4x4 model_mat;       // updated per frame
} LogoShapeData;

// ── Per-animation CPU data ─────────────────────────────────────────────────────
typedef struct {
    uint8_t    target_id[4];
    int32_t    end_time_cs;
    uint32_t   event_count;
    LogoKF    *events;             // malloc'd
    int32_t    shape_idx;          // resolved shape, -1 if none
} LogoAnimData;

// ── Matrix helpers ──────────────────────────────────────────────────────────────

static simd_float4x4 logo_mat_perspective(float fovY, float aspect, float nr, float fr) {
    float fy = 1.f / tanf(fovY * .5f);
    float dz = nr - fr;
    return (simd_float4x4){ .columns = {
        { fy/aspect, 0, 0, 0 },
        { 0, fy, 0, 0 },
        { 0, 0, fr/dz, -1 },
        { 0, 0, fr*nr/dz, 0 },
    }};
}

static simd_float4x4 logo_mat_lookAt(simd_float3 eye, simd_float3 ctr, simd_float3 up) {
    simd_float3 f = simd_normalize(eye - ctr);
    simd_float3 r = simd_normalize(simd_cross(up, f));
    simd_float3 u = simd_cross(f, r);
    return (simd_float4x4){ .columns = {
        { r.x, u.x, f.x, 0 },
        { r.y, u.y, f.y, 0 },
        { r.z, u.z, f.z, 0 },
        { -simd_dot(r,eye), -simd_dot(u,eye), -simd_dot(f,eye), 1 },
    }};
}

// ── Quaternion helpers ─────────────────────────────────────────────────────────

// Normalize a raw quaternion; treat all-zero (identity stored as 0,0,0,0) as (1,0,0,0).
static void logo_quat_normalize(float w, float x, float y, float z,
                                 float *rw, float *rx, float *ry, float *rz)
{
    float sq = w*w + x*x + y*y + z*z;
    if (sq > 0.0f) {
        float inv = 1.0f / sqrtf(sq);
        *rw = w*inv; *rx = x*inv; *ry = y*inv; *rz = z*inv;
    } else {
        *rw = 1.0f; *rx = *ry = *rz = 0.0f;   // all-zero → identity
    }
}

// Linear lerp + sign-flip (matches WR Quaternion_LerpInterpolation).
// Pre-normalizes both quaternions so that all-zero keyframes (stored identity)
// lerp smoothly toward/from non-zero rotations.
static void logo_quat_lerp(LogoKF *a, LogoKF *b, float t,
                            float *rw, float *rx, float *ry, float *rz)
{
    float aw, ax, ay, az, bw, bx, by, bz;
    logo_quat_normalize(a->qw, a->qx, a->qy, a->qz, &aw, &ax, &ay, &az);
    logo_quat_normalize(b->qw, b->qx, b->qy, b->qz, &bw, &bx, &by, &bz);

    float cosom = ax*bx + ay*by + az*bz + aw*bw;
    if (cosom < 0.0f) { bw=-bw; bx=-bx; by=-by; bz=-bz; }
    float s0 = 1.0f-t, s1 = t;
    *rw = s0*aw + s1*bw;
    *rx = s0*ax + s1*bx;
    *ry = s0*ay + s1*by;
    *rz = s0*az + s1*bz;
}

// Quaternion + translation → Metal column-major 4x4
// WR convention: v' = v × M  ⟺  Metal: v' = M × v  (M = transpose of WR matrix)
// Quaternion_QuaternionToMatrix sets WR rotation rows {x,y,z};
// Metal columns = WR rows (transpose).
static simd_float4x4 logo_quat_to_matrix(float qw, float qx, float qy, float qz,
                                          float tx, float ty, float tz)
{
    // Normalize
    float sq = qw*qw + qx*qx + qy*qy + qz*qz;
    float inv = sq > 0.0f ? 1.0f/sqrtf(sq) : 1.0f;
    qw*=inv; qx*=inv; qy*=inv; qz*=inv;

    float x2=qx+qx, y2=qy+qy, z2=qz+qz;
    float xx=qx*x2, xy=qx*y2, xz=qx*z2;
    float yy=qy*y2, yz=qy*z2, zz=qz*z2;
    float wx=qw*x2, wy=qw*y2, wz=qw*z2;

    // WR row x: {1-(yy+zz), xy-wz, xz+wy}  → Metal col 0
    // WR row y: {xy+wz,  1-(xx+zz), yz-wx}  → Metal col 1
    // WR row z: {xz-wy,  yz+wx,  1-(xx+yy)} → Metal col 2
    // translation                              → Metal col 3
    return (simd_float4x4){ .columns = {
        { 1.f-(yy+zz), xy-wz,       xz+wy,       0.f },
        { xy+wz,        1.f-(xx+zz), yz-wx,       0.f },
        { xz-wy,        yz+wx,       1.f-(xx+yy), 0.f },
        { tx, ty, tz, 1.f },
    }};
}

// Evaluate one animation at localTime → model matrix
static simd_float4x4 logo_eval_anim(LogoAnimData *anim, int32_t localTime)
{
    uint32_t ec = anim->event_count;
    if (ec == 0) return matrix_identity_float4x4;

    LogoKF *evs = anim->events;

    // Past end → last keyframe
    if (localTime >= anim->end_time_cs || localTime >= evs[ec-1].time_cs) {
        LogoKF *e = &evs[ec-1];
        return logo_quat_to_matrix(e->qw, e->qx, e->qy, e->qz, e->px, e->py, e->pz);
    }
    // Before first keyframe → first keyframe
    if (ec < 2 || localTime <= evs[0].time_cs) {
        LogoKF *e = &evs[0];
        return logo_quat_to_matrix(e->qw, e->qx, e->qy, e->qz, e->px, e->py, e->pz);
    }

    // Find bracket, scanning backwards from second-to-last (matches WR logic)
    for (int j = (int)ec - 2; j >= 0; --j) {
        if (localTime >= evs[j].time_cs) {
            LogoKF *prev = &evs[j];
            LogoKF *next = &evs[j+1];
            float dTime = (float)(localTime - prev->time_cs) /
                          (float)(next->time_cs - prev->time_cs);

            float rw, rx, ry, rz;
            logo_quat_lerp(prev, next, dTime, &rw, &rx, &ry, &rz);

            float px = prev->px + (next->px - prev->px) * dTime;
            float py = prev->py + (next->py - prev->py) * dTime;
            float pz = prev->pz + (next->pz - prev->pz) * dTime;

            return logo_quat_to_matrix(rw, rx, ry, rz, px, py, pz);
        }
    }

    LogoKF *e = &evs[0];
    return logo_quat_to_matrix(e->qw, e->qx, e->qy, e->qz, e->px, e->py, e->pz);
}

// ── LogoIntroView ──────────────────────────────────────────────────────────────

@interface LogoIntroView () {
    CVDisplayLinkRef _displayLink;
}
@property id<MTLDevice>               device;
@property id<MTLCommandQueue>         commandQueue;
@property id<MTLRenderPipelineState>  pipeline;
@property id<MTLDepthStencilState>    depthState;
@property id<MTLTexture>              depthTex;
@property CAMetalLayer               *metalLayer;

// Per-shape Metal buffers (parallel arrays, size = _shapeCount)
@property NSUInteger                  shapeCount;
@property NSMutableArray<id<MTLBuffer>> *vertexBufs;
@property NSMutableArray<id<MTLBuffer>> *indexBufs;

// Per-shape CPU data
@property LogoShapeData              *shapes;    // malloc'd
@property NSUInteger                  animCount;
@property LogoAnimData               *anims;     // malloc'd

// Textures keyed by 4-byte tex_id (stored as NSData → MTLTexture)
@property NSMutableDictionary<NSData *, id<MTLTexture>> *texMap;
@property id<MTLTexture>              whiteTex;  // fallback

// Script info
@property float                       sceneCx, sceneCy, sceneCz, sceneRadius;
@property uint32_t                    scriptLengthCs;
@property NSDate                     *startTime;
@property BOOL                        animDone;
@end

@implementation LogoIntroView

- (instancetype)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (!self) return nil;

    _device = MTLCreateSystemDefaultDevice();
    self.wantsLayer = YES;
    _metalLayer = (CAMetalLayer *)self.layer;
    _metalLayer.device       = _device;
    _metalLayer.pixelFormat  = MTLPixelFormatBGRA8Unorm;
    _metalLayer.drawableSize = CGSizeMake(frame.size.width * 2, frame.size.height * 2);

    _commandQueue = [_device newCommandQueue];
    _vertexBufs   = [NSMutableArray new];
    _indexBufs    = [NSMutableArray new];
    _texMap       = [NSMutableDictionary new];

    if (![self loadAssets] || ![self buildPipeline]) return nil;

    _startTime = [NSDate date];
    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, logoCB, (__bridge void *)self);
    CVDisplayLinkStart(_displayLink);
    return self;
}

- (CALayer *)makeBackingLayer { return [CAMetalLayer layer]; }

// ── Asset loading ──────────────────────────────────────────────────────────────

- (id<MTLTexture>)loadJPEG:(NSString *)path {
    NSURL *url = [NSURL fileURLWithPath:path];
    CGImageSourceRef src = CGImageSourceCreateWithURL((__bridge CFURLRef)url, nil);
    if (!src) return nil;
    CGImageRef img = CGImageSourceCreateImageAtIndex(src, 0, nil);
    CFRelease(src);
    if (!img) return nil;

    size_t w = CGImageGetWidth(img), h = CGImageGetHeight(img);
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    unsigned char *px = calloc(w * h * 4, 1);
    CGContextRef cx = CGBitmapContextCreate(px, w, h, 8, w*4, cs,
                          kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGContextDrawImage(cx, CGRectMake(0,0,w,h), img);
    CGContextRelease(cx); CGColorSpaceRelease(cs); CGImageRelease(img);

    MTLTextureDescriptor *td = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
        width:w height:h mipmapped:NO];
    id<MTLTexture> tex = [_device newTextureWithDescriptor:td];
    [tex replaceRegion:MTLRegionMake2D(0,0,w,h)
           mipmapLevel:0 withBytes:px bytesPerRow:w*4];
    free(px);
    return tex;
}

- (id<MTLTexture>)makeWhiteTexture {
    MTLTextureDescriptor *td = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
        width:1 height:1 mipmapped:NO];
    id<MTLTexture> tex = [_device newTextureWithDescriptor:td];
    uint8_t white[4] = { 200, 200, 200, 255 };
    [tex replaceRegion:MTLRegionMake2D(0,0,1,1)
           mipmapLevel:0 withBytes:white bytesPerRow:4];
    return tex;
}

- (BOOL)loadAssets {
    NSString *cwd  = NSFileManager.defaultManager.currentDirectoryPath;
    NSString *adir = [cwd stringByAppendingPathComponent:@"LogoAssets"];

    // ── Read logo.bin ──
    NSData *bin = [NSData dataWithContentsOfFile:
                   [adir stringByAppendingPathComponent:@"logo.bin"]];
    if (!bin) { NSLog(@"LogoIntroRenderer: logo.bin not found in %@", adir); return NO; }

    const uint8_t *b = bin.bytes;
    size_t off = 0;

    // Header (32 bytes)
    uint32_t scriptFlags, scriptLengthCs, shapeCount, animCount;
    float cx, cy, cz, sr;
    memcpy(&scriptFlags,    b+off, 4); off += 4;
    memcpy(&scriptLengthCs, b+off, 4); off += 4;
    memcpy(&shapeCount,     b+off, 4); off += 4;
    memcpy(&animCount,      b+off, 4); off += 4;
    memcpy(&cx,  b+off, 4); off += 4;
    memcpy(&cy,  b+off, 4); off += 4;
    memcpy(&cz,  b+off, 4); off += 4;
    memcpy(&sr,  b+off, 4); off += 4;

    _sceneCx = cx; _sceneCy = cy; _sceneCz = cz; _sceneRadius = sr;
    _scriptLengthCs = scriptLengthCs;
    _shapeCount = shapeCount;

    NSLog(@"LogoIntro: %u shapes, %u anims, script=%.1fs, scene=(%.1f,%.1f,%.1f) r=%.1f",
          shapeCount, animCount, scriptLengthCs/100.0, cx, cy, cz, sr);

    // ── Load fallback white texture ──
    _whiteTex = [self makeWhiteTexture];

    // ── Shapes ──
    _shapes = calloc(shapeCount, sizeof(LogoShapeData));

    for (uint32_t i = 0; i < shapeCount; ++i) {
        LogoShapeData *sh = &_shapes[i];

        memcpy(sh->shape_id, b+off, 4); off += 4;
        memcpy(sh->tex_id,   b+off, 4); off += 4;

        uint32_t vc, ic;
        memcpy(&vc, b+off, 4); off += 4;
        memcpy(&ic, b+off, 4); off += 4;

        memcpy(&sh->init_mat, b+off, 64); off += 64;
        sh->model_mat = sh->init_mat;   // default = initial pose

        size_t vbytes = vc * 32;
        size_t ibytes = ic * 4;
        sh->index_count = ic;

        id<MTLBuffer> vb = [_device newBufferWithBytes:b+off length:vbytes options:0];
        id<MTLBuffer> ib = [_device newBufferWithBytes:b+off+vbytes length:ibytes options:0];
        [_vertexBufs addObject:vb];
        [_indexBufs  addObject:ib];
        off += vbytes + ibytes;

        // Mark texture index as not-yet-resolved (-1 = white)
        sh->tex_idx = -1;
    }

    // ── Animations ──
    _animCount = animCount;
    _anims = calloc(animCount, sizeof(LogoAnimData));

    for (uint32_t i = 0; i < animCount; ++i) {
        LogoAnimData *an = &_anims[i];

        memcpy(an->target_id, b+off, 4); off += 4;
        uint32_t af; memcpy(&af, b+off, 4); off += 4;  // flags (unused)
        memcpy(&an->end_time_cs, b+off, 4); off += 4;
        memcpy(&an->event_count, b+off, 4); off += 4;

        an->events = malloc(an->event_count * sizeof(LogoKF));
        memcpy(an->events, b+off, an->event_count * sizeof(LogoKF));
        off += an->event_count * 32;   // exactly sizeof(LogoKF)

        // Resolve target shape index
        an->shape_idx = -1;
        for (NSUInteger j = 0; j < shapeCount; ++j) {
            if (memcmp(an->target_id, _shapes[j].shape_id, 4) == 0) {
                an->shape_idx = (int32_t)j;
                break;
            }
        }
        char sid[5] = {an->target_id[0], an->target_id[1],
                       an->target_id[2], an->target_id[3], 0};
        NSLog(@"  Anim → '%s' (shape idx %d)", sid, an->shape_idx);
    }

    // ── Load textures & resolve shape tex_idx ──
    for (NSUInteger i = 0; i < shapeCount; ++i) {
        LogoShapeData *sh = &_shapes[i];
        NSData *key = [NSData dataWithBytes:sh->tex_id length:4];

        // All-0xFF = kNoTexture → white
        if (sh->tex_id[0] == 0xFF) {
            _shapes[i].tex_idx = -1;
            continue;
        }

        if (!_texMap[key]) {
            char tname[5] = { sh->tex_id[0], sh->tex_id[1],
                              sh->tex_id[2], sh->tex_id[3], 0 };
            // Strip trailing spaces
            for (int k = 3; k >= 0 && tname[k] == ' '; k--) tname[k] = '\0';
            NSString *fname = [NSString stringWithFormat:@"logo_%s.jpg", tname];
            NSString *fpath = [adir stringByAppendingPathComponent:fname];
            id<MTLTexture> tex = [self loadJPEG:fpath];
            if (tex) {
                _texMap[key] = tex;
                NSLog(@"  Texture '%s' loaded", tname);
            } else {
                NSLog(@"  Texture '%s' not found, using white", tname);
            }
        }

        if (_texMap[key]) {
            // Store NSUInteger index so we can look up during render
            // We'll resolve by key at draw time instead of integer index
            sh->tex_idx = 0;  // sentinel: has entry in texMap
        }
    }

    return YES;
}

// ── Pipeline ──────────────────────────────────────────────────────────────────

- (BOOL)buildPipeline {
    NSError *err = nil;
    id<MTLLibrary> lib = [_device newLibraryWithSource:kLogoShaders options:nil error:&err];
    if (!lib) { NSLog(@"LogoIntro shader error: %@", err); return NO; }

    MTLVertexDescriptor *vd = [MTLVertexDescriptor new];
    vd.attributes[0].format = MTLVertexFormatFloat3; vd.attributes[0].offset =  0; vd.attributes[0].bufferIndex = 0;
    vd.attributes[1].format = MTLVertexFormatFloat3; vd.attributes[1].offset = 12; vd.attributes[1].bufferIndex = 0;
    vd.attributes[2].format = MTLVertexFormatFloat2; vd.attributes[2].offset = 24; vd.attributes[2].bufferIndex = 0;
    vd.layouts[0].stride = 32;

    MTLRenderPipelineDescriptor *pd = [MTLRenderPipelineDescriptor new];
    pd.vertexFunction                  = [lib newFunctionWithName:@"logo_vert"];
    pd.fragmentFunction                = [lib newFunctionWithName:@"logo_frag"];
    pd.vertexDescriptor                = vd;
    pd.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pd.depthAttachmentPixelFormat      = MTLPixelFormatDepth32Float;
    _pipeline = [_device newRenderPipelineStateWithDescriptor:pd error:&err];
    if (!_pipeline) { NSLog(@"LogoIntro pipeline error: %@", err); return NO; }

    MTLDepthStencilDescriptor *dd = [MTLDepthStencilDescriptor new];
    dd.depthCompareFunction = MTLCompareFunctionLess;
    dd.depthWriteEnabled    = YES;
    _depthState = [_device newDepthStencilStateWithDescriptor:dd];
    return YES;
}

// ── Render loop ───────────────────────────────────────────────────────────────

static CVReturn logoCB(CVDisplayLinkRef dl, const CVTimeStamp *now,
                       const CVTimeStamp *out, CVOptionFlags fi,
                       CVOptionFlags *fo, void *ctx) {
    [(__bridge LogoIntroView *)ctx render];
    return kCVReturnSuccess;
}

- (void)render {
    // ── Compute local animation time (centiseconds) ──
    NSTimeInterval elapsed = -[_startTime timeIntervalSinceNow];
    int32_t localTime = (int32_t)(elapsed * 100.0);  // to centiseconds

    // Loop the animation
    if (_scriptLengthCs > 0 && localTime > (int32_t)_scriptLengthCs) {
        localTime = localTime % (int32_t)_scriptLengthCs;
    }

    // ── Evaluate animations → update shape model matrices ──
    for (NSUInteger i = 0; i < _animCount; ++i) {
        LogoAnimData *an = &_anims[i];
        if (an->shape_idx < 0) continue;   // no matching shape (e.g. camr)
        _shapes[an->shape_idx].model_mat = logo_eval_anim(an, localTime);
    }

    // ── Find camr animation for camera eye (use last keyframe if past end) ──
    simd_float3 camEye  = simd_make_float3(_sceneCx, _sceneCy + _sceneRadius*0.4f,
                                            _sceneCz - _sceneRadius*3.0f);
    simd_float3 camLook = simd_make_float3(_sceneCx, _sceneCy, _sceneCz);
    for (NSUInteger i = 0; i < _animCount; ++i) {
        if (memcmp(_anims[i].target_id, "camr", 4) == 0) {
            simd_float4x4 cm = logo_eval_anim(&_anims[i], localTime);
            // Use the translation column (col 3) as camera eye
            camEye = simd_make_float3(cm.columns[3].x, cm.columns[3].y, cm.columns[3].z);
            // Keep the fixed look-at point
            break;
        }
    }

    // ── Get drawable ──
    id<CAMetalDrawable> drawable = [_metalLayer nextDrawable];
    if (!drawable) return;
    CGSize sz = _metalLayer.drawableSize;

    // ── Recreate depth texture if needed ──
    if (!_depthTex || _depthTex.width != (NSUInteger)sz.width
                   || _depthTex.height != (NSUInteger)sz.height) {
        MTLTextureDescriptor *dd = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
            width:(NSUInteger)sz.width height:(NSUInteger)sz.height mipmapped:NO];
        dd.usage       = MTLTextureUsageRenderTarget;
        dd.storageMode = MTLStorageModePrivate;
        _depthTex = [_device newTextureWithDescriptor:dd];
    }

    // ── View-projection matrix ──
    // Original engine: near=0.06, far=60.0, zoom=kShapeCameraZoom=0.5
    // WR zoom = tan(half_horizontal_FOV); fovY = 2*atan(zoom/aspect)
    float aspect = (float)(sz.width / sz.height);
    float wrZoom = 0.5f;
    float fovY   = 2.f * atanf(wrZoom / aspect);
    simd_float4x4 proj = logo_mat_perspective(fovY, aspect, 0.06f, 60.f);
    simd_float4x4 view = logo_mat_lookAt(camEye, camLook, simd_make_float3(0,1,0));
    simd_float4x4 vp   = simd_mul(proj, view);

    // ── Render pass ──
    MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor new];
    pass.colorAttachments[0].texture     = drawable.texture;
    pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
    pass.colorAttachments[0].clearColor  = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    pass.depthAttachment.texture         = _depthTex;
    pass.depthAttachment.loadAction      = MTLLoadActionClear;
    pass.depthAttachment.clearDepth      = 1.0;
    pass.depthAttachment.storeAction     = MTLStoreActionDontCare;

    id<MTLCommandBuffer>        cmd = [_commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> enc = [cmd renderCommandEncoderWithDescriptor:pass];

    [enc setRenderPipelineState:_pipeline];
    [enc setDepthStencilState:_depthState];
    [enc setCullMode:MTLCullModeBack];
    [enc setFrontFacingWinding:MTLWindingClockwise];

    // ── Draw each shape ──
    for (NSUInteger i = 0; i < _shapeCount; ++i) {
        // Skip the camera model (it's at the camera position, looks wrong)
        if (memcmp(_shapes[i].shape_id, "camr", 4) == 0) continue;

        LogoUni uni = { .model = _shapes[i].model_mat, .vp = vp };

        // Select texture
        id<MTLTexture> tex = _whiteTex;
        if (_shapes[i].tex_id[0] != 0xFF) {
            NSData *key = [NSData dataWithBytes:_shapes[i].tex_id length:4];
            id<MTLTexture> found = _texMap[key];
            if (found) tex = found;
        }

        [enc setVertexBuffer:_vertexBufs[i] offset:0 atIndex:0];
        [enc setVertexBytes:&uni  length:sizeof(uni) atIndex:1];
        [enc setFragmentBytes:&uni length:sizeof(uni) atIndex:1];
        [enc setFragmentTexture:tex atIndex:0];
        [enc drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                        indexCount:_shapes[i].index_count
                         indexType:MTLIndexTypeUInt32
                       indexBuffer:_indexBufs[i]
                 indexBufferOffset:0];
    }

    [enc endEncoding];
    [cmd presentDrawable:drawable];
    [cmd commit];
}

- (void)setFrameSize:(NSSize)sz {
    [super setFrameSize:sz];
    _metalLayer.drawableSize = CGSizeMake(sz.width * 2, sz.height * 2);
}

- (void)dealloc {
    CVDisplayLinkStop(_displayLink);
    CVDisplayLinkRelease(_displayLink);
    for (NSUInteger i = 0; i < _animCount; ++i)
        free(_anims[i].events);
    free(_shapes);
    free(_anims);
}

@end
