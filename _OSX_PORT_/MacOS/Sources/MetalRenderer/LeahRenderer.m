#import "MetalRenderer.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <CoreVideo/CVDisplayLink.h>
#import <ImageIO/ImageIO.h>
#import <simd/simd.h>
#import <math.h>

// ── Shaders ───────────────────────────────────────────────────────────────────
// Vertex layout (must match leah.bin, 32 bytes per vertex):
//   float3 position  @ offset  0
//   float3 normal    @ offset 12
//   float2 uv        @ offset 24

static NSString * const kLeahShaders =
    @"#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct Vin  { float3 pos [[attribute(0)]]; float3 nrm [[attribute(1)]]; float2 uv [[attribute(2)]]; };\n"
    "struct Vout { float4 pos [[position]]; float3 nrm; float2 uv; };\n"
    "struct Uni  { float4x4 mvp; float4x4 rotY; };\n"
    "vertex Vout leah_vert(Vin in [[stage_in]], constant Uni &u [[buffer(1)]]) {\n"
    "    Vout o;\n"
    "    o.pos = u.mvp  * float4(in.pos, 1.0);\n"
    "    o.nrm = (u.rotY * float4(in.nrm, 0.0)).xyz;\n"
    "    o.uv  = in.uv;\n"
    "    return o;\n"
    "}\n"
    "fragment float4 leah_frag(Vout in [[stage_in]],\n"
    "                          texture2d<float> tex [[texture(0)]],\n"
    "                          constant Uni &u [[buffer(1)]]) {\n"
    "    constexpr sampler s(filter::linear, address::clamp_to_edge);\n"
    "    float4 c = tex.sample(s, in.uv);\n"
    "    float3 L = normalize(float3(0.6, 1.0, 0.8));\n"
    "    float  d = max(0.0f, dot(normalize(in.nrm), L));\n"
    "    return float4(c.rgb * (0.35f + 0.65f * d), c.a);\n"
    "}\n";

// ── Uniform struct (mirrors shader) ───────────────────────────────────────────
typedef struct { simd_float4x4 mvp; simd_float4x4 rotY; } LeahUniforms;

// ── Matrix helpers ─────────────────────────────────────────────────────────────

static simd_float4x4 mat_perspective(float fovY, float aspect, float nr, float fr) {
    float fy = 1.f / tanf(fovY * .5f);
    float dz = nr - fr;
    return (simd_float4x4){ .columns = {
        { fy/aspect, 0,       0,           0 },
        { 0,         fy,      0,           0 },
        { 0,         0,  fr/dz,          -1 },
        { 0,         0,  fr*nr/dz,        0 },
    }};
}

static simd_float4x4 mat_lookAt(simd_float3 eye, simd_float3 ctr, simd_float3 up) {
    simd_float3 f = simd_normalize(eye - ctr);           // backward from scene
    simd_float3 r = simd_normalize(simd_cross(up, f));   // right
    simd_float3 u = simd_cross(f, r);                    // corrected up
    return (simd_float4x4){ .columns = {
        { r.x, u.x, f.x, 0 },
        { r.y, u.y, f.y, 0 },
        { r.z, u.z, f.z, 0 },
        { -simd_dot(r,eye), -simd_dot(u,eye), -simd_dot(f,eye), 1 },
    }};
}

static simd_float4x4 mat_rotY(float a) {
    float c = cosf(a), s = sinf(a);
    return (simd_float4x4){ .columns = {
        {  c, 0, -s, 0 },
        {  0, 1,  0, 0 },
        {  s, 0,  c, 0 },
        {  0, 0,  0, 1 },
    }};
}

static simd_float4x4 mat_translate(float tx, float ty, float tz) {
    return (simd_float4x4){ .columns = {
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        { tx, ty, tz, 1 },
    }};
}

// ── LeahView ──────────────────────────────────────────────────────────────────

