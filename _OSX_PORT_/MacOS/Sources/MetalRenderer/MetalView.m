#import "MetalRenderer.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <CoreVideo/CVDisplayLink.h>
#import <math.h>

static NSString * const kShaderSource =
    @"#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct VertexOut { float4 pos [[position]]; };\n"
    "vertex VertexOut vert(uint vid [[vertex_id]],\n"
    "                      constant float2 *verts [[buffer(0)]],\n"
    "                      constant float2x2 &rot  [[buffer(1)]]) {\n"
    "    VertexOut o;\n"
    "    o.pos = float4(rot * verts[vid], 0.0, 1.0);\n"
    "    return o;\n"
    "}\n"
    "fragment float4 frag(VertexOut in [[stage_in]]) {\n"
    "    return float4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

@interface MetalView () {
    CVDisplayLinkRef _displayLink;
}
@property (nonatomic) id<MTLDevice>              device;
@property (nonatomic) id<MTLCommandQueue>        commandQueue;
@property (nonatomic) id<MTLRenderPipelineState> pipeline;
@property (nonatomic) id<MTLBuffer>              vertexBuffer;
@property (nonatomic) CAMetalLayer              *metalLayer;
@property (nonatomic) float                      angle;
@end

@implementation MetalView

- (instancetype)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (!self) return nil;

    _device = MTLCreateSystemDefaultDevice();
    self.wantsLayer = YES;  // triggers makeBackingLayer

    _metalLayer = (CAMetalLayer *)self.layer;
    _metalLayer.device        = _device;
    _metalLayer.pixelFormat   = MTLPixelFormatBGRA8Unorm;
    _metalLayer.drawableSize  = CGSizeMake(frame.size.width * 2, frame.size.height * 2);

    _commandQueue = [_device newCommandQueue];

    NSError *err = nil;
    id<MTLLibrary> lib = [_device newLibraryWithSource:kShaderSource options:nil error:&err];
    if (!lib) { NSLog(@"Shader error: %@", err); return nil; }

    MTLRenderPipelineDescriptor *desc = [MTLRenderPipelineDescriptor new];
    desc.vertexFunction                        = [lib newFunctionWithName:@"vert"];
    desc.fragmentFunction                      = [lib newFunctionWithName:@"frag"];
    desc.colorAttachments[0].pixelFormat       = MTLPixelFormatBGRA8Unorm;
    _pipeline = [_device newRenderPipelineStateWithDescriptor:desc error:&err];
    if (!_pipeline) { NSLog(@"Pipeline error: %@", err); return nil; }

    // Two triangles forming a square, coordinates in [-0.5, 0.5]
    float verts[] = {
        -0.5f, -0.5f,   0.5f, -0.5f,   -0.5f,  0.5f,
         0.5f, -0.5f,   0.5f,  0.5f,   -0.5f,  0.5f,
    };
    _vertexBuffer = [_device newBufferWithBytes:verts length:sizeof(verts)
                                        options:MTLResourceStorageModeShared];

    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, displayCallback, (__bridge void *)self);
    CVDisplayLinkStart(_displayLink);

    return self;
}

- (CALayer *)makeBackingLayer { return [CAMetalLayer layer]; }

static CVReturn displayCallback(CVDisplayLinkRef dl, const CVTimeStamp *now,
                                const CVTimeStamp *out, CVOptionFlags fi,
                                CVOptionFlags *fo, void *ctx) {
    [(__bridge MetalView *)ctx render];
    return kCVReturnSuccess;
}

- (void)render {
    _angle += 0.01f;
    float c = cosf(_angle), s = sinf(_angle);
    CGSize sz = _metalLayer.drawableSize;
    float aspect = (float)(sz.width / sz.height);
    // column-major float2x2 with aspect correction: scale x by 1/aspect after rotation
    float rot[4] = { c/aspect, s, -s/aspect, c };

    id<CAMetalDrawable> drawable = [_metalLayer nextDrawable];
    if (!drawable) return;

    MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor new];
    pass.colorAttachments[0].texture     = drawable.texture;
    pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
    pass.colorAttachments[0].clearColor  = MTLClearColorMake(0, 0, 0, 1);
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;

    id<MTLCommandBuffer>        cmd = [_commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> enc = [cmd renderCommandEncoderWithDescriptor:pass];

    [enc setRenderPipelineState:_pipeline];
    [enc setVertexBuffer:_vertexBuffer offset:0 atIndex:0];
    [enc setVertexBytes:rot length:sizeof(rot) atIndex:1];
    [enc drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
    [enc endEncoding];

    [cmd presentDrawable:drawable];
    [cmd commit];
}

- (void)setFrameSize:(NSSize)newSize {
    [super setFrameSize:newSize];
    _metalLayer.drawableSize = CGSizeMake(newSize.width * 2, newSize.height * 2);
}

- (void)dealloc {
    CVDisplayLinkStop(_displayLink);
    CVDisplayLinkRelease(_displayLink);
}

@end
