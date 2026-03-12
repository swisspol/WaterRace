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

// ── Uniform structs ────────────────────────────────────────────────────────────
typedef struct { simd_float4x4 model; simd_float4x4 vp; } LogoUni;
typedef struct { simd_float4x4 mvp;   simd_float4x4 rotY; } LeahUni;

// ── Per-shape data for multi-shape rotating models (G-Boat, etc.) ─────────────
typedef struct {
    uint8_t  tex_id[4];
    uint32_t index_count;
} ModelShapeData;

// ── Leah shaders ───────────────────────────────────────────────────────────────
static NSString * const kLeahShaders =
    @"#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct LVin  { float3 pos [[attribute(0)]]; float3 nrm [[attribute(1)]]; float2 uv [[attribute(2)]]; };\n"
    "struct LVout { float4 pos [[position]]; float3 nrm; float2 uv; };\n"
    "struct LUni  { float4x4 mvp; float4x4 rotY; };\n"
    "vertex LVout leah_vert(LVin in [[stage_in]], constant LUni &u [[buffer(1)]]) {\n"
    "    LVout o;\n"
    "    o.pos = u.mvp  * float4(in.pos, 1.0);\n"
    "    o.nrm = (u.rotY * float4(in.nrm, 0.0)).xyz;\n"
    "    o.uv  = in.uv;\n"
    "    return o;\n"
    "}\n"
    "fragment float4 leah_frag(LVout in [[stage_in]],\n"
    "                          texture2d<float> tex [[texture(0)]],\n"
    "                          constant LUni &u [[buffer(1)]]) {\n"
    "    constexpr sampler s(filter::linear, address::clamp_to_edge);\n"
    "    float4 c = tex.sample(s, in.uv);\n"
    "    float3 L = normalize(float3(0.6, 1.0, 0.8));\n"
    "    float  d = max(0.0f, dot(normalize(in.nrm), L));\n"
    "    return float4(c.rgb * (0.35f + 0.65f * d), c.a);\n"
    "}\n";

// ── Animation keyframe ────────────────────────────────────────────────────────
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
    simd_float4x4 init_mat;    // initial pose (used before animation starts)
    simd_float4x4 model_mat;   // updated per frame by animation
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

// ── WaterRace MetaFile parser helpers ──────────────────────────────────────────
// All MetaFile data is big-endian (PowerPC / classic Mac OS).

static inline uint32_t mf_u32(const uint8_t *p) {
    return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|(uint32_t)p[2]<<8|(uint32_t)p[3];
}
static inline int32_t  mf_s32(const uint8_t *p) { return (int32_t)mf_u32(p); }
static inline float    mf_f32(const uint8_t *p) { uint32_t u=mf_u32(p); float f; memcpy(&f,&u,4); return f; }
static inline BOOL     mf_4cc(const uint8_t *p, const char *t) {
    return p[0]==t[0]&&p[1]==t[1]&&p[2]==t[2]&&p[3]==t[3];
}

// WR row-vector transform: v' = v × M  (M rows = local axes, row 3 = translation)
static void mf_xform_pt(const float v[3], const float m[4][3], float out[3]) {
    out[0] = v[0]*m[0][0]+v[1]*m[1][0]+v[2]*m[2][0]+m[3][0];
    out[1] = v[0]*m[0][1]+v[1]*m[1][1]+v[2]*m[2][1]+m[3][1];
    out[2] = v[0]*m[0][2]+v[1]*m[1][2]+v[2]*m[2][2]+m[3][2];
}
static void mf_xform_nrm(const float n[3], const float m[4][3], float out[3]) {
    out[0] = n[0]*m[0][0]+n[1]*m[1][0]+n[2]*m[2][0];
    out[1] = n[0]*m[0][1]+n[1]*m[1][1]+n[2]*m[2][1];
    out[2] = n[0]*m[0][2]+n[1]*m[1][2]+n[2]*m[2][2];
    float l = sqrtf(out[0]*out[0]+out[1]*out[1]+out[2]*out[2]);
    if (l > 1e-6f) { out[0]/=l; out[1]/=l; out[2]/=l; }
    else            { out[0]=0;  out[1]=1;  out[2]=0; }
}

// ── Additional matrix helpers ──────────────────────────────────────────────────

static simd_float4x4 mat_rotY(float a) {
    float c = cosf(a), s = sinf(a);
    return (simd_float4x4){ .columns = {
        {  c, 0, -s, 0 }, { 0, 1, 0, 0 }, { s, 0, c, 0 }, { 0, 0, 0, 1 },
    }};
}

static simd_float4x4 mat_translate(float tx, float ty, float tz) {
    simd_float4x4 m = matrix_identity_float4x4;
    m.columns[3] = (simd_float4){ tx, ty, tz, 1 };
    return m;
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
@property id<MTLDepthStencilState>    depthState;
@property id<MTLTexture>              depthTex;
@property CAMetalLayer               *metalLayer;

// ── Logo scene ─────────────────────────────────────────────────────────────────
@property id<MTLRenderPipelineState>  logoPipeline;
@property NSUInteger                  shapeCount;
@property NSMutableArray<id<MTLBuffer>> *vertexBufs;
@property NSMutableArray<id<MTLBuffer>> *indexBufs;
@property LogoShapeData              *shapes;    // malloc'd
@property NSUInteger                  animCount;
@property LogoAnimData               *anims;     // malloc'd
@property NSMutableDictionary<NSData *, id<MTLTexture>> *texMap;
@property id<MTLTexture>              whiteTex;
@property float                       sceneCx, sceneCy, sceneCz, sceneRadius;
@property uint32_t                    scriptLengthCs;
@property NSDate                     *startTime;

// ── Leah scene ─────────────────────────────────────────────────────────────────
@property id<MTLRenderPipelineState>  leahPipeline;
@property id<MTLBuffer>               leahVertexBuf;
@property id<MTLBuffer>               leahIndexBuf;
@property id<MTLTexture>              leahTexture;
@property uint32_t                    leahIndexCount;
@property simd_float3                 leahCenter;
@property float                       leahDiameter;
@property float                       leahAngle;

// ── G-Boat scene ───────────────────────────────────────────────────────────────
@property NSMutableArray<id<MTLBuffer>> *gboatVertexBufs;
@property NSMutableArray<id<MTLBuffer>> *gboatIndexBufs;
@property NSMutableDictionary<NSData *, id<MTLTexture>> *gboatTexMap;
@property NSUInteger                  gboatShapeCount;
@property ModelShapeData             *gboatShapes;   // malloc'd
@property simd_float3                 gboatCenter;
@property float                       gboatDiameter;
@property float                       gboatAngle;

// ── Scene state (0=logo, 1=leah, 2=gboat) ─────────────────────────────────────
@property NSInteger                   scene;
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

    _commandQueue    = [_device newCommandQueue];
    _vertexBufs      = [NSMutableArray new];
    _indexBufs       = [NSMutableArray new];
    _texMap          = [NSMutableDictionary new];
    _gboatVertexBufs = [NSMutableArray new];
    _gboatIndexBufs  = [NSMutableArray new];
    _gboatTexMap     = [NSMutableDictionary new];

    if (![self loadAssets] || ![self buildPipeline]) return nil;
    [self loadLeahAssets];    // best-effort
    [self loadGBoatAssets];   // best-effort
    [self buildLeahPipeline]; // shared pipeline for Leah + G-Boat

    _startTime = [NSDate date];
    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, logoCB, (__bridge void *)self);
    CVDisplayLinkStart(_displayLink);
    return self;
}

