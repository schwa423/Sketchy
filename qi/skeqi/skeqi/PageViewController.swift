import UIKit
import Metal
import MetalKit

import FirebaseDatabase
import PromiseKit

class PageViewController: UIViewController, MTKViewDelegate, GIDSignInUIDelegate {

  let device: MTLDevice = MTLCreateSystemDefaultDevice()!
  let library: MTLLibrary
  let commandQueue: MTLCommandQueue
  var mtkView: MTKView { get { return super.view as! MTKView } }
  
  var page: RenderablePage? = nil
  var strokeFitters = [UITouch: UITouchStrokeFitter]()
  var incomingStrokes = [[StrokeSegment]]()

  let firebaseProvider = UIApplication.sharedApplication().delegate as! FirebaseRefProvider
  var pageSyncer: FirebasePageSyncer? = nil
  
  var pageScale : float2 {
    get {
      if let bounds = view?.bounds {
        let width = Float(CGRectGetWidth(bounds))
        let height = Float(CGRectGetHeight(bounds))
        // Scale so that rendering isn't stretched to match the screen aspect-ratio.
        return width > height ? float2(height / width, 1.0) : float2(1.0, width / height)
      }
      assert(false)
      return float2(1.0, 1.0)
    }
  }

  // TODO: move ref into FirebasePageSyncer?
  var ref : FIRDatabaseReference? = nil
  
  required init?(coder aDecoder: NSCoder) {
    library = device.newDefaultLibrary()!
    commandQueue = device.newCommandQueue()
    super.init(coder: aDecoder)
  }

  func mtkView(view: MTKView, drawableSizeWillChange size: CGSize) {
    print("mtkView drawableSizeWillChange")
  }

  func drawInMTKView(view: MTKView) {
    guard let drawable = view.currentDrawable,
          let descriptor = view.currentRenderPassDescriptor,
          let page = self.page
    else { return }
    
    mtkView.clearColor = page.clearColor
    page.update(commandQueue)

    let commandBuffer = commandQueue.commandBuffer()
    commandBuffer.label = "PageViewController frame command buffer"

    let encoder = commandBuffer.renderCommandEncoderWithDescriptor(descriptor)
    encoder.label = "Page render encoder"

    page.draw(encoder, pageScale: pageScale)

    encoder.endEncoding()
    commandBuffer.presentDrawable(drawable)
    commandBuffer.commit()
  }

  override func viewDidLoad() {
    super.viewDidLoad()

    mtkView.device = device
    mtkView.delegate = self
  }
  
  override func viewWillAppear(animated: Bool) {
    super.viewWillAppear(animated)
    
    ref = firebaseProvider.getFirebaseRef().child("/pages/\(firebaseProvider.pageId)/strokes")
    
    page = RenderablePage(device: device, library: library)
    pageSyncer = FirebasePageSyncer(ref!, page:page!)
    page!.addObserver(pageSyncer)
  }
  
  override func viewDidDisappear(animated: Bool) {
    super.viewDidDisappear(animated)
    pageSyncer!.destroy()
    pageSyncer = nil
    ref = nil
    page = nil
  }

  override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
    for touch in touches {
      assert(strokeFitters[touch] === nil)
      let fitter = UITouchStrokeFitter(page: page!, view:view!)
      strokeFitters[touch] = fitter
      fitter.startStroke(touch)
    }
  }

  override func touchesCancelled(touches: Set<UITouch>?, withEvent event: UIEvent?) {
    assert(false)  // not implemented
  }

  override func touchesMoved(touches: Set<UITouch>, withEvent event: UIEvent?) {
    for touch in touches {
      strokeFitters[touch]!.continueStroke(touch, withEvent:event!)
    }
  }

  override func touchesEnded(touches: Set<UITouch>, withEvent event: UIEvent?) {
    for touch in touches {
      strokeFitters[touch]!.finishStroke(touch, withEvent: event!)
      strokeFitters.removeValueForKey(touch)
    }
  }
  
  // MARK: Actions
  
  @IBAction func googleSignOut(sender: AnyObject) {
    firebaseProvider.googleSignIn.signOut()
    do {
      try FIRAuth.auth()!.signOut()
    } catch {
      // TODO: handle errors
    }
  }
  
  @IBAction func clearPage(sender: AnyObject) {
    // TODO: if others are collaboratively editing this same page, it won't be cleared for them.
    ref!.removeValue()
    incomingStrokes.removeAll()
    page!.clear()
  }
  
  @IBAction func changeStyle(sender: AnyObject) {
    page!.changeStyle()
  }
}
