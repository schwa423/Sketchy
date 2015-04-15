//
//  QiController
//  skeqi
//
//  Created by Josh Gargus on 4/10/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

import UIKit
import Metal

class RenderableStroke: Stroke {
    var vertexCount : Int
    var buffer: MTLBuffer! = nil
    
    override init(_ arcs : ArcList) {
        vertexCount = 300
        super.init(arcs)
    }
    func encodeDrawCalls(renderEncoder: MTLRenderCommandEncoder) {
        if (buffer == nil) {
            // buffer = renderEncoder(vertexCount * vertexSize, options: MTLResourceOptions.OptionCPUCacheModeWriteCombined)
            buffer = renderEncoder.device.newBufferWithLength(vertexCount * vertexSize, options: nil)
            buffer.label = "stroke verts"
            self.tesselate_(UnsafeMutablePointer<UInt8>(buffer.contents()), totalVertexCount: vertexCount, lengthOffset: 0.0)
        }

        renderEncoder.pushDebugGroup("draw stroke")
        renderEncoder.setVertexBuffer(buffer, offset: 0, atIndex: 0)
        renderEncoder.setVertexBuffer(buffer, offset: 28, atIndex: 1)
        renderEncoder.drawPrimitives(.TriangleStrip, vertexStart: 0, vertexCount: vertexCount, instanceCount: 1)
        renderEncoder.popDebugGroup()
    }
}

class QiController: GameViewController {

    var stroke : RenderableStroke?
    var strokePipelineState : MTLRenderPipelineState?
    
    override func setUpPipeline(library: MTLLibrary?) {
        super.setUpPipeline(library)
        
        let fragmentProgram = library?.newFunctionWithName("passThroughFragment")
        let vertexProgram = library?.newFunctionWithName("passThroughVertex2")
        
        let vertexDescriptor = MTLVertexDescriptor()
        vertexDescriptor.attributes[0].format = MTLVertexFormat.Float4
        vertexDescriptor.attributes[0].bufferIndex = 0;
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[1].format = MTLVertexFormat.UChar4
        vertexDescriptor.attributes[1].bufferIndex = 0;
        vertexDescriptor.attributes[1].offset = 0;
        vertexDescriptor.layouts[0].stride = 32;
        vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunction.PerVertex;
//        vertexDescriptor.layouts[1].stride = 32;
//        vertexDescriptor.layouts[1].stepFunction = MTLVertexStepFunction.PerVertex;
        
        let pipelineStateDescriptor = MTLRenderPipelineDescriptor()
        pipelineStateDescriptor.vertexFunction = vertexProgram
        pipelineStateDescriptor.fragmentFunction = fragmentProgram
        pipelineStateDescriptor.vertexDescriptor = vertexDescriptor
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = .BGRA8Unorm  // TODO: needed if set elsewhere earlier?
        
        var pipelineError : NSError?
        strokePipelineState = library?.device.newRenderPipelineStateWithDescriptor(pipelineStateDescriptor, error: &pipelineError)
        if (strokePipelineState == nil) {
            println("Failed to create pipeline state, error \(pipelineError)")
        }

        let π = Float(M_PI)
//        let startRadians : Float = π*7/6
        let startPoint = Point2d(0.5, 0.0);
//        let radiusList : [Float] = [0.3, 0.18, 0.06, 0.09, 0.12, 0.18, 0.24]
//        let radiansChangeList : [Float] = [π/2, π/2, -π/4, -π/4, -π/4, -π/4, -π*3/2]
        let startRadians : Float = 0.0
//        let startPoint = Point2d(0.5, 0.0);
        let radiusList : [Float] = [0.5, 0.4, 0.32, 0.256, 0.2, 0.164, 0.131, 0.105, 0.084, 0.067]
        let radiansChangeList : [Float] = [π/2, π/2, π/2, π/2, π/2, π/2, π/2, π/2, π/2, π/2]
//        let radiusList : [Float] = [1.0, 1.0,]
//        let radiansChangeList : [Float] = [π/2, π/2]
//        let radiusList : [Float] = [0.5]
//        let radiansChangeList : [Float] = [π/2]

        stroke = RenderableStroke(ArcList(startPoint: startPoint, startRadians: startRadians, radiusList: radiusList, radiansChangeList: radiansChangeList))
    }
    
    override func encodeDrawCalls(renderEncoder: MTLRenderCommandEncoder) {
        super.encodeDrawCalls(renderEncoder)
        
        if (true) {
            renderEncoder.pushDebugGroup("draw all strokes")
            renderEncoder.setRenderPipelineState(strokePipelineState!)
            stroke!.encodeDrawCalls(renderEncoder);
            renderEncoder.popDebugGroup()
        }
    }
    
    func printShaderFunctions(library: MTLLibrary?) {
        if let functionNames = library?.functionNames as? [NSString] {
            for name in functionNames {
                println("found shader function: \(name)")
            }
        } else {
            println("no library provided")
        }
    }
    
    override func touchesBegan(touches: Set<NSObject>, withEvent event: UIEvent) {
        
    }
    
    override func touchesCancelled(touches: Set<NSObject>!, withEvent event: UIEvent!) {
        
    }
    
    override func touchesMoved(touches: Set<NSObject>, withEvent event: UIEvent) {
        for touch in touches {
            let touch1 = touch as! UITouch
            println("touch in qi");
        }
    }
    
    override func touchesEnded(touches: Set<NSObject>, withEvent event: UIEvent) {
        
    }
}