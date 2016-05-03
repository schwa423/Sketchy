#include <metal_stdlib>

using namespace metal;

// Format of vertices produced by the tesellator, and consumed by the vertex shader.
struct StrokeVertexIn {
    float4 pos;
    float2 norm;
    float length;
};

// Format of vertices output by the vertex shader.
struct StrokeVertexOut {
  float4  position [[position]];
  float4  color;
  float sz [[point_size]];
};

// Vertex shader.
vertex StrokeVertexOut strokeVertex(uint vid [[ vertex_id ]],
                                    constant StrokeVertexIn* vertices [[ buffer(0) ]],
                                    constant float* lengthAndReciprocal [[buffer(1)]],
                                    constant float* time [[buffer(2)]])
{
  StrokeVertexIn in = vertices[vid];
  StrokeVertexOut out;

  constexpr float width = 0.05;
  // Domain of amplitude is [0, 0.5] because range of sin+1 is [0,2].
  constexpr float amplitude = 0.4;

  out.position = in.pos;
  out.position.xy += in.norm * width * (1.0 - amplitude * (1.0 + sin(50 * (in.length - time[0]))));
  // Use reciprocal-length so that the tip of the stroke is white, and the tail is black.
  out.color.rgb = float3(in.length * lengthAndReciprocal[1]);
  out.color.a = 0.5;

  return out;
};

// Fragment shader.
fragment half4 strokeFragmentPassThrough(StrokeVertexOut inFrag [[stage_in]])
{
    return half4(inFrag.color);
};

// Format of stroke segments that are used by the tesselator.
struct StrokeSegment {
  float2 bez[4];
  float reparam[4];
  float length;
};

// Compute shader that tesselates strokes into triangle-strips.
kernel void strokeBezierTesselate(constant StrokeSegment* seg [[ buffer(0) ]],
                                  constant float* startAndTotalLength [[ buffer(1) ]],
                                  constant float* t_divisor [[ buffer(2) ]],
                                  device StrokeVertexIn* vertices [[ buffer(3) ]],
                                  uint2 gid [[ thread_position_in_grid ]]) {
  // TODO: Any potential performance increace by passing in t_divisor reciprocal?
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

  // Compute position, normal, and arc-length of vertex.
  float2 p0 = seg->bez[0] * omt + seg->bez[1] * t;
  float2 p1 = seg->bez[1] * omt + seg->bez[2] * t;
  float2 p2 = seg->bez[2] * omt + seg->bez[3] * t;
  float2 p01 = p0 * omt + p1 * t;
  float2 p12 = p1 * omt + p2 * t;
  float2 p = p01 * omt + p12 * t;
  float2 tangent = normalize(p12 - p01);
  float2 normal = float2(-tangent.y, tangent.x);
  float length = startAndTotalLength[0] + t * seg->length;
  
  // Stroke width tapers at the tip and tail.
  constexpr float cap_length = 5.0f;
  float end_dist = min(length, startAndTotalLength[1] - length);
  float cap = max(0.0f, 1.0f - end_dist * cap_length);
  normal *= (1.0f - cap * cap);
  
  // Output 2 vertices, with the same position but opposite normals.
  uint index = gid.x * 2;
  vertices[index].pos.xy = p;
  vertices[index].pos.zw = float2(0,1);
  vertices[index].norm = normal;
  vertices[index].length = length;
  vertices[index + 1].pos.xy = p;
  vertices[index + 1].pos.zw = float2(0,1);
  vertices[index + 1].norm = -normal;
  vertices[index + 1].length = length;
}
