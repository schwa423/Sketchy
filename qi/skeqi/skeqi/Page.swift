import Metal
import MetalKit

class PageObserver : Equatable {
  typealias ObserverFunc = (Stroke2!)->Void
  let onFinalize: ObserverFunc
  required init(onFinalize _onFinalize: ObserverFunc!) {
    onFinalize = _onFinalize
  }
}

func ==(lhs: PageObserver, rhs: PageObserver) -> Bool {
  return lhs === rhs
}

class Page2 {
  let device: MTLDevice
  let library: MTLLibrary
  let renderPipeline: MTLRenderPipelineState
  let computePipeline: MTLComputePipelineState

  var strokes = [Stroke2]()
  var dirtyStrokes = [Stroke2]()
  var nextStrokeIndex = 0

  private var observers = [PageObserver]()

  init?(device: MTLDevice, library: MTLLibrary) {
    self.device = device
    self.library = library

    // Set up render pipeline.
    let vertexFunction = library.newFunctionWithName("strokeVertex")!
    let fragmentFunction = library.newFunctionWithName("strokeFragmentPassThrough")!
    let pipelineDescriptor = MTLRenderPipelineDescriptor()
    pipelineDescriptor.vertexFunction = vertexFunction
    pipelineDescriptor.fragmentFunction = fragmentFunction
    pipelineDescriptor.colorAttachments[0].pixelFormat = .BGRA8Unorm;
    try! renderPipeline = device.newRenderPipelineStateWithDescriptor(pipelineDescriptor)

    // Set up compute pipeline.
    let kernelFunction = library.newFunctionWithName("strokeBezierTesselate")!
    try! computePipeline = device.newComputePipelineStateWithFunction(kernelFunction)
  }

  // Add an observer that is called whenever a Stroke is finalized.
  func addObserver(observer: PageObserver!) { observers.append(observer) }
  func removeObserver(observer: PageObserver!) {
    if let index = observers.indexOf(observer) {
      observers.removeAtIndex(index)
    }
  }

  func update(commandQueue: MTLCommandQueue) {
    if dirtyStrokes.isEmpty { return }

    let commandBuffer = commandQueue.commandBuffer()
    let commandEncoder = commandBuffer.computeCommandEncoder()
    commandEncoder.pushDebugGroup("Tesselate Bezier Strokes")
    commandEncoder.setComputePipelineState(computePipeline)

    for stroke in dirtyStrokes {
      tesselateStroke(stroke, encoder: commandEncoder)
    }
    dirtyStrokes.removeAll(keepCapacity: true)

    commandEncoder.popDebugGroup()
    commandEncoder.endEncoding()
    commandBuffer.commit()
  }

  func draw(renderEncoder: MTLRenderCommandEncoder) {
    renderEncoder.pushDebugGroup("Render Page Strokes")
    renderEncoder.setRenderPipelineState(renderPipeline)
    for stroke in strokes {
      stroke.draw(renderEncoder)
    }
    renderEncoder.popDebugGroup()
  }

  func newStroke() -> Stroke2 {
    strokes.append(Stroke2(index: nextStrokeIndex++))
    return strokes.last!
  }

  func setStrokePath(stroke: Stroke2, path: [Bezier3]) {
    assert(strokes[stroke.index] === stroke)
    stroke.path = path
    dirtyStrokes.append(stroke)
  }

  func finalizeStroke(stroke: Stroke2) {
    // TODO: perhaps put all strokes into one shared buffer.

    for observer in observers {
      observer.onFinalize(stroke)
    }
  }

  private func tesselateStroke(stroke: Stroke2, encoder: MTLComputeCommandEncoder) {
    if stroke.path.isEmpty { return }

    let vertexCounts = computeStrokeVertexCounts(stroke)
    var totalVertexCount = 0
    for count in vertexCounts {
      assert(count == (count / 2) * 2)
      totalVertexCount += count
    }

    assert(32 == sizeof(Vertex))
    stroke.buffer = getBufferOfLength(totalVertexCount * sizeof(Vertex), existing: stroke.buffer)
    stroke.vertexCount = totalVertexCount

    // Tesselate stroke on GPU using compute shader.
    assert(32 == sizeof(Bezier3))
    var offset = 0
    for i in 0..<stroke.path.count {
      withUnsafePointer(&stroke.path[i]) {
        encoder.setBytes($0, length: sizeof(Bezier3), atIndex:0)
      }
      var tDivisor : Float = Float(vertexCounts[i]) / 2.0 - 1.0
      encoder.setBytes(&tDivisor, length: sizeof(Float), atIndex:1)
      encoder.setBuffer(stroke.buffer, offset: offset, atIndex:2)
      let threadgroupSize = MTLSizeMake(vertexCounts[i] / 2, 1, 1)
      let threadgroups = MTLSizeMake(1, 1, 1)
      encoder.dispatchThreadgroups(threadgroups, threadsPerThreadgroup: threadgroupSize)

      offset += vertexCounts[i] * sizeof(Vertex)
    }
  }

  private func computeStrokeVertexCounts(stroke: Stroke2) -> [Int] {
    return stroke.path.map {
      (bezier: Bezier3) -> Int in
      // TODO: Compute a number based on the length of each path segment.
      128
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


struct Vertex {
  var px, py, pz, pw, nx, ny, length : Float
  var cr, cg, cb, ca : UInt8
}

class Stroke2 : CustomStringConvertible {
  let index: Int
  var path = [Bezier3]()
  var vertexCount: Int = 0
  var offset: Int = 0
  var buffer: MTLBuffer?

  var fromFirebase: Bool = false

  init(index: Int) {
    self.index = index
  }

  func draw(encoder: MTLRenderCommandEncoder) {
    if (vertexCount > 0) {
      encoder.setVertexBuffer(buffer, offset: offset, atIndex: 0)
      encoder.drawPrimitives(.TriangleStrip, vertexStart: 0, vertexCount: vertexCount)
    }
  }

  var description: String {
    var string = String("Stroke{\n  vertexCount: \(vertexCount)\n  path: {")
    for bez in path {
      string += "\n    \(bez.description)"
    }
    string += path.isEmpty ? "}\n}" : "\n  }\n}"
    return string
  }
}
