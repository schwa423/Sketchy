#include <metal_stdlib>

using namespace metal;

// Format of vertices produced by the tesellator, and consumed by the vertex shader.
struct StrokeVertexIn {
  float4 pos;
  float2 norm;
  float dir;  // direction of normal, also used for parameterization
  float length;
};

// Format of vertices output by the vertex shader.
struct StrokeVertexOut {
  float4 pos [[position]];
  float2 uv;
  float time;
};

// TODO: comment
struct StrokeParams {
  float length;
  float reciprocalLength;
  float width;
  float reciprocalWidth;
  float time;
};

// TODO: comment
struct SineParams {
  float amplitude;
  float period;
  float speed;
};

// Return a number between 0 and 1.
float sineFactor(SineParams params, float arcLength, float time) {
  return 1.0 - params.amplitude * (1.0 + sin(params.period * (arcLength - params.speed * time)));
}

// Vertex shader.
vertex StrokeVertexOut fractalTiling_vert(uint vid [[ vertex_id ]],
                                          constant StrokeVertexIn* vertices [[ buffer(0) ]],
                                          constant StrokeParams* strokeParams [[ buffer(1) ]],
                                          constant SineParams* sineParams [[buffer(2)]])
{
  StrokeVertexIn in = vertices[vid];
  StrokeParams stroke = strokeParams[0];
  StrokeVertexOut out;
  
  out.pos = in.pos;
  out.pos.xy += in.norm * in.dir * stroke.width * sineFactor(sineParams[0], in.length, stroke.time);
  
  // TODO: bias by some per-stroke value (perhaps stroke start-time?) so that the beginning of each
  // stroke doesn't look the same.  I tried to use the a multiple of the total stroke length... this
  // looked OK for finalized strokes, but caused strokes to flash wildly while being drawn.
  out.uv.x = in.length * stroke.reciprocalWidth;
  out.uv.y = in.dir;
  out.time = stroke.time;
  
  return out;
}

// Fragment shader.
// Credit: "Fractal Tiling" by Inigo Quilez, https://www.shadertoy.com/view/Ml2GWy
fragment half4 fractalTiling_frag(StrokeVertexOut inFrag [[stage_in]])
{
  float time = inFrag.time;
  
  // Bias tex-coords to make stroke look cylindrical.
  float2 pos = float2(inFrag.uv.x, asin(inFrag.uv.y));
  // TODO: make this match sine speed in vertex shader.
  pos -= float2(time);
  pos *= 32.0;
  float3 col = float3(0.0);
  
  for (int i = 0; i < 6; ++i) {
    float2 a = floor(pos);
    float2 b = fract(pos);
    
    float4 w = fract((sin(a.x*7.0+31.0*a.y + 0.01*time)+float4(0.035,0.01,0.0,0.7))*13.545317); // randoms
    
    col +=
        w.xyz *                                   // color
        smoothstep(0.45,0.55,w.w) *               // intensity
        sqrt( 16.0*b.x*b.y*(1.0-b.x)*(1.0-b.y) ); // pattern
    
    pos /= 2.0; // lacunarity
    col /= 2.0; // attenuate high frequencies
  }
  
  col = pow( 2.5*col, float3(1.0,1.0,0.7) );    // contrast and color shape
  return half4(col.x, col.y, col.z, 1.0);
}

// Vertex shader.
vertex StrokeVertexOut blackWhite_vert(uint vid [[ vertex_id ]],
                                       constant StrokeVertexIn* vertices [[ buffer(0) ]],
                                       constant StrokeParams* strokeParams [[ buffer(1) ]],
                                       constant SineParams* sineParams [[buffer(2)]])
{
  StrokeVertexIn in = vertices[vid];
  StrokeParams stroke = strokeParams[0];
  StrokeVertexOut out;
  
  out.pos = in.pos;
  out.pos.xy += in.norm * in.dir * stroke.width * sineFactor(sineParams[0], in.length, stroke.time);
  // Use reciprocal-length so that the tip of the stroke is white, and the tail is black.
  float brightness = in.length * stroke.reciprocalLength;
  // Square the brightness so that it is approximately linear, perceptually.
  out.uv.x = brightness * brightness;
  
  return out;
}

// Fragment shader.
fragment half4 blackWhite_frag(StrokeVertexOut inFrag [[stage_in]])
{
  half4 out;
  out.rgb = half3(inFrag.uv.x);
  out.a = 1.0;
  return out;
}

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
  vertices[index].dir = 1.0;
  vertices[index].length = length;
  vertices[index + 1].pos.xy = p;
  vertices[index + 1].pos.zw = float2(0,1);
  vertices[index + 1].norm = normal;
  vertices[index + 1].dir = -1.0;
  vertices[index + 1].length = length;
}