@interface LeahView () {
    CVDisplayLinkRef _displayLink;
}
@property id<MTLDevice>              device;
@property id<MTLCommandQueue>        commandQueue;
@property id<MTLRenderPipelineState> pipeline;
@property id<MTLDepthStencilState>   depthState;
@property id<MTLBuffer>              vertexBuf;
@property id<MTLBuffer>              indexBuf;
@property id<MTLTexture>             texture;
@property id<MTLTexture>             depthTex;
@property CAMetalLayer              *metalLayer;
@property uint32_t                   indexCount;
@property simd_float3                center;
@property float                      diameter;
@property float                      angle;
@end

@implementation LeahView

- (instancetype)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (!self) return nil;

    _device = MTLCreateSystemDefaultDevice();
    self.wantsLayer = YES;
    _metalLayer = (CAMetalLayer *)self.layer;
    _metalLayer.device      = _device;
    _metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    _metalLayer.drawableSize = CGSizeMake(frame.size.width * 2, frame.size.height * 2);

    _commandQueue = [_device newCommandQueue];

    if (![self loadAssets] || ![self buildPipeline]) return nil;

    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, displayCB, (__bridge void *)self);
    CVDisplayLinkStart(_displayLink);
    return self;
}

- (CALayer *)makeBackingLayer { return [CAMetalLayer layer]; }

// ── Asset loading ─────────────────────────────────────────────────────────────

- (BOOL)loadAssets {
    NSString *cwd  = NSFileManager.defaultManager.currentDirectoryPath;
    NSString *adir = [cwd stringByAppendingPathComponent:@"LeahAssets"];

    // ── Mesh binary ──
    NSData *bin = [NSData dataWithContentsOfFile:
                   [adir stringByAppendingPathComponent:@"leah.bin"]];
    if (!bin) { NSLog(@"LeahRenderer: leah.bin not found in %@", adir); return NO; }

    const uint8_t *b = bin.bytes;
    size_t off = 0;

    uint32_t vc, ic;
    memcpy(&vc, b + off, 4); off += 4;
    memcpy(&ic, b + off, 4); off += 4;
    _indexCount = ic;

    float bbox[6];
    memcpy(bbox, b + off, 24); off += 24;
    _center   = simd_make_float3((bbox[0]+bbox[3])*.5f,
                                  (bbox[1]+bbox[4])*.5f,
                                  (bbox[2]+bbox[5])*.5f);
    _diameter = fmaxf(fmaxf(bbox[3]-bbox[0], bbox[4]-bbox[1]), bbox[5]-bbox[2]);

    size_t vbytes = vc * 8 * sizeof(float);
    size_t ibytes = ic * sizeof(uint32_t);
    _vertexBuf = [_device newBufferWithBytes:b+off length:vbytes options:0]; off += vbytes;
    _indexBuf  = [_device newBufferWithBytes:b+off length:ibytes options:0];

    // ── Texture ──
    NSURL *url = [NSURL fileURLWithPath:
                  [adir stringByAppendingPathComponent:@"leah_Tlea.jpg"]];
    CGImageSourceRef src = CGImageSourceCreateWithURL((__bridge CFURLRef)url, nil);
    if (!src) { NSLog(@"LeahRenderer: texture not found"); return NO; }
    CGImageRef img = CGImageSourceCreateImageAtIndex(src, 0, nil);
    CFRelease(src);

    size_t w = CGImageGetWidth(img), h = CGImageGetHeight(img);
    // Draw into RGBA bitmap. CGBitmapContext (Y-up) draws CGImage (Y-down)
    // without flipping → image stored bottom-row-first, matching WaterRace
    // V=0-at-bottom convention in Metal texture sampling.
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
    unsigned char  *px = calloc(w * h * 4, 1);
    CGContextRef    cx = CGBitmapContextCreate(px, w, h, 8, w*4, cs,
                             kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGContextDrawImage(cx, CGRectMake(0,0,w,h), img);
    CGContextRelease(cx); CGColorSpaceRelease(cs); CGImageRelease(img);

    MTLTextureDescriptor *td = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
        width:w height:h mipmapped:NO];
    _texture = [_device newTextureWithDescriptor:td];
    [_texture replaceRegion:MTLRegionMake2D(0,0,w,h)
                mipmapLevel:0 withBytes:px bytesPerRow:w*4];
    free(px);
    return YES;
}

