import UIKit
import Metal
import MetalKit




class PageViewController: UIViewController, MTKViewDelegate {

  let device: MTLDevice
  let library: MTLLibrary
  let commandQueue: MTLCommandQueue

  let page: Page2
  var strokeFitters = [UITouch: StrokeFitter]()
  var mtkView: MTKView { get { return super.view as! MTKView } }

  required init?(coder aDecoder: NSCoder) {
    device = MTLCreateSystemDefaultDevice()!
    library = device.newDefaultLibrary()!
    commandQueue = device.newCommandQueue()
    page = Page2(device: device, library: library)!
    super.init(coder: aDecoder)
  }


  func mtkView(view: MTKView, drawableSizeWillChange size: CGSize) {
    print("mtkView drawableSizeWillChange")
  }

  func drawInMTKView(view: MTKView) {
    guard let drawable = view.currentDrawable,
          let descriptor = view.currentRenderPassDescriptor
    else { return; }

    page.update(commandQueue)

    let commandBuffer = commandQueue.commandBuffer()
    commandBuffer.label = "PageViewController frame command buffer"

    let encoder = commandBuffer.renderCommandEncoderWithDescriptor(descriptor)
    encoder.label = "Page render encoder"

    page.draw(encoder)

    encoder.endEncoding()
    commandBuffer.presentDrawable(drawable)
    commandBuffer.commit()
  }

  override func viewDidLoad() {
    super.viewDidLoad()
    mtkView.device = device
    mtkView.delegate = self
    mtkView.clearColor = MTLClearColor(red: 0.2, green: 0.6, blue: 0.6, alpha: 1.0)
  }

  override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
    for touch in touches {
      assert(strokeFitters[touch] === nil)
      let fitter = StrokeFitter(page: page)
      strokeFitters[touch] = fitter
      fitter.startStroke(touch)
    }
  }

  override func touchesCancelled(touches: Set<UITouch>?, withEvent event: UIEvent?) {
    assert(false)  // not implemented
  }

  override func touchesMoved(touches: Set<UITouch>, withEvent event: UIEvent?) {
    for touch in touches {
      let fitter = strokeFitters[touch]!
      let actual = event?.coalescedTouchesForTouch(touch) ?? [touch]
      let predicted = event?.predictedTouchesForTouch(touch) ?? []
      fitter.continueStroke(actual, predicted: predicted)
    }
  }

  override func touchesEnded(touches: Set<UITouch>, withEvent event: UIEvent?) {
    for touch in touches {
      let fitter = strokeFitters[touch]!
      fitter.continueStroke([touch])
      fitter.finishStroke(touch)
      strokeFitters.removeValueForKey(touch)
    }
  }
}

class Page2 {
  let device: MTLDevice
  let library: MTLLibrary
  let renderPipeline: MTLRenderPipelineState
  let computePipeline: MTLComputePipelineState

  var strokes = [Stroke2]()
  var dirtyStrokes = [Stroke2]()
  var nextStrokeIndex = 0

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
      encoder.setBuffer(stroke.buffer, offset: offset, atIndex: 1)
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


extension UITouch {
  func normalizedLocationInView(view: UIView?) -> CGPoint {
    if let bounds = view?.bounds ?? window?.bounds {
      var pt = locationInView(view)
      pt.x = pt.x / CGRectGetWidth(bounds) * 2.0 - 1.0
      pt.y = pt.y / CGRectGetHeight(bounds) * -2.0 + 1.0
      return pt
    }
    assert(false)
    return CGPoint(x: 0.0, y: 0.0)
  }

  var location: CGPoint {
    get { return locationInView(self.view) }
  }