- (BOOL)acceptsFirstResponder { return YES; }

- (void)keyDown:(NSEvent *)event {
    if (_scene < 2) _scene++;
}

- (CALayer *)makeBackingLayer { return [CAMetalLayer layer]; }

// ── Asset loading ──────────────────────────────────────────────────────────────

- (id<MTLTexture>)loadJPEGFromData:(NSData *)jpegData {
    CGImageSourceRef src = CGImageSourceCreateWithData((__bridge CFDataRef)jpegData, nil);
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
    NSString *adir = [cwd stringByAppendingPathComponent:@"Assets"];
    NSString *path = [adir stringByAppendingPathComponent:@"Logo Intro"];
    NSData *data   = [NSData dataWithContentsOfFile:path];
    if (!data) { NSLog(@"LogoIntroRenderer: 'Logo Intro' MetaFile not found at %@", path); return NO; }

    const uint8_t *b   = data.bytes;
    NSUInteger     len = data.length;
    if (len < 16) return NO;

    _whiteTex = [self makeWhiteTexture];

    // ── Tag table ──────────────────────────────────────────────────────────
    int32_t  ttPos    = mf_s32(b + 8);
    uint32_t tagCount = mf_u32(b + ttPos);
    NSUInteger eBase  = (NSUInteger)ttPos + 4;

    // ── Pass 1: count renderable shapes and kType_Animation anims ──────────
    uint32_t shapeCount = 0, animCount = 0;
    for (uint32_t i = 0; i < tagCount; ++i) {
        const uint8_t *e = b + eBase + i * 24;
        if (eBase + i * 24 + 24 > len) break;
        if (mf_4cc(e + 4, "shap")) {
            int32_t spos = mf_s32(e + 20);
            if (spos >= 0 && (NSUInteger)spos + 228 <= len) {
                uint16_t shading = (uint16_t)(b[spos+226]<<8 | b[spos+227]);
                if (shading < 16) shapeCount++;
            }
        } else if (mf_4cc(e + 4, "<-->") && mf_4cc(e + 8, "anim")) {
            int32_t apos = mf_s32(e + 20);
            if (apos >= 0 && (NSUInteger)apos + 12 <= len && mf_4cc(b + apos + 8, "anim"))
                animCount++;
        }
    }

    _shapeCount = shapeCount;
    _animCount  = animCount;
    _shapes     = calloc(shapeCount, sizeof(LogoShapeData));
    _anims      = calloc(animCount,  sizeof(LogoAnimData));

    // ── Pass 2: parse shapes ───────────────────────────────────────────────
    uint32_t shapIdx = 0;
    for (uint32_t i = 0; i < tagCount; ++i) {
        const uint8_t *e = b + eBase + i * 24;
        if (eBase + i * 24 + 24 > len) break;
        if (!mf_4cc(e + 4, "shap")) continue;

        int32_t spos = mf_s32(e + 20);
        if (spos < 0 || (NSUInteger)spos + 300 > len) continue;
        const uint8_t *sh = b + spos;

        uint16_t shading = (uint16_t)(sh[226]<<8 | sh[227]);
        if (shading >= 16) continue;   // FX / sprite shape

        int32_t pc = mf_s32(sh + 0);
        int32_t tc = mf_s32(sh + 8);
        if (pc <= 0 || tc <= 0) continue;

        // Find child pnts/trig/nrls (depth-bounded)
        uint8_t childDepth = e[0] + 1;
        int32_t pntsPos = -1, trigPos = -1, nrlsPos = -1, nrlsSize = 0;
        for (uint32_t j = i + 1; j < tagCount; ++j) {
            const uint8_t *ce = b + eBase + j * 24;
            if (eBase + j * 24 + 24 > len) break;
            if (ce[0] < childDepth) break;
            if (!mf_4cc(ce + 4, "<-->")) continue;
            int32_t cpos = mf_s32(ce + 20), csz = mf_s32(ce + 16);
            if (mf_4cc(ce + 8, "pnts") && pntsPos < 0) pntsPos = cpos;
            if (mf_4cc(ce + 8, "trig") && trigPos < 0) trigPos = cpos;
            if (mf_4cc(ce + 8, "nrls") && nrlsPos < 0) { nrlsPos = cpos; nrlsSize = csz; }
        }
        if (pntsPos < 0 || trigPos < 0) continue;
        if ((NSUInteger)pntsPos + pc*24 > len || (NSUInteger)trigPos + tc*12 > len) continue;

        const uint8_t *pnts = b + pntsPos;
        const uint8_t *trig = b + trigPos;
        const uint8_t *nrls = (nrlsPos >= 0 && nrlsSize >= pc*12 &&
                               (NSUInteger)nrlsPos + pc*12 <= len) ? b + nrlsPos : NULL;

        // Compute normals if not stored (in LOCAL space — no matrix applied)
        float *computed = NULL;
        if (!nrls) {
            computed = calloc(pc * 3, sizeof(float));
            for (int ti = 0; ti < tc; ti++) {
                uint32_t ai = mf_u32(trig + ti*12+0);
                uint32_t bi = mf_u32(trig + ti*12+4);
                uint32_t ci = mf_u32(trig + ti*12+8);
                float ap[3] = { mf_f32(pnts+ai*24), mf_f32(pnts+ai*24+4), mf_f32(pnts+ai*24+8) };
                float bp[3] = { mf_f32(pnts+bi*24), mf_f32(pnts+bi*24+4), mf_f32(pnts+bi*24+8) };
                float cp[3] = { mf_f32(pnts+ci*24), mf_f32(pnts+ci*24+4), mf_f32(pnts+ci*24+8) };
                float ux=bp[0]-ap[0], uy=bp[1]-ap[1], uz=bp[2]-ap[2];
                float vx=cp[0]-ap[0], vy=cp[1]-ap[1], vz=cp[2]-ap[2];
                float nx=uy*vz-uz*vy, ny=uz*vx-ux*vz, nz=ux*vy-uy*vx;
                uint32_t face[3] = { ai, bi, ci };
                for (int fi = 0; fi < 3; fi++) {
                    uint32_t idx = face[fi];
                    if (idx < (uint32_t)pc) {
                        computed[idx*3]+=nx; computed[idx*3+1]+=ny; computed[idx*3+2]+=nz;
                    }
                }
            }
            for (int vi = 0; vi < pc; vi++) {
                float *n = computed + vi*3;
                float l = sqrtf(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
                if (l > 1e-6f) { n[0]/=l; n[1]/=l; n[2]/=l; }
                else            { n[0]=0;  n[1]=1;  n[2]=0;  }
            }
        }

        // Vertex buffer: float3 pos (local), float3 nrm (local), float2 uv
        float *vbuf = malloc(pc * 32);
        for (int vi = 0; vi < pc; vi++) {
            const uint8_t *vp = pnts + vi * 24;
            float *dst = vbuf + vi * 8;
            dst[0] = mf_f32(vp);  dst[1] = mf_f32(vp+4);  dst[2] = mf_f32(vp+8);
            if (nrls) {
                dst[3] = mf_f32(nrls+vi*12); dst[4] = mf_f32(nrls+vi*12+4); dst[5] = mf_f32(nrls+vi*12+8);
            } else {
                dst[3] = computed[vi*3]; dst[4] = computed[vi*3+1]; dst[5] = computed[vi*3+2];
            }
            dst[6] = mf_f32(vp+12);        // u
            dst[7] = 1.0f - mf_f32(vp+16); // v (WR V=0-at-bottom → Metal V=0-at-top)
        }
        free(computed);
        [_vertexBufs addObject:[_device newBufferWithBytes:vbuf length:pc*32 options:0]];
        free(vbuf);

        // Index buffer
        uint32_t *ibuf = malloc(tc * 12);
        for (int ti = 0; ti < tc; ti++) {
            ibuf[ti*3+0] = mf_u32(trig + ti*12+0);
            ibuf[ti*3+1] = mf_u32(trig + ti*12+4);
            ibuf[ti*3+2] = mf_u32(trig + ti*12+8);
        }
        [_indexBufs addObject:[_device newBufferWithBytes:ibuf length:tc*12 options:0]];
        free(ibuf);

        // Fill shape record
        LogoShapeData *sd = &_shapes[shapIdx];
        memcpy(sd->shape_id, sh + 296, 4);  // shape id at header +296
        memcpy(sd->tex_id,   sh + 200, 4);  // texture id at header +200
        sd->index_count = (uint32_t)tc * 3;
        // Init matrix: WR row-major [4][3] → Metal column-major 4x4
        float m[4][3];
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 3; c++)
                m[r][c] = mf_f32(sh + 36 + (r*3 + c)*4);
        sd->init_mat = (simd_float4x4){ .columns = {
            { m[0][0], m[0][1], m[0][2], 0.f },
            { m[1][0], m[1][1], m[1][2], 0.f },
            { m[2][0], m[2][1], m[2][2], 0.f },
            { m[3][0], m[3][1], m[3][2], 1.f },
        }};
        sd->model_mat = sd->init_mat;

        char sid[5] = { sd->shape_id[0], sd->shape_id[1], sd->shape_id[2], sd->shape_id[3], 0 };
        NSLog(@"  Shape '%s': %d verts, %d tris", sid, pc, tc);
        shapIdx++;
    }
    _shapeCount = shapIdx;

    // ── Pass 3: parse kType_Animation animations ───────────────────────────
    uint32_t animIdx = 0;
    for (uint32_t i = 0; i < tagCount && animIdx < animCount; ++i) {
        const uint8_t *e = b + eBase + i * 24;
        if (eBase + i * 24 + 24 > len) break;
        if (!mf_4cc(e + 4, "<-->") || !mf_4cc(e + 8, "anim")) continue;
        int32_t apos = mf_s32(e + 20);
        if (apos < 0 || (NSUInteger)apos + 24 > len) continue;
        if (!mf_4cc(b + apos + 8, "anim")) continue;   // not kType_Animation

        LogoAnimData *ad = &_anims[animIdx];
        memcpy(ad->target_id, b + apos, 4);
        ad->end_time_cs = mf_s32(b + apos + 12);
        ad->event_count = mf_u32(b + apos + 20);
        // Clamp event count to available data
        uint32_t maxEv = (uint32_t)((len - (NSUInteger)apos - 24) / 32);
        if (ad->event_count > maxEv) ad->event_count = maxEv;
        ad->events = malloc(ad->event_count * sizeof(LogoKF));
        for (uint32_t k = 0; k < ad->event_count; ++k) {
            const uint8_t *ep = b + apos + 24 + k * 32;
            LogoKF *kf = &ad->events[k];
            kf->time_cs = mf_s32(ep);
            kf->qw = mf_f32(ep+ 4);  // rotX = qw
            kf->qx = mf_f32(ep+ 8);  // rotY = qx
            kf->qy = mf_f32(ep+12);  // rotZ = qy
            kf->qz = mf_f32(ep+16);  // rotW = qz
            kf->px = mf_f32(ep+20);
            kf->py = mf_f32(ep+24);
            kf->pz = mf_f32(ep+28);
        }
        // Resolve target shape
        ad->shape_idx = -1;
        for (uint32_t j = 0; j < _shapeCount; ++j) {
            if (memcmp(ad->target_id, _shapes[j].shape_id, 4) == 0) {
                ad->shape_idx = (int32_t)j; break;
            }
        }
        char tid[5] = { ad->target_id[0], ad->target_id[1], ad->target_id[2], ad->target_id[3], 0 };
        NSLog(@"  Anim → '%s' (shape idx %d)", tid, ad->shape_idx);
        animIdx++;
    }
    _animCount = animIdx;

    // ── Pass 4: extract textures from cptx/txim PICT data ─────────────────
    for (uint32_t i = 0; i < tagCount; ++i) {
        const uint8_t *e = b + eBase + i * 24;
        if (eBase + i * 24 + 24 > len) break;
        if (!mf_4cc(e + 4, "cptx")) continue;
        uint8_t cptxDepth = e[0];
        for (uint32_t j = i + 1; j < tagCount; ++j) {
            const uint8_t *ne = b + eBase + j * 24;
            if (eBase + j * 24 + 24 > len) break;
            if (ne[0] <= cptxDepth && !mf_4cc(ne + 4, "<-->")) break;
            if (!mf_4cc(ne + 4, "<-->") || !mf_4cc(ne + 8, "txim")) continue;
            int32_t tpos = mf_s32(ne + 20), tsz = mf_s32(ne + 16);
            if (tpos < 0 || tsz < 2 || (NSUInteger)tpos + tsz > len) break;
            // Scan PICT for embedded JPEG (SOI=FFD8 … EOI=FFD9)
            const uint8_t *pict = b + tpos;
            NSInteger soi = -1, eoi = -1;
            for (int32_t k = 0; k < tsz - 1; k++) {
                if (pict[k]==0xFF && pict[k+1]==0xD8 && soi < 0) soi = k;
                if (pict[k]==0xFF && pict[k+1]==0xD9)             eoi = k + 2;
            }
            if (soi >= 0 && eoi > soi) {
                NSData *jpeg = [NSData dataWithBytes:pict+soi length:eoi-soi];
                id<MTLTexture> tex = [self loadJPEGFromData:jpeg];
                if (tex) {
                    NSData *key = [NSData dataWithBytes:e + 12 length:4];
                    _texMap[key] = tex;
                    char tid[5] = { e[12], e[13], e[14], e[15], 0 };
                    NSLog(@"  Texture '%s' loaded", tid);
                }
            }
            break;
        }
    }
    // ── Scene bounds from final keyframe positions of non-camera shapes ────
    float cx = 0, cy = 0, cz = 0;
    NSUInteger posCount = 0;
    for (uint32_t i = 0; i < _animCount; ++i) {
        if (memcmp(_anims[i].target_id, "camr", 4) == 0) continue;
        if (_anims[i].event_count == 0) continue;
        LogoKF *last = &_anims[i].events[_anims[i].event_count - 1];
        cx += last->px; cy += last->py; cz += last->pz;
        posCount++;
    }
    if (posCount > 0) { cx /= posCount; cy /= posCount; cz /= posCount; }
    float rmax = 1.0f;
    for (uint32_t i = 0; i < _animCount; ++i) {
        if (memcmp(_anims[i].target_id, "camr", 4) == 0) continue;
        if (_anims[i].event_count == 0) continue;
        LogoKF *last = &_anims[i].events[_anims[i].event_count - 1];
        float dx = last->px - cx, dy = last->py - cy, dz = last->pz - cz;
        float r = sqrtf(dx*dx + dy*dy + dz*dz);
        if (r > rmax) rmax = r;
    }
    _sceneCx = cx; _sceneCy = cy; _sceneCz = cz;
    _sceneRadius = rmax * 1.2f;

    // ── Script duration ────────────────────────────────────────────────────
    for (uint32_t i = 0; i < tagCount; ++i) {
        const uint8_t *e = b + eBase + i * 24;
        if (eBase + i * 24 + 24 > len) break;
        if (!mf_4cc(e + 4, "scrp")) continue;
        int32_t spos = mf_s32(e + 20);
        if (spos >= 0 && (NSUInteger)spos + 56 <= len) {
            int32_t lenCs = mf_s32(b + spos + 52);
            if (lenCs > 0) _scriptLengthCs = (uint32_t)lenCs;
        }
        break;
    }

    NSLog(@"LogoIntro: %lu shapes, %lu anims, script=%.1fs, scene=(%.1f,%.1f,%.1f) r=%.1f",
          (unsigned long)_shapeCount, (unsigned long)_animCount,
          _scriptLengthCs/100.0, cx, cy, cz, _sceneRadius);
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
    _logoPipeline = [_device newRenderPipelineStateWithDescriptor:pd error:&err];
    if (!_logoPipeline) { NSLog(@"LogoIntro pipeline error: %@", err); return NO; }

    MTLDepthStencilDescriptor *dd = [MTLDepthStencilDescriptor new];
    dd.depthCompareFunction = MTLCompareFunctionLess;
    dd.depthWriteEnabled    = YES;
    _depthState = [_device newDepthStencilStateWithDescriptor:dd];
    return YES;
}

