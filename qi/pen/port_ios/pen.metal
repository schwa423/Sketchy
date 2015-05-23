#include <metal_stdlib>

using namespace metal;

struct VertexInOut
{
    float4  position [[position]];
    float4  color;
};

struct StrokeVertexIn
{
    float4 position [[ attribute(0) ]];
    float4 color    [[ attribute(1) ]];
};

/*
struct StrokeVertex {
    float4 pos;
    float2 norm;
    float
}
*/

vertex VertexInOut strokeVertexPassThrough(StrokeVertexIn vert [[ stage_in ]])
{
    VertexInOut outVertex;

    outVertex.position = vert.position;
    outVertex.color    = vert.color; // float4(0.3, 0.3, 0.3, 1);

    return outVertex;
};

fragment half4 strokeFragmentPassThrough(VertexInOut inFrag [[stage_in]])
{
    return half4(inFrag.color);
};
