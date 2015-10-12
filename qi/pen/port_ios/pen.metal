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
    packed_uchar4 color;
};

vertex StrokeVertexOut strokeVertex(uint vid [[ vertex_id ]],
                                    constant StrokeVertexIn* vertices [[ buffer(0) ]])
{
  StrokeVertexIn in = vertices[vid];
  StrokeVertexOut out;

  constexpr float normalizeColor = 1.0 / 255.0;
  constexpr float width = 0.02;

  out.position = in.pos;
  out.position.xy += in.norm * width;
  out.color = static_cast<float4>(uchar4(in.color)) * normalizeColor;

  out.sz = 4.0f;

  return out;
};

fragment half4 strokeFragmentPassThrough(StrokeVertexOut inFrag [[stage_in]])
{
    return half4(inFrag.color);
};



struct Bezier2f {
  float2 pts[4];
};

kernel void strokeBezierTesselate(constant Bezier2f* bez [[ buffer(0) ]],
                                  device StrokeVertexIn* vertices [[ buffer(1) ]],
                                  uint2 gid [[ thread_position_in_grid ]]) {
  // The 63.0 is because:
  // - we assume that we tesselate 128 vertices per segment...
  //   see Page::ComputeVertexCounts())
  // - we want t to range from 0 to 1 (that's why 63 instead of 64)
  // - we generate 2 vertices for each value of t, each with opposite normals
  // TODO: make this more flexible
  float t = float(gid.x) / 63.0f;

  float omt = 1.0f - t;
  float2 p0 = bez->pts[0] * omt + bez->pts[1] * t;
  float2 p1 = bez->pts[1] * omt + bez->pts[2] * t;
  float2 p2 = bez->pts[2] * omt + bez->pts[3] * t;
  float2 p01 = p0 * omt + p1 * t;
  float2 p12 = p1 * omt + p2 * t;
  float2 p = p01 * omt + p12 * t;
  float2 tangent = normalize(p12 - p01);
  float2 normal = float2(-tangent.y, tangent.x);

  float4 color = float4(t * 255, t * 255, t * 255, 255);
  
  uint index = gid.x * 2;
  vertices[index].pos.xy = p;
  vertices[index].pos.zw = float2(0,1);
  vertices[index].norm = normal;
  vertices[index].length = 0;
  vertices[index].color = uchar4(color);
  vertices[index + 1].pos.xy = p;
  vertices[index + 1].pos.zw = float2(0,1);
  vertices[index + 1].norm = -normal;
  vertices[index + 1].length = 0;
  vertices[index + 1].color = uchar4(color);
}