// ── Leah asset loading (direct MetaFile parse) ────────────────────────────────
// MetaFile format: big-endian, tag table at offset 8 of header.
// Tag table: uint32 count, then count × 24-byte entries:
//   [0] u8 depth, [1] u8 flags, [2-3] i16 version,
//   [4-7] OSType tag, [8-11] OSType subtag, [12-15] OSType id,
//   [16-19] i32 dataSize, [20-23] i32 dataPos
// Shape header at dataPos: 300 bytes (see prepare_leah.py for field layout).
// Vertex (pnts):  6 big-endian floats = x,y,z,u,v,c  (24 bytes each)
// Normal (nrls):  3 big-endian floats = nx,ny,nz      (12 bytes each)
// Triangle (trig):3 big-endian uint32 = a,b,c         (12 bytes each)
// Texture (cptx/txim): PICT v2 data containing a CompressedQuickTime JPEG

- (void)loadLeahAssets {
    NSString *cwd  = NSFileManager.defaultManager.currentDirectoryPath;
    NSString *path = [[cwd stringByAppendingPathComponent:@"Assets"]
                           stringByAppendingPathComponent:@"Leah"];
    NSData *data = [NSData dataWithContentsOfFile:path];
    if (!data) { NSLog(@"Leah: MetaFile not found at %@", path); return; }

    const uint8_t *b   = data.bytes;
    NSUInteger     len = data.length;
    if (len < 16) return;

    // ── Tag table ──────────────────────────────────────────────────────────────
    int32_t  ttPos    = mf_s32(b + 8);
    uint32_t tagCount = mf_u32(b + ttPos);
    NSUInteger eBase  = ttPos + 4;   // base of 24-byte entry array

    // ── Find shap + its geometry subtags, and cptx texture ────────────────────
    int32_t shapIdx = -1;
    int32_t pntsIdx = -1, trigIdx = -1, nrlsIdx = -1;
    int32_t cptxIdx = -1, tximIdx = -1;

    for (uint32_t i = 0; i < tagCount; ++i) {
        const uint8_t *e = b + eBase + i * 24;
        if (eBase + i * 24 + 24 > len) break;
        const uint8_t *tag = e + 4, *sub = e + 8;

        if (shapIdx < 0 && mf_4cc(tag, "shap"))                                         shapIdx = i;
        if (shapIdx >= 0 && pntsIdx < 0 && mf_4cc(tag, "<-->") && mf_4cc(sub, "pnts")) pntsIdx = i;
        if (shapIdx >= 0 && trigIdx < 0 && mf_4cc(tag, "<-->") && mf_4cc(sub, "trig")) trigIdx = i;
        if (shapIdx >= 0 && nrlsIdx < 0 && mf_4cc(tag, "<-->") && mf_4cc(sub, "nrls")) nrlsIdx = i;
        if (cptxIdx < 0 && mf_4cc(tag, "cptx"))                                         cptxIdx = i;
        if (cptxIdx >= 0 && tximIdx < 0 && mf_4cc(tag, "<-->") && mf_4cc(sub, "txim")) tximIdx = i;
    }
    if (shapIdx < 0 || pntsIdx < 0 || trigIdx < 0) {
        NSLog(@"Leah: geometry entries not found"); return;
    }

    // ── Shape header ───────────────────────────────────────────────────────────
    int32_t shapPos = mf_s32(b + eBase + shapIdx * 24 + 20);
    if (shapPos < 0 || (NSUInteger)shapPos + 300 > len) return;
    const uint8_t *sh = b + shapPos;
    int32_t pc = mf_s32(sh + 0);   // pointCount
    int32_t tc = mf_s32(sh + 8);   // triCount

    // Shape local matrix: 4 rows × 3 floats at offset 36 (row 3 = translation)
    float m[4][3];
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 3; c++)
            m[r][c] = mf_f32(sh + 36 + (r*3 + c)*4);

    // ── Geometry data pointers ─────────────────────────────────────────────────
    int32_t pntsPos = mf_s32(b + eBase + pntsIdx * 24 + 20);
    int32_t trigPos = mf_s32(b + eBase + trigIdx * 24 + 20);
    if (pntsPos < 0 || (NSUInteger)pntsPos + pc*24 > len) return;
    if (trigPos < 0 || (NSUInteger)trigPos + tc*12 > len) return;
    const uint8_t *pnts = b + pntsPos;
    const uint8_t *trig = b + trigPos;

    const uint8_t *nrls = NULL;
    if (nrlsIdx >= 0) {
        int32_t nrlsPos  = mf_s32(b + eBase + nrlsIdx * 24 + 20);
        int32_t nrlsSize = mf_s32(b + eBase + nrlsIdx * 24 + 16);
        if (nrlsPos >= 0 && nrlsSize >= pc*12 && (NSUInteger)nrlsPos + pc*12 <= len)
            nrls = b + nrlsPos;
    }

    // ── Compute per-vertex normals if no stored normals ────────────────────────
    float *computed = NULL;
    if (!nrls) {
        computed = calloc(pc * 3, sizeof(float));
        for (int ti = 0; ti < tc; ti++) {
            uint32_t ai = mf_u32(trig + ti*12+0);
            uint32_t bi = mf_u32(trig + ti*12+4);
            uint32_t ci = mf_u32(trig + ti*12+8);
            // Read local positions
            float ap[3] = { mf_f32(pnts+ai*24), mf_f32(pnts+ai*24+4), mf_f32(pnts+ai*24+8) };
            float bp[3] = { mf_f32(pnts+bi*24), mf_f32(pnts+bi*24+4), mf_f32(pnts+bi*24+8) };
            float cp[3] = { mf_f32(pnts+ci*24), mf_f32(pnts+ci*24+4), mf_f32(pnts+ci*24+8) };
            // Transform to world space
            float aw[3], bw[3], cw[3];
            mf_xform_pt(ap, m, aw); mf_xform_pt(bp, m, bw); mf_xform_pt(cp, m, cw);
            float ux=bw[0]-aw[0], uy=bw[1]-aw[1], uz=bw[2]-aw[2];
            float vx=cw[0]-aw[0], vy=cw[1]-aw[1], vz=cw[2]-aw[2];
            float nx=uy*vz-uz*vy, ny=uz*vx-ux*vz, nz=ux*vy-uy*vx;
            uint32_t face[3] = { ai, bi, ci };
            for (int fi = 0; fi < 3; fi++) {
                uint32_t idx = face[fi];
                computed[idx*3]+=nx; computed[idx*3+1]+=ny; computed[idx*3+2]+=nz;
            }
        }
        for (int vi = 0; vi < pc; vi++) {
            float *n = computed + vi*3;
            float l = sqrtf(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
            if (l > 1e-6f) { n[0]/=l; n[1]/=l; n[2]/=l; }
            else            { n[0]=0;  n[1]=1;  n[2]=0;  }
        }
    }

    // ── Build vertex buffer: float3 pos, float3 nrm, float2 uv (32 bytes) ──────
    float *vbuf = malloc(pc * 32);
    float bxn=FLT_MAX,byn=FLT_MAX,bzn=FLT_MAX;
    float bxx=-FLT_MAX,byx=-FLT_MAX,bzx=-FLT_MAX;

    for (int vi = 0; vi < pc; vi++) {
        const uint8_t *vp = pnts + vi * 24;
        float lp[3] = { mf_f32(vp), mf_f32(vp+4), mf_f32(vp+8) };
        float u = mf_f32(vp+12), v = mf_f32(vp+16);

        float wp[3];
        mf_xform_pt(lp, m, wp);

        float wn[3];
        if (nrls) {
            float ln[3] = { mf_f32(nrls+vi*12), mf_f32(nrls+vi*12+4), mf_f32(nrls+vi*12+8) };
            mf_xform_nrm(ln, m, wn);
        } else {
            wn[0]=computed[vi*3]; wn[1]=computed[vi*3+1]; wn[2]=computed[vi*3+2];
        }

        float *dst = vbuf + vi * 8;
        dst[0]=wp[0]; dst[1]=wp[1]; dst[2]=wp[2];
        dst[3]=wn[0]; dst[4]=wn[1]; dst[5]=wn[2];
        dst[6]=u;     dst[7]=1.0f - v;   // flip V: WR V=0-at-bottom → Metal V=0-at-top

        bxn=fminf(bxn,wp[0]); bxx=fmaxf(bxx,wp[0]);
        byn=fminf(byn,wp[1]); byx=fmaxf(byx,wp[1]);
        bzn=fminf(bzn,wp[2]); bzx=fmaxf(bzx,wp[2]);
    }
    free(computed);

    _leahCenter   = simd_make_float3((bxn+bxx)/2, (byn+byx)/2, (bzn+bzx)/2);
    _leahDiameter = fmaxf(fmaxf(bxx-bxn, byx-byn), bzx-bzn);
    _leahVertexBuf = [_device newBufferWithBytes:vbuf length:pc*32 options:0];
    free(vbuf);

    // ── Index buffer ───────────────────────────────────────────────────────────
    _leahIndexCount = tc * 3;
    uint32_t *ibuf = malloc(tc * 12);
    for (int ti = 0; ti < tc; ti++) {
        ibuf[ti*3+0] = mf_u32(trig + ti*12+0);
        ibuf[ti*3+1] = mf_u32(trig + ti*12+4);
        ibuf[ti*3+2] = mf_u32(trig + ti*12+8);
    }
    _leahIndexBuf = [_device newBufferWithBytes:ibuf length:tc*12 options:0];
    free(ibuf);

    // ── Texture: scan PICT data for embedded JPEG (SOI=FFD8 … EOI=FFD9) ───────
    if (tximIdx >= 0) {
        int32_t tximPos  = mf_s32(b + eBase + tximIdx * 24 + 20);
        int32_t tximSize = mf_s32(b + eBase + tximIdx * 24 + 16);
        if (tximPos >= 0 && tximSize > 2 && (NSUInteger)tximPos + tximSize <= len) {
            const uint8_t *pict = b + tximPos;
            NSInteger soi = -1, eoi = -1;
            for (int32_t j = 0; j < tximSize - 1; j++) {
                if (pict[j] == 0xFF && pict[j+1] == 0xD8 && soi < 0) soi = j;
                if (pict[j] == 0xFF && pict[j+1] == 0xD9)            eoi = j + 2;
            }
            if (soi >= 0 && eoi > soi) {
                NSData *jpeg = [NSData dataWithBytes:pict+soi length:eoi-soi];
                _leahTexture = [self loadJPEGFromData:jpeg];
            }
        }
    }

    NSLog(@"Leah: %d verts, %d tris, center=(%.1f,%.1f,%.1f), diam=%.1f%@",
          pc, tc, _leahCenter.x, _leahCenter.y, _leahCenter.z, _leahDiameter,
          _leahTexture ? @"" : @" [no texture]");
}