// ── Pipeline ──────────────────────────────────────────────────────────────────

- (BOOL)buildPipeline {
    NSError *err = nil;
    id<MTLLibrary> lib = [_device newLibraryWithSource:kLeahShaders
                                               options:nil error:&err];
    if (!lib) { NSLog(@"Shader error: %@", err); return NO; }

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
    _pipeline = [_device newRenderPipelineStateWithDescriptor:pd error:&err];
    if (!_pipeline) { NSLog(@"Pipeline error: %@", err); return NO; }

    MTLDepthStencilDescriptor *dd = [MTLDepthStencilDescriptor new];
    dd.depthCompareFunction = MTLCompareFunctionLess;
    dd.depthWriteEnabled    = YES;
    _depthState = [_device newDepthStencilStateWithDescriptor:dd];
    return YES;
}

// ── Render loop ───────────────────────────────────────────────────────────────

static CVReturn displayCB(CVDisplayLinkRef dl, const CVTimeStamp *now,
                          const CVTimeStamp *out, CVOptionFlags fi,
                          CVOptionFlags *fo, void *ctx) {
    [(__bridge LeahView *)ctx render];
    return kCVReturnSuccess;
}

- (void)render {
    _angle += 0.008f;

    id<CAMetalDrawable> drawable = [_metalLayer nextDrawable];
    if (!drawable) return;

    CGSize sz = _metalLayer.drawableSize;

    // Recreate depth texture if size changed
    if (!_depthTex || _depthTex.width != (NSUInteger)sz.width
                   || _depthTex.height != (NSUInteger)sz.height) {
        MTLTextureDescriptor *dd = [MTLTextureDescriptor
            texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
            width:(NSUInteger)sz.width height:(NSUInteger)sz.height mipmapped:NO];
        dd.usage       = MTLTextureUsageRenderTarget;
        dd.storageMode = MTLStorageModePrivate;
        _depthTex = [_device newTextureWithDescriptor:dd];
    }

    // Matrices
    float aspect   = (float)(sz.width / sz.height);
    float camDist  = _diameter * 1.6f;
    simd_float3 eye = simd_make_float3(_center.x, _center.y, _center.z + camDist);

    simd_float4x4 proj  = mat_perspective(M_PI / 4.0, aspect, 0.1f, 500.f);
    simd_float4x4 view  = mat_lookAt(eye, _center, simd_make_float3(0,1,0));
    simd_float4x4 rotY  = mat_rotY(_angle);
    simd_float4x4 toO   = mat_translate(-_center.x, -_center.y, -_center.z);
    simd_float4x4 fromO = mat_translate( _center.x,  _center.y,  _center.z);
    // model = T(center) * R_y * T(-center)  — rotates around model center
    simd_float4x4 model = simd_mul(simd_mul(fromO, rotY), toO);
    simd_float4x4 mvp   = simd_mul(simd_mul(proj, view), model);

    LeahUniforms uni = { .mvp = mvp, .rotY = rotY };

    // Render pass
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

    [enc setRenderPipelineState:_pipeline];
    [enc setDepthStencilState:_depthState];
    [enc setCullMode:MTLCullModeBack];
    [enc setFrontFacingWinding:MTLWindingClockwise];
    [enc setVertexBuffer:_vertexBuf  offset:0 atIndex:0];
    [enc setVertexBytes:&uni  length:sizeof(uni) atIndex:1];
    [enc setFragmentBytes:&uni length:sizeof(uni) atIndex:1];
    [enc setFragmentTexture:_texture atIndex:0];
    [enc drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                    indexCount:_indexCount
                     indexType:MTLIndexTypeUInt32
                   indexBuffer:_indexBuf
             indexBufferOffset:0];
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
}

@end
