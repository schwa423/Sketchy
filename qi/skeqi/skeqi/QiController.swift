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
    var buffer: MTLBuffer?

    override init(_ arcs : ArcList) {
        vertexCount = 300
        super.init(arcs)
    }

    private func getBuffer(device: MTLDevice) -> MTLBuffer {
      if (buffer != nil) { return buffer! }
      let buf = device.newBufferWithLength(vertexCount * vertexSize,
                                           options: .CPUCacheModeDefaultCache);
      buf.label = "stroke verts"
      self.tesselate_(UnsafeMutablePointer<UInt8>(buf.contents()), totalVertexCount: vertexCount, lengthOffset: 0.0)
      buffer = buf
      return buf
    }

    func encodeDrawCalls(renderEncoder: MTLRenderCommandEncoder) {
        let buf = getBuffer(renderEncoder.device)
        renderEncoder.pushDebugGroup("draw stroke")
        renderEncoder.setVertexBuffer(buf, offset: 0, atIndex: 0)
        renderEncoder.setVertexBuffer(buf, offset: 28, atIndex: 1)
        renderEncoder.drawPrimitives(.TriangleStrip, vertexStart: 0, vertexCount: vertexCount, instanceCount: 1)
        renderEncoder.popDebugGroup()
    }
}

class QiController: GameViewController {
  var delegate : QiControllerDelegate
  var stroke : RenderableStroke?

  required init?(coder aDecoder: NSCoder) {
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
    if let functionNames = library?.functionNames {
      for name in functionNames {
        print("found shader function: \(name)")
      }
    } else {
      print("no library provided")
    }
  }

  override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
    delegate.touchesBegan(touches, withEvent: event)
  }

  override func touchesCancelled(touches: Set<UITouch>?, withEvent event: UIEvent?) {
    delegate.touchesCancelled(touches, withEvent: event)
  }

  override func touchesMoved(touches: Set<UITouch>, withEvent event: UIEvent?) {
    delegate.touchesMoved(touches, withEvent: event)
  }

  override func touchesEnded(touches: Set<UITouch>, withEvent event: UIEvent?) {
    delegate.touchesEnded(touches, withEvent: event)
  }
}