// ── Leah pipeline ─────────────────────────────────────────────────────────────

- (void)buildLeahPipeline {
    NSError *err = nil;
    id<MTLLibrary> lib = [_device newLibraryWithSource:kLeahShaders options:nil error:&err];
    if (!lib) { NSLog(@"Leah shader error: %@", err); return; }

    MTLVertexDescriptor *vd = [MTLVertexDescriptor new];
    vd.attributes[0].format = MTLVertexFormatFloat3; vd.attributes[0].offset =  0; vd.attributes[0].bufferIndex = 0;
    vd.attributes[1].format = MTLVertexFormatFloat3; vd.attributes[1].offset = 12; vd.attributes[1].bufferIndex = 0;
    vd.attributes[2].format = MTLVertexFormatFloat2; vd.attributes[2].offset = 24; vd.attributes[2].bufferIndex = 0;
    vd.layouts[0].stride = 32;

    MTLRenderPipelineDescriptor *pd = [MTLRenderPipelineDescriptor new];
    pd.vertexFunction                  = [lib newFunctionWithName:@"leah_vert"];
    pd.fragmentFunction                = [lib newFunctionWithName:@"leah_frag"];
    pd.vertexDescriptor                = vd;
    pd.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pd.depthAttachmentPixelFormat      = MTLPixelFormatDepth32Float;
    _leahPipeline = [_device newRenderPipelineStateWithDescriptor:pd error:&err];
    if (!_leahPipeline) NSLog(@"Leah pipeline error: %@", err);
}

