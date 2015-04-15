//
//  Shaders.metal
//  MetalSwift
//
//  Created by Seth Sowerby on 8/14/14.
//  Copyright (c) 2014 Seth Sowerby. All rights reserved.
//

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
    uchar4 color    [[ attribute(1) ]];
};

vertex VertexInOut passThroughVertex(uint vid [[ vertex_id ]],
                                     constant packed_float4* position  [[ buffer(0) ]],
                                     constant packed_uchar4* color    [[ buffer(1) ]])
{
    VertexInOut outVertex;
    
    outVertex.position = position[vid];
    outVertex.color    = static_cast<float4>(uchar4(color[vid]));
    
    return outVertex;
};

vertex VertexInOut passThroughVertex2(StrokeVertexIn vert [[ stage_in ]])
{
    VertexInOut outVertex;
    
    outVertex.position = vert.position;
    outVertex.color    = float4(0.3, 0.3, 0.3, 1);
    
    return outVertex;
};

fragment half4 passThroughFragment(VertexInOut inFrag [[stage_in]])
{
    return half4(inFrag.color);
};