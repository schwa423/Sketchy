#include <metal_stdlib>

using namespace metal;

struct StrokeVertexOut {
    float4  position [[position]];
    float4  color;
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

  return out;
};

fragment half4 strokeFragmentPassThrough(StrokeVertexOut inFrag [[stage_in]])
{
    return half4(inFrag.color);
};
