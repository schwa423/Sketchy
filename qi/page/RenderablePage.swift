//
//  RenderablePage.swift
//  skeqi
//
//  Created by Josh Gargus on 4/24/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Metal
import simd

// TODO: document
class RenderablePage : Page {
  let device: MTLDevice
  let library: MTLLibrary
  let renderPipeline: MTLRenderPipelineState
  let computePipeline: MTLComputePipelineState
  let enableAlphaBlending = false
  private let sineParams : SineParams
  
  // TODO: fix this hack
  var time: Float = 0.0
  
  init?(device: MTLDevice, library: MTLLibrary) {
    self.device = device
    self.library = library
    
    // Set up render pipeline.
//    let vertexFunction = library.newFunctionWithName("blackWhite_vert")!
//    let fragmentFunction = library.newFunctionWithName("blackWhite_frag")!
//    self.sineParams = SineParams(amplitude: 0.4, period: 50.0, speed: 0.125)
    let vertexFunction = library.newFunctionWithName("fractalTiling_vert")!
    let fragmentFunction = library.newFunctionWithName("fractalTiling_frag")!
    self.sineParams = SineParams(amplitude: 0.2, period: 50.0, speed: 0.0625)
    let pipelineDescriptor = MTLRenderPipelineDescriptor()
    pipelineDescriptor.vertexFunction = vertexFunction
    pipelineDescriptor.fragmentFunction = fragmentFunction
    let renderbufferAttachment = pipelineDescriptor.colorAttachments[0]
    renderbufferAttachment.pixelFormat = .BGRA8Unorm
    if enableAlphaBlending {
      renderbufferAttachment.blendingEnabled = true
      renderbufferAttachment.rgbBlendOperation = .Add
      renderbufferAttachment.alphaBlendOperation = .Add
      renderbufferAttachment.sourceRGBBlendFactor = .SourceAlpha
      renderbufferAttachment.sourceAlphaBlendFactor = .SourceAlpha
      renderbufferAttachment.destinationRGBBlendFactor = .OneMinusSourceAlpha;
      renderbufferAttachment.destinationAlphaBlendFactor = .OneMinusSourceAlpha;
    }
    
    try! renderPipeline = device.newRenderPipelineStateWithDescriptor(pipelineDescriptor)
    
    // Set up compute pipeline.
    let kernelFunction = library.newFunctionWithName("strokeBezierTesselate")!
    try! computePipeline = device.newComputePipelineStateWithFunction(kernelFunction)
  }
  
  override func instantiateStroke() -> Stroke {
    return RenderableStroke(index: nextStrokeIndex())
  }
  
  override func finalizeStroke(stroke: Stroke) {
    // TODO: perhaps put all strokes into one shared buffer.
    super.finalizeStroke(stroke)
  }
  
  func update(commandQueue: MTLCommandQueue) {
    time += 0.016 // 16 milliseconds
    
    if !dirtyStrokes.isEmpty {
      let commandBuffer = commandQueue.commandBuffer()
      let commandEncoder = commandBuffer.computeCommandEncoder()
      commandEncoder.pushDebugGroup("Tesselate Bezier Strokes")
      commandEncoder.setComputePipelineState(computePipeline)
      
      for stroke in dirtyStrokes {
        tesselateStroke(stroke as! RenderableStroke, encoder: commandEncoder)
      }
      dirtyStrokes.removeAll(keepCapacity: true)
      
      commandEncoder.popDebugGroup()
      commandEncoder.endEncoding()
      commandBuffer.commit()
    }
  }
  
  func draw(renderEncoder: MTLRenderCommandEncoder) {
    renderEncoder.pushDebugGroup("Render Page Strokes")
    renderEncoder.setRenderPipelineState(renderPipeline)
    
    // SineParams are the same for all strokes, so set them once here.
    var sineParams = self.sineParams;
    renderEncoder.setVertexBytes(&sineParams, length: sizeof(SineParams), atIndex: 2);
    
    for stroke in strokes {
      (stroke as! RenderableStroke).draw(renderEncoder, time: time)
    }
    renderEncoder.popDebugGroup()
  }
  
