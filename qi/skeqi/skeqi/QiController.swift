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

// TODO: inherits from NSObject so we can alloc it from Objective-C implementations
// of QiStrokeFitter.  Kinda kludgy.
@objc class Stroke2 : NSObject {
    var vertexCount : Int
    var buffer: MTLBuffer!

    override init() {
        vertexCount = 0
    }

    func encodeDrawCalls(renderEncoder: MTLRenderCommandEncoder) {
        if (vertexCount > 0) {
            renderEncoder.pushDebugGroup("draw stroke")
            renderEncoder.setVertexBuffer(buffer, offset: 0, atIndex: 0)
            renderEncoder.setVertexBuffer(buffer, offset: 28, atIndex: 1)
            renderEncoder.drawPrimitives(.TriangleStrip, vertexStart: 0, vertexCount: vertexCount, instanceCount: 1)
            renderEncoder.popDebugGroup()
        }
    }

}


@objc protocol QiPage : QiDrawable {
    var vertexSize : Int { get }
    var metalLibrary : MTLLibrary? { get }
    
    func addStroke(stroke: Stroke2);
}

// TODO: inherits from NSObject so maybe we can use Id<Page> from Objective-C
// implementations of QiStrokeFitter.  Kinda kludgy.
@objc class Page : NSObject, QiPage {
    var strokes = [Stroke2]()
    let vertexSize : Int = 32

    var strokePipelineState : MTLRenderPipelineState?
    var metalLibrary : MTLLibrary?

    func encodeDrawCalls(renderEncoder: MTLRenderCommandEncoder) {
        for stroke in strokes { stroke.encodeDrawCalls(renderEncoder) }
    }

    func addStroke(stroke: Stroke2) {
        strokes.append(stroke)
    }

    func setUpPipeline(library: MTLLibrary?) {
        if (metalLibrary != nil) {
            if (metalLibrary !== library) {
                println("Attempting to initialize Page with a different MTLLibrary");
            }
            return;
        }
        metalLibrary = library;

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
    }
}

protocol QiTouchHandler {
    func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent);
    func touchesCancelled(touches: Set<UITouch>!, withEvent event: UIEvent!);
    func touchesMoved(touches: Set<UITouch>, withEvent event: UIEvent);
    func touchesEnded(touches: Set<UITouch>, withEvent event: UIEvent);
}

class StrokeTouchHandler : QiTouchHandler {
    var activeFitters = [UITouch : QiStrokeFitter]()
    var freeFitters = [QiStrokeFitter]()
    var page : Page

    init(page mypage: Page) {
        page = mypage
    }
    func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent) {
        for touch in touches {
            let fitter = obtainFitter()
            activeFitters.updateValue(fitter, forKey:touch)
            fitter.startStroke()
            fitter.addSamplePoint(getTouchPosition(touch))
        }
    }
    func touchesCancelled(touches: Set<UITouch>!, withEvent event: UIEvent!) {
        assert(false, "touchesCancelled(): not implemented")
    }
    func touchesMoved(touches: Set<UITouch>, withEvent event: UIEvent) {
        for touch in touches {
            if let fitter = activeFitters[touch] {
                fitter.addSamplePoint(getTouchPosition(touch))
            } else {
                assert(false, "touchesMoved(): could not find fitter");
            }
        }
    }
    func touchesEnded(touches: Set<UITouch>, withEvent event: UIEvent) {
        for touch in touches {
            if let fitter = activeFitters[touch] {
                fitter.addSamplePoint(getTouchPosition(touch))
                fitter.finishStroke()
                activeFitters[touch] = nil
                freeFitters.append(fitter)
            } else {
                assert(false, "touchesEnded(): could not find fitter");
            }
        }
    }
    func getTouchPosition(touch : UITouch) -> CGPoint {
        var pt = touch.locationInView(touch.view);
        pt.x = (pt.x / touch.view.bounds.width * 2.0) - 1.0;
        pt.y = (pt.y / touch.view.bounds.height * -2.0) + 1.0;
        return pt;
    }

    func obtainFitter() -> QiStrokeFitter {
        if freeFitters.count > 0 {
            return freeFitters.removeLast()
        } else {
            return BezierFitter(page: page)
        }
    }
}


class QiController: GameViewController {

    var stroke : RenderableStroke?
    var strokePipelineState : MTLRenderPipelineState?

    let page = Page()
    var touchHandler : QiTouchHandler

    required init(coder aDecoder: NSCoder) {
        touchHandler = StrokeTouchHandler(page: page)
        super.init(coder: aDecoder)

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

    override func setUpPipeline(library: MTLLibrary?) {
        super.setUpPipeline(library)
        page.setUpPipeline(library)
    }

    override func encodeDrawCalls(renderEncoder: MTLRenderCommandEncoder) {
        super.encodeDrawCalls(renderEncoder)

        if (true) {
            renderEncoder.pushDebugGroup("draw all strokes")
            renderEncoder.setRenderPipelineState(page.strokePipelineState!)
            stroke!.encodeDrawCalls(renderEncoder)
            renderEncoder.popDebugGroup()
        }

        page.encodeDrawCalls(renderEncoder)
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

    func transformTouchSet(touches: Set<NSObject>) -> Set<UITouch> {
        var set = Set<UITouch>()
        for touch in touches { set.insert(touch as! UITouch) }
        return set
    }

    override func touchesBegan(touches: Set<NSObject>, withEvent event: UIEvent) {
        touchHandler.touchesBegan(transformTouchSet(touches), withEvent: event)
    }

    override func touchesCancelled(touches: Set<NSObject>!, withEvent event: UIEvent!) {
        touchHandler.touchesCancelled(transformTouchSet(touches), withEvent: event)
    }

    override func touchesMoved(touches: Set<NSObject>, withEvent event: UIEvent) {
        touchHandler.touchesMoved(transformTouchSet(touches), withEvent: event)
    }

    override func touchesEnded(touches: Set<NSObject>, withEvent event: UIEvent) {
        touchHandler.touchesEnded(transformTouchSet(touches), withEvent: event)
    }
}