// ── Render loop ───────────────────────────────────────────────────────────────

static CVReturn logoCB(CVDisplayLinkRef dl, const CVTimeStamp *now,
                       const CVTimeStamp *out, CVOptionFlags fi,
                       CVOptionFlags *fo, void *ctx) {
    [(__bridge LogoIntroView *)ctx render];
    return kCVReturnSuccess;
}

- (void)render {
    if (_scene == 2) { [self renderGBoat]; return; }
    if (_scene == 1) { [self renderLeah];  return; }

    // ── Compute local animation time (centiseconds) ──
    NSTimeInterval elapsed = -[_startTime timeIntervalSinceNow];
    int32_t localTime = (int32_t)(elapsed * 100.0);
    if (_scriptLengthCs > 0 && localTime > (int32_t)_scriptLengthCs)
        localTime = localTime % (int32_t)_scriptLengthCs;

    // ── Evaluate animations → update shape model matrices ──
    for (NSUInteger i = 0; i < _animCount; ++i) {
        LogoAnimData *an = &_anims[i];
        if (an->shape_idx < 0) continue;
        _shapes[an->shape_idx].model_mat = logo_eval_anim(an, localTime);
    }

    // ── Camera: use camr animation translation as eye, look at scene center ──
    simd_float3 camEye  = simd_make_float3(_sceneCx, _sceneCy + _sceneRadius*0.4f,
                                            _sceneCz - _sceneRadius*3.0f);
    simd_float3 camLook = simd_make_float3(_sceneCx, _sceneCy, _sceneCz);
    for (NSUInteger i = 0; i < _animCount; ++i) {
        if (memcmp(_anims[i].target_id, "camr", 4) == 0) {
            simd_float4x4 cm = logo_eval_anim(&_anims[i], localTime);
            camEye = simd_make_float3(cm.columns[3].x, cm.columns[3].y, cm.columns[3].z);
            break;
        }
    }

    id<CAMetalDrawable> drawable = [_metalLayer nextDrawable];
    if (!drawable) return;
    CGSize sz = _metalLayer.drawableSize;

    if (!_depthTex || _depthTex.width != (NSUInteger)sz.width
                   || _depthTex.height != (NSUInteger)sz.height) {
        MTLTextureDescriptor *dd = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
            width:(NSUInteger)sz.width height:(NSUInteger)sz.height mipmapped:NO];
        dd.usage = MTLTextureUsageRenderTarget; dd.storageMode = MTLStorageModePrivate;
        _depthTex = [_device newTextureWithDescriptor:dd];
    }

    float aspect = (float)(sz.width / sz.height);
    simd_float4x4 proj = logo_mat_perspective(2.f * atanf(0.5f / aspect), aspect, 0.06f, 60.f);
    simd_float4x4 view = logo_mat_lookAt(camEye, camLook, simd_make_float3(0,1,0));
    simd_float4x4 vp   = simd_mul(proj, view);

    MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor new];
    pass.colorAttachments[0].texture     = drawable.texture;
    pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
    pass.colorAttachments[0].clearColor  = MTLClearColorMake(0, 0, 0, 1);
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    pass.depthAttachment.texture         = _depthTex;
    pass.depthAttachment.loadAction      = MTLLoadActionClear;
    pass.depthAttachment.clearDepth      = 1.0;
    pass.depthAttachment.storeAction     = MTLStoreActionDontCare;

    id<MTLCommandBuffer>        cmd = [_commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> enc = [cmd renderCommandEncoderWithDescriptor:pass];
    [enc setRenderPipelineState:_logoPipeline];
    [enc setDepthStencilState:_depthState];
    [enc setCullMode:MTLCullModeBack];
    [enc setFrontFacingWinding:MTLWindingClockwise];

    for (NSUInteger i = 0; i < _shapeCount; ++i) {
        if (memcmp(_shapes[i].shape_id, "camr", 4) == 0) continue;
        LogoUni uni = { .model = _shapes[i].model_mat, .vp = vp };
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

- (void)renderLeah {
    if (!_leahPipeline || !_leahVertexBuf) return;

    _leahAngle += 0.008f;

    id<CAMetalDrawable> drawable = [_metalLayer nextDrawable];
    if (!drawable) return;
    CGSize sz = _metalLayer.drawableSize;

    if (!_depthTex || _depthTex.width != (NSUInteger)sz.width
                   || _depthTex.height != (NSUInteger)sz.height) {
        MTLTextureDescriptor *dd = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
            width:(NSUInteger)sz.width height:(NSUInteger)sz.height mipmapped:NO];
        dd.usage = MTLTextureUsageRenderTarget; dd.storageMode = MTLStorageModePrivate;
        _depthTex = [_device newTextureWithDescriptor:dd];
    }

    float aspect  = (float)(sz.width / sz.height);
    float camDist = _leahDiameter * 1.6f;
    simd_float3 eye = simd_make_float3(_leahCenter.x, _leahCenter.y,
                                        _leahCenter.z + camDist);
    simd_float4x4 proj  = logo_mat_perspective(M_PI / 4.0, aspect, 0.1f, 500.f);
    simd_float4x4 view  = logo_mat_lookAt(eye, _leahCenter, simd_make_float3(0,1,0));
    simd_float4x4 rotY  = mat_rotY(_leahAngle);
    simd_float4x4 toO   = mat_translate(-_leahCenter.x, -_leahCenter.y, -_leahCenter.z);
    simd_float4x4 fromO = mat_translate( _leahCenter.x,  _leahCenter.y,  _leahCenter.z);
    simd_float4x4 model = simd_mul(simd_mul(fromO, rotY), toO);
    simd_float4x4 mvp   = simd_mul(simd_mul(proj, view), model);
    LeahUni uni = { .mvp = mvp, .rotY = rotY };

    MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor new];
    pass.colorAttachments[0].texture     = drawable.texture;
    pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
    pass.colorAttachments[0].clearColor  = MTLClearColorMake(0.08, 0.08, 0.12, 1);
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    pass.depthAttachment.texture         = _depthTex;
    pass.depthAttachment.loadAction      = MTLLoadActionClear;
    pass.depthAttachment.clearDepth      = 1.0;
    pass.depthAttachment.storeAction     = MTLStoreActionDontCare;

    id<MTLCommandBuffer>        cmd = [_commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> enc = [cmd renderCommandEncoderWithDescriptor:pass];
    [enc setRenderPipelineState:_leahPipeline];
    [enc setDepthStencilState:_depthState];
    [enc setCullMode:MTLCullModeBack];
    [enc setFrontFacingWinding:MTLWindingClockwise];
    [enc setVertexBuffer:_leahVertexBuf  offset:0 atIndex:0];
    [enc setVertexBytes:&uni  length:sizeof(uni) atIndex:1];
    [enc setFragmentBytes:&uni length:sizeof(uni) atIndex:1];
    [enc setFragmentTexture:(_leahTexture ?: _whiteTex) atIndex:0];
    [enc drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                    indexCount:_leahIndexCount
                     indexType:MTLIndexTypeUInt32
                   indexBuffer:_leahIndexBuf
             indexBufferOffset:0];
    [enc endEncoding];
    [cmd presentDrawable:drawable];
    [cmd commit];
}

// ── G-Boat asset loading ──────────────────────────────────────────────────────
// Same MetaFile format as Leah; multiple shapes each with their own texture.
// Vertex positions/normals are transformed into a common model space at load time.

- (void)loadGBoatAssets {
    NSString *cwd  = NSFileManager.defaultManager.currentDirectoryPath;
    NSString *path = [[cwd stringByAppendingPathComponent:@"Assets"]
                           stringByAppendingPathComponent:@"G-Boat"];
    NSData *data = [NSData dataWithContentsOfFile:path];
    if (!data) { NSLog(@"GBoat: MetaFile not found at %@", path); return; }

    const uint8_t *b   = data.bytes;
    NSUInteger     len = data.length;
    if (len < 16) return;

    int32_t  ttPos    = mf_s32(b + 8);
    uint32_t tagCount = mf_u32(b + ttPos);
    NSUInteger eBase  = (NSUInteger)ttPos + 4;

    // ── Count renderable shapes ────────────────────────────────────────────
    uint32_t shapeCount = 0;
    for (uint32_t i = 0; i < tagCount; ++i) {
        const uint8_t *e = b + eBase + i * 24;
        if (eBase + i * 24 + 24 > len) break;
        if (!mf_4cc(e + 4, "shap")) continue;
        int32_t spos = mf_s32(e + 20);
        if (spos >= 0 && (NSUInteger)spos + 228 <= len) {
            uint16_t shading = (uint16_t)(b[spos+226]<<8 | b[spos+227]);
            if (shading < 16) shapeCount++;
        }
    }
    _gboatShapes = calloc(shapeCount, sizeof(ModelShapeData));

    // ── Parse shapes ───────────────────────────────────────────────────────
    uint32_t shapIdx = 0;
    float bxn=FLT_MAX,byn=FLT_MAX,bzn=FLT_MAX;
    float bxx=-FLT_MAX,byx=-FLT_MAX,bzx=-FLT_MAX;

    for (uint32_t i = 0; i < tagCount; ++i) {
        const uint8_t *e = b + eBase + i * 24;
        if (eBase + i * 24 + 24 > len) break;
        if (!mf_4cc(e + 4, "shap")) continue;

        int32_t spos = mf_s32(e + 20);
        if (spos < 0 || (NSUInteger)spos + 300 > len) continue;
        const uint8_t *sh = b + spos;

        uint16_t shading = (uint16_t)(sh[226]<<8 | sh[227]);
        if (shading >= 16) continue;

        int32_t pc = mf_s32(sh + 0), tc = mf_s32(sh + 8);
        if (pc <= 0 || tc <= 0) continue;

        // Shape local matrix
        float m[4][3];
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 3; c++)
                m[r][c] = mf_f32(sh + 36 + (r*3 + c)*4);

        // Find child pnts/trig/nrls (depth-bounded)
        uint8_t childDepth = e[0] + 1;
        int32_t pntsPos = -1, trigPos = -1, nrlsPos = -1, nrlsSize = 0;
        for (uint32_t j = i + 1; j < tagCount; ++j) {
            const uint8_t *ce = b + eBase + j * 24;
            if (eBase + j * 24 + 24 > len) break;
            if (ce[0] < childDepth) break;
            if (!mf_4cc(ce + 4, "<-->")) continue;
            int32_t cpos = mf_s32(ce + 20), csz = mf_s32(ce + 16);
            if (mf_4cc(ce + 8, "pnts") && pntsPos < 0) pntsPos = cpos;
            if (mf_4cc(ce + 8, "trig") && trigPos < 0) trigPos = cpos;
            if (mf_4cc(ce + 8, "nrls") && nrlsPos < 0) { nrlsPos = cpos; nrlsSize = csz; }
        }
        if (pntsPos < 0 || trigPos < 0) continue;
        if ((NSUInteger)pntsPos + pc*24 > len || (NSUInteger)trigPos + tc*12 > len) continue;

        const uint8_t *pnts = b + pntsPos;
        const uint8_t *trig = b + trigPos;
        const uint8_t *nrls = (nrlsPos >= 0 && nrlsSize >= pc*12 &&
                               (NSUInteger)nrlsPos + pc*12 <= len) ? b + nrlsPos : NULL;

        // Compute normals if not stored
        float *computed = NULL;
        if (!nrls) {
            computed = calloc(pc * 3, sizeof(float));
            for (int ti = 0; ti < tc; ti++) {
                uint32_t ai = mf_u32(trig+ti*12+0), bi2 = mf_u32(trig+ti*12+4), ci2 = mf_u32(trig+ti*12+8);
                float ap[3]={ mf_f32(pnts+ai*24),mf_f32(pnts+ai*24+4),mf_f32(pnts+ai*24+8) };
                float bp[3]={ mf_f32(pnts+bi2*24),mf_f32(pnts+bi2*24+4),mf_f32(pnts+bi2*24+8) };
                float cp2[3]={ mf_f32(pnts+ci2*24),mf_f32(pnts+ci2*24+4),mf_f32(pnts+ci2*24+8) };
                float aw[3],bw[3],cw[3];
                mf_xform_pt(ap,m,aw); mf_xform_pt(bp,m,bw); mf_xform_pt(cp2,m,cw);
                float ux=bw[0]-aw[0],uy=bw[1]-aw[1],uz=bw[2]-aw[2];
                float vx=cw[0]-aw[0],vy=cw[1]-aw[1],vz=cw[2]-aw[2];
                float nx=uy*vz-uz*vy,ny=uz*vx-ux*vz,nz=ux*vy-uy*vx;
                uint32_t face[3]={ai,bi2,ci2};
                for (int fi=0;fi<3;fi++) {
                    uint32_t idx=face[fi];
                    if (idx<(uint32_t)pc){computed[idx*3]+=nx;computed[idx*3+1]+=ny;computed[idx*3+2]+=nz;}
                }
            }
            for (int vi=0;vi<pc;vi++){
                float *n=computed+vi*3; float l=sqrtf(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
                if(l>1e-6f){n[0]/=l;n[1]/=l;n[2]/=l;}else{n[0]=0;n[1]=1;n[2]=0;}
            }
        }

        // Build vertex buffer (world-space positions/normals)
        float *vbuf = malloc(pc * 32);
        for (int vi = 0; vi < pc; vi++) {
            const uint8_t *vp = pnts + vi * 24;
            float lp[3] = { mf_f32(vp), mf_f32(vp+4), mf_f32(vp+8) };
            float wp[3]; mf_xform_pt(lp, m, wp);
            float wn[3];
            if (nrls) {
                float ln[3]={ mf_f32(nrls+vi*12),mf_f32(nrls+vi*12+4),mf_f32(nrls+vi*12+8) };
                mf_xform_nrm(ln, m, wn);
            } else {
                wn[0]=computed[vi*3]; wn[1]=computed[vi*3+1]; wn[2]=computed[vi*3+2];
            }
            float *dst = vbuf + vi * 8;
            dst[0]=wp[0]; dst[1]=wp[1]; dst[2]=wp[2];
            dst[3]=wn[0]; dst[4]=wn[1]; dst[5]=wn[2];
            dst[6]=mf_f32(vp+12); dst[7]=1.0f-mf_f32(vp+16);
            bxn=fminf(bxn,wp[0]); bxx=fmaxf(bxx,wp[0]);
            byn=fminf(byn,wp[1]); byx=fmaxf(byx,wp[1]);
            bzn=fminf(bzn,wp[2]); bzx=fmaxf(bzx,wp[2]);
        }
        free(computed);
        [_gboatVertexBufs addObject:[_device newBufferWithBytes:vbuf length:pc*32 options:0]];
        free(vbuf);

        // Index buffer
        uint32_t *ibuf = malloc(tc * 12);
        for (int ti = 0; ti < tc; ti++) {
            ibuf[ti*3+0]=mf_u32(trig+ti*12+0);
            ibuf[ti*3+1]=mf_u32(trig+ti*12+4);
            ibuf[ti*3+2]=mf_u32(trig+ti*12+8);
        }
        [_gboatIndexBufs addObject:[_device newBufferWithBytes:ibuf length:tc*12 options:0]];
        free(ibuf);

        ModelShapeData *sd = &_gboatShapes[shapIdx];
        memcpy(sd->tex_id, sh + 200, 4);
        sd->index_count = (uint32_t)tc * 3;
        char sid[5]={ sh[296+0],sh[296+1],sh[296+2],sh[296+3],0 };
        NSLog(@"  GBoat shape '%s': %d verts, %d tris", sid, pc, tc);
        shapIdx++;
    }
    _gboatShapeCount = shapIdx;

    _gboatCenter   = simd_make_float3((bxn+bxx)/2, (byn+byx)/2, (bzn+bzx)/2);
    _gboatDiameter = fmaxf(fmaxf(bxx-bxn, byx-byn), bzx-bzn);

    // ── Extract textures ───────────────────────────────────────────────────
    for (uint32_t i = 0; i < tagCount; ++i) {
        const uint8_t *e = b + eBase + i * 24;
        if (eBase + i * 24 + 24 > len) break;
        if (!mf_4cc(e + 4, "cptx")) continue;
        uint8_t cptxDepth = e[0];
        for (uint32_t j = i + 1; j < tagCount; ++j) {
            const uint8_t *ne = b + eBase + j * 24;
            if (eBase + j * 24 + 24 > len) break;
            if (ne[0] <= cptxDepth && !mf_4cc(ne + 4, "<-->")) break;
            if (!mf_4cc(ne + 4, "<-->") || !mf_4cc(ne + 8, "txim")) continue;
            int32_t tpos = mf_s32(ne + 20), tsz = mf_s32(ne + 16);
            if (tpos < 0 || tsz < 2 || (NSUInteger)tpos + tsz > len) break;
            const uint8_t *pict = b + tpos;
            NSInteger soi = -1, eoi = -1;
            for (int32_t k = 0; k < tsz - 1; k++) {
                if (pict[k]==0xFF && pict[k+1]==0xD8 && soi < 0) soi = k;
                if (pict[k]==0xFF && pict[k+1]==0xD9)             eoi = k + 2;
            }
            if (soi >= 0 && eoi > soi) {
                NSData *jpeg = [NSData dataWithBytes:pict+soi length:eoi-soi];
                id<MTLTexture> tex = [self loadJPEGFromData:jpeg];
                if (tex) {
                    NSData *key = [NSData dataWithBytes:e + 12 length:4];
                    if (!_gboatTexMap[key]) {  // keep first occurrence per id
                        _gboatTexMap[key] = tex;
                        char tid[5]={e[12],e[13],e[14],e[15],0};
                        NSLog(@"  GBoat texture '%s' loaded", tid);
                    }
                }
            }
            break;
        }
    }

    NSLog(@"GBoat: %lu shapes, center=(%.1f,%.1f,%.1f), diam=%.1f",
          (unsigned long)_gboatShapeCount,
          _gboatCenter.x, _gboatCenter.y, _gboatCenter.z, _gboatDiameter);
}

// ── G-Boat render ─────────────────────────────────────────────────────────────

- (void)renderGBoat {
    if (!_leahPipeline || _gboatShapeCount == 0) return;

    _gboatAngle += 0.008f;

    id<CAMetalDrawable> drawable = [_metalLayer nextDrawable];
    if (!drawable) return;
    CGSize sz = _metalLayer.drawableSize;

    if (!_depthTex || _depthTex.width != (NSUInteger)sz.width
                   || _depthTex.height != (NSUInteger)sz.height) {
        MTLTextureDescriptor *dd = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
            width:(NSUInteger)sz.width height:(NSUInteger)sz.height mipmapped:NO];
        dd.usage = MTLTextureUsageRenderTarget; dd.storageMode = MTLStorageModePrivate;
        _depthTex = [_device newTextureWithDescriptor:dd];
    }

    float aspect  = (float)(sz.width / sz.height);
    float camDist = _gboatDiameter * 1.6f;
    simd_float3 eye = simd_make_float3(_gboatCenter.x, _gboatCenter.y,
                                        _gboatCenter.z + camDist);
    simd_float4x4 proj  = logo_mat_perspective(M_PI / 4.0, aspect, 0.1f, 500.f);
    simd_float4x4 view  = logo_mat_lookAt(eye, _gboatCenter, simd_make_float3(0,1,0));
    simd_float4x4 rotY  = mat_rotY(_gboatAngle);
    simd_float4x4 toO   = mat_translate(-_gboatCenter.x, -_gboatCenter.y, -_gboatCenter.z);
    simd_float4x4 fromO = mat_translate( _gboatCenter.x,  _gboatCenter.y,  _gboatCenter.z);
    simd_float4x4 model = simd_mul(simd_mul(fromO, rotY), toO);
    simd_float4x4 mvp   = simd_mul(simd_mul(proj, view), model);
    LeahUni uni = { .mvp = mvp, .rotY = rotY };

    MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor new];
    pass.colorAttachments[0].texture     = drawable.texture;
    pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
    pass.colorAttachments[0].clearColor  = MTLClearColorMake(0.08, 0.08, 0.12, 1);
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    pass.depthAttachment.texture         = _depthTex;
    pass.depthAttachment.loadAction      = MTLLoadActionClear;
    pass.depthAttachment.clearDepth      = 1.0;
    pass.depthAttachment.storeAction     = MTLStoreActionDontCare;

    id<MTLCommandBuffer>        cmd = [_commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> enc = [cmd renderCommandEncoderWithDescriptor:pass];
    [enc setRenderPipelineState:_leahPipeline];
    [enc setDepthStencilState:_depthState];
    [enc setCullMode:MTLCullModeBack];
    [enc setFrontFacingWinding:MTLWindingClockwise];
    [enc setVertexBytes:&uni  length:sizeof(uni) atIndex:1];
    [enc setFragmentBytes:&uni length:sizeof(uni) atIndex:1];

    for (NSUInteger i = 0; i < _gboatShapeCount; ++i) {
        NSData *key = [NSData dataWithBytes:_gboatShapes[i].tex_id length:4];
        id<MTLTexture> tex = _gboatTexMap[key] ?: _whiteTex;
        [enc setVertexBuffer:_gboatVertexBufs[i] offset:0 atIndex:0];
        [enc setFragmentTexture:tex atIndex:0];
        [enc drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                        indexCount:_gboatShapes[i].index_count
                         indexType:MTLIndexTypeUInt32
                       indexBuffer:_gboatIndexBufs[i]
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
    free(_gboatShapes);
}

@end