  private func tesselateStroke(stroke: RenderableStroke, encoder: MTLComputeCommandEncoder) {
    if stroke.path.isEmpty { return }
    
    let vertexCounts = computeStrokeVertexCounts(stroke)
    var totalVertexCount = 0
    for count in vertexCounts {
      assert(count == (count / 2) * 2)
      totalVertexCount += count
    }
    
    assert(32 == sizeof(StrokeVertex))
    stroke.buffer = getBufferOfLength(totalVertexCount * sizeof(StrokeVertex),
                                      existing: stroke.buffer)
    stroke.vertexCount = totalVertexCount
    
    // Tesselate stroke on GPU using compute shader.
    assert(52 == sizeof(StrokeSegment))
    var offset = 0
    var startAndTotalLength = float2(0.0, stroke.length)
    for i in 0..<stroke.path.count {
      withUnsafePointer(&stroke.path[i]) {
        // TODO: not sure why + 4 is necessary.  Does this need to be a multiple of 8 bytes?
        encoder.setBytes($0, length: sizeof(StrokeSegment) + 4, atIndex:0)
      }
      
      encoder.setBytes(&startAndTotalLength, length: sizeof(float2), atIndex:1)
      startAndTotalLength.x += stroke.path[i].length
      
      var tDivisor : Float = Float(vertexCounts[i]) / 2.0 - 1.0
      encoder.setBytes(&tDivisor, length: sizeof(Float), atIndex:2)
      
      encoder.setBuffer(stroke.buffer, offset: offset, atIndex:3)
      
      let threadgroupSize = MTLSizeMake(vertexCounts[i] / 2, 1, 1)
      let threadgroups = MTLSizeMake(1, 1, 1)
      encoder.dispatchThreadgroups(threadgroups, threadsPerThreadgroup: threadgroupSize)
      
      offset += vertexCounts[i] * sizeof(StrokeVertex)
    }
  }
  
  private func computeStrokeVertexCounts(stroke: RenderableStroke) -> [Int] {
    return stroke.path.map {
      (seg: StrokeSegment) -> Int in
        return max(2, 2 * Int(seg.length * 160))
    }
  }
  
  private func getBufferOfLength(length: Int,
                                 existing: MTLBuffer?=nil,
                                 minLength: Int=0,
                                 options: MTLResourceOptions = .CPUCacheModeDefaultCache) -> MTLBuffer {
    if let existing = existing {
      let minLength = (minLength <= 0) ? length : minLength
      if existing.length >= minLength { return existing }
    }
    return device.newBufferWithLength(length, options: options)
  }
}

// Not used directly.  Matches the vertices that are created by RenderablePage.tesselateStroke().
private struct StrokeVertex {
  var px, py, pz, pw, nx, ny, dir, length : Float
}

// Corresponds to the StrokeParams struct consumed by the vertex shaders.
private struct StrokeParams {
  // Domain of amplitude is [0, 0.5] because range of sin+1 is [0,2].
  let length, reciprocalLength, width, reciprocalWidth, time : Float
  init(length : Float, width : Float, time : Float) {
    self.length = length
    self.reciprocalLength = 1.0 / length
    self.width = width
    self.reciprocalWidth = 1.0 / width
    self.time = time
  }
}

// Corresponds to the SineParams struct consumed by the vertex shaders.
private struct SineParams {
  // Domain of amplitude is [0, 0.5] because range of sin+1 is [0,2].
  let amplitude, period, speed : Float
  init(amplitude : Float, period : Float, speed : Float) {
    self.amplitude = amplitude
    self.period = period  // TODO: this is really "frequency", not "period"
    self.speed = speed
  }
}

private class RenderableStroke : Stroke {
  var vertexCount: Int = 0
  var offset: Int = 0
  var buffer: MTLBuffer?
  
  override init(index: Int) {
    super.init(index: index)
  }
  
  func draw(encoder: MTLRenderCommandEncoder, time: Float) {
    if (vertexCount > 0) {
      encoder.setVertexBuffer(buffer, offset: offset, atIndex: 0)
      
      var strokeParams = StrokeParams(length: length, width: 0.05, time: time)
      encoder.setVertexBytes(&strokeParams, length: sizeof(StrokeParams), atIndex: 1);
      
      // Time not set here, it is the same for all strokes so it is set in RenderablePage.draw().
      
      encoder.drawPrimitives(.TriangleStrip, vertexStart: 0, vertexCount: vertexCount)
    }
  }
  
  override var description: String {
    var string = String("RenderableStroke{\n  vertexCount: \(vertexCount)\n  path: {")
    for bez in path {
      string += "\n    \(bez.description)"
    }
    string += path.isEmpty ? "}\n}" : "\n  }\n}"
    return string
  }
}
