//
//  QiController
//  skeqi
//
//  Created by Josh Gargus on 4/10/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

import UIKit
import Metal

// TODO: port any desired Arc-processing stuff to qi::pen
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
  var delegate : QiControllerDelegate
  var stroke : RenderableStroke?

  required init(coder aDecoder: NSCoder) {
    delegate = Skeqi_iOS()

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

  override func viewDidLoad() {
    super.viewDidLoad()

    // TODO: clear queue when viewDidUnload().
    delegate.metalQueue = commandQueue;
  }

  override func update() {
    super.update();
    delegate.update();
  }

  override func encodeDrawCalls(renderEncoder: MTLRenderCommandEncoder) {
    super.encodeDrawCalls(renderEncoder)
    delegate.encodeDrawCalls(renderEncoder)
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
    delegate.touchesBegan(touches, withEvent: event)
  }

  override func touchesCancelled(touches: Set<NSObject>!, withEvent event: UIEvent!) {
    delegate.touchesCancelled(touches, withEvent: event)
  }

  override func touchesMoved(touches: Set<NSObject>, withEvent event: UIEvent) {
    delegate.touchesMoved(touches, withEvent: event)
  }

  override func touchesEnded(touches: Set<NSObject>, withEvent event: UIEvent) {
    delegate.touchesEnded(touches, withEvent: event)
  }
}
