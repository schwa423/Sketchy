#include <metal_stdlib>

using namespace metal;

struct StrokeVertexOut {
    float4  position [[position]];
    float4  color;
    float sz [[point_size]];
};

struct StrokeVertexIn {
    float4 pos;
    float2 norm;
    float length;
    // TODO: this is no longer used
    packed_uchar4 color;
};

vertex StrokeVertexOut strokeVertex(uint vid [[ vertex_id ]],
                                    constant StrokeVertexIn* vertices [[ buffer(0) ]],
                                    constant float* lengthNormalizer [[buffer(1)]],
                                    constant float* time [[buffer(2)]])
{
  StrokeVertexIn in = vertices[vid];
  StrokeVertexOut out;

  constexpr float width = 0.02;

  out.position = in.pos;
  out.position.xy += in.norm * width * (1.5 + sin((in.length + time[0]) * 50));
  out.color.rgb = float3(in.length * lengthNormalizer[0]);
  out.color.a = 0.5;
  
  // TODO: I think this is no longer used
  out.sz = 4.0f;

  return out;
};

fragment half4 strokeFragmentPassThrough(StrokeVertexOut inFrag [[stage_in]])
{
    return half4(inFrag.color);
};

struct StrokeSegment {
  float2 bez[4];
  float reparam[4];
  float length;
};

kernel void strokeBezierTesselate(constant StrokeSegment* seg [[ buffer(0) ]],
                                  constant float* startLength [[ buffer(1) ]],
                                  constant float* t_divisor [[ buffer(2) ]],
                                  device StrokeVertexIn* vertices [[ buffer(3) ]],
                                  uint2 gid [[ thread_position_in_grid ]]) {
  // The 63.0 is because:
  // - we assume that we tesselate 128 vertices per segment...
  //   see Page::ComputeVertexCounts())
  // - we want t to range from 0 to 1 (that's why 63 instead of 64)
  // - we generate 2 vertices for each value of t, each with opposite normals
  // TODO: make this more flexible
  float t = float(gid.x) / t_divisor[0];
  float omt = 1.0f - t;

  // Perform arc-length reparameteriztion.
  float t0 = seg->reparam[0] * omt + seg->reparam[1] * t;
  float t1 = seg->reparam[1] * omt + seg->reparam[2] * t;
  float t2 = seg->reparam[2] * omt + seg->reparam[3] * t;
  float t01 = t0 * omt + t1 * t;
  float t12 = t1 * omt + t2 * t;
  t = t01 * omt + t12 * t;
  omt = 1.0f - t;

  float2 p0 = seg->bez[0] * omt + seg->bez[1] * t;
  float2 p1 = seg->bez[1] * omt + seg->bez[2] * t;
  float2 p2 = seg->bez[2] * omt + seg->bez[3] * t;
  float2 p01 = p0 * omt + p1 * t;
  float2 p12 = p1 * omt + p2 * t;
  float2 p = p01 * omt + p12 * t;
  float2 tangent = normalize(p12 - p01);
  float2 normal = float2(-tangent.y, tangent.x);
  float length = startLength[0] + t * seg->length;
  
  float4 color = float4(t * 255, t * 255, t * 255, 255);

  uint index = gid.x * 2;
  vertices[index].pos.xy = p;
  vertices[index].pos.zw = float2(0,1);
  vertices[index].norm = normal;
  vertices[index].length = length;
  vertices[index].color = uchar4(color);
  vertices[index + 1].pos.xy = p;
  vertices[index + 1].pos.zw = float2(0,1);
  vertices[index + 1].norm = -normal;
  vertices[index + 1].length = length;
  vertices[index + 1].color = uchar4(color);
}