  var normalizedLocation: CGPoint {
    get { return normalizedLocationInView(self.view) }
  }
}

extension float2 {
  init(_ pt: CGPoint) {
    self.init(x: Float(pt.x), y: Float(pt.y))
  }
}

func ==(left: float2, right: float2) -> Bool {
  return left.x == right.x && left.y == right.y
}

extension Array {
  var first: Element { return self[0] }
  var last: Element { return self[endIndex - 1] }
}

extension ArraySlice {
  var first: Element { return self[startIndex] }
  var last: Element { return self[endIndex - 1] }
}

class StrokeFitter {
  let page: Page2
  let stroke: Stroke2
  var points = [float2]()
  var params = [Float]()
  var path = [Bezier3]()
  let errorThreshold = Float(0.0002)


  init(page: Page2) {
    self.page = page
    self.stroke = page.newStroke()
  }

  func startStroke(touch: UITouch) {
    points.append(float2(touch.normalizedLocation))
    params.append(0.0)
  }

  func continueStroke(touches: [UITouch], predicted: [UITouch] = []) {
    for touch in touches {
      let pt = float2(touch.normalizedLocation)
      let dist = distance(pt, points.last)
      points.append(pt)
      params.append(params.last + dist)
    }

    // Recursively compute a list of cubic Bezier segments.
    // TODO: don't recompute stable path segments near the beginning of the stroke.
    let leftTangent = points[1] - points[0];
    let rightTangent = points[points.count - 2] - points[points.count - 1];

    path.removeAll(keepCapacity: true)
    fitSampleRange(0..<points.count, leftTangent: leftTangent, rightTangent: rightTangent);

    page.setStrokePath(stroke, path: path)
  }

  func finishStroke(touch: UITouch) {
    page.finalizeStroke(stroke)
    print("Finished stroke with \(path.count) cubic beziers")
    print(stroke.description)
  }

  // TODO: investigate reparameterization.
  func fitSampleRange(range: Range<Int>, leftTangent: float2, rightTangent: float2) {
    assert(range.count > 1)

    let startIndex = range.startIndex
    let endIndex = range.endIndex

    if range.count == 2 {
      // Only two points... use a heuristic.
      // TODO: Double-check this heuristic (perhaps normalization needed?).
      // TODO: Perhaps this segment can be omitted entirely (perhaps blending
      //       endpoints of the adjacent segments.
      var line = Bezier3()
      line.pt0 = points[startIndex]
      line.pt3 = points[endIndex-1]
      line.pt1 = line.pt0 + 0.25 * leftTangent
      line.pt2 = line.pt3 + 0.25 * rightTangent
      path.append(line)
      return
    }

    // Normalize cumulative length between 0.0 and 1.0.
    let paramShift = -params[startIndex];
    let paramScale = 1.0 / (params[endIndex-1] + paramShift);

    let bez = FitBezier3ToPoints(points[startIndex..<endIndex],
                                 params: params[startIndex..<endIndex],
                                 paramShift: paramShift,
                                 paramScale: paramScale,
                                 startTangent: leftTangent,
                                 endTangent: rightTangent)

    // TODO: does Swift rounding/trunctation work like C?
    var splitIndex = (startIndex + endIndex) / 2
    var maxError = Float(0.0)
    for i in startIndex..<endIndex {
      let t = (params[i] + paramShift) * paramScale
      let diff = points[i] - bez.Evaluate(t)
      let error = dot(diff, diff)
      if (error > maxError) {
        maxError = error;
        splitIndex = i;
      }
    }

    // The current fit is good enough... add it to the path and stop recursion.
    if (maxError < errorThreshold) {
      path.append(bez);
      return;
    }

    // Error is too large... split into two ranges and fit each.
    assert(splitIndex > startIndex && splitIndex < endIndex-1);
    let middleTangent = points[splitIndex + 1] - points[splitIndex - 1];
    fitSampleRange(startIndex...splitIndex, leftTangent: leftTangent, rightTangent: middleTangent * -1.0);
    fitSampleRange(splitIndex..<endIndex, leftTangent: middleTangent, rightTangent: rightTangent);
  }
}
