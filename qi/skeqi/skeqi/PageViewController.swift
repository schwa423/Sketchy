import UIKit
import Metal
import MetalKit

class PageViewController: UIViewController, MTKViewDelegate, GIDSignInUIDelegate {

  let device: MTLDevice
  let library: MTLLibrary
  let commandQueue: MTLCommandQueue

  // TODO: this should be a global Firebase obtained from the AppDelegate.
  let firebase: SkeqiFirebase
  var page: Page2
  var strokeFitters = [UITouch: StrokeFitter]()
  var mtkView: MTKView { get { return super.view as! MTKView } }

  required init?(coder aDecoder: NSCoder) {
    device = MTLCreateSystemDefaultDevice()!
    library = device.newDefaultLibrary()!
    commandQueue = device.newCommandQueue()
    firebase = SkeqiFirebase(url: "https://blistering-inferno-9169.firebaseio.com/")
    page = Page2(device: device, library: library)!

    super.init(coder: aDecoder)

    firebase.signIn(delegate: self).then { data in
      print("ABOUT TO REQUEST DATA FOR PAGE!!!!!")
    }
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

    page = Page2(device: device, library: library)!

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
