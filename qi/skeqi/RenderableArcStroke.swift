//
//  RenderableArcStroke.swift
//  skeqi
//
//  Created by Josh Gargus on 4/24/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Metal

// TODO: port any desired Arc-processing stuff to qi::pen
class RenderableArcStroke: ArcStroke {
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

func createExampleRenderableArcStroke() -> RenderableArcStroke {
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

  return RenderableArcStroke(ArcList(startPoint: startPoint,
                                     startRadians: startRadians,
                                     radiusList: radiusList,
                                     radiansChangeList: radiansChangeList))
}