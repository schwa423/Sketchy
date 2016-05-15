import UIKit
import Metal
import MetalKit

import Firebase
import PromiseKit

class FirebasePageObserver : PageObserver {
  let ref: Firebase

  init(_ firebase: Firebase) {
    self.ref = firebase
  }
  
  override func onFinalizeStroke(stroke: Stroke!) -> Void {
    // TODO: this prevents resending strokes that were already received from Firebase, but it 
    // doesn't prevent duplication of strokes that are created locally, then sent and received
    // from Firebase.
    if !stroke.fromFirebase {
      var path = [Float]()
      for seg in stroke.path {
        path.append(seg.curve.pt0[0])
        path.append(seg.curve.pt0[1])
        path.append(seg.curve.pt1[0])
        path.append(seg.curve.pt1[1])
        path.append(seg.curve.pt2[0])
        path.append(seg.curve.pt2[1])
        path.append(seg.curve.pt3[0])
        path.append(seg.curve.pt3[1])
        // TODO: worthwhile to also append the length and arc-length reparameterization?
        // Or is it better to simply recompute them, as now?
      }
      ref.childByAutoId().setValue(path)
    }
  }
}

class PageViewController: UIViewController, MTKViewDelegate, GIDSignInUIDelegate {

  let device: MTLDevice = MTLCreateSystemDefaultDevice()!
  let library: MTLLibrary
  let commandQueue: MTLCommandQueue
  var mtkView: MTKView { get { return super.view as! MTKView } }
  
  var page: RenderablePage? = nil
  var strokeFitters = [UITouch: StrokeFitter]()
  var incomingStrokes = [[StrokeSegment]]()

  let firebaseProvider = UIApplication.sharedApplication().delegate as! FirebaseRefProvider
  var query : UInt = 0
  var ref : Firebase? = nil
  
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

    for incoming in incomingStrokes {
      let stroke = page.newStroke()
      stroke.fromFirebase = true
      page.setStrokePath(stroke, path: incoming)
      page.finalizeStroke(stroke)
    }
    incomingStrokes.removeAll()
    
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
    mtkView.clearColor = MTLClearColor(red: 0.1, green: 0.0, blue: 0.1, alpha: 1.0)
  }
  
  override func viewWillAppear(animated: Bool) {
    super.viewWillAppear(animated)
    
    ref = firebaseProvider.getFirebaseRef().childByAppendingPath("/pages/\(firebaseProvider.pageId)/strokes")
    
    page = RenderablePage(device: device, library: library)
    page!.addObserver(FirebasePageObserver(ref!))
    
    query = ref!.observeEventType(.ChildAdded, withBlock: { (snapshot: FDataSnapshot!) -> Void in
      let array = snapshot.value as! NSArray
      assert(array.count % 8 == 0, "Array must be multiple of size of Bezier3")
      var path = [StrokeSegment]()
      path.reserveCapacity(array.count % 8)
      for (var i = 0; i < array.count; i += 8) {
        var bez = Bezier3()
        bez.pt0[0] = array.objectAtIndex(i) as! Float
        bez.pt0[1] = array.objectAtIndex(i+1) as! Float
        bez.pt1[0] = array.objectAtIndex(i+2) as! Float
        bez.pt1[1] = array.objectAtIndex(i+3) as! Float
        bez.pt2[0] = array.objectAtIndex(i+4) as! Float
        bez.pt2[1] = array.objectAtIndex(i+5) as! Float
        bez.pt3[0] = array.objectAtIndex(i+6) as! Float
        bez.pt3[1] = array.objectAtIndex(i+7) as! Float
        path.append(StrokeSegment(bez))
      }
      self.incomingStrokes.append(path)
    });
  }
  
  override func viewDidDisappear(animated: Bool) {
    super.viewDidDisappear(animated)
    ref!.removeObserverWithHandle(query)
    ref = nil
    query = 0
    page = nil
    incomingStrokes.removeAll()
  }

  override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
    for touch in touches {
      assert(strokeFitters[touch] === nil)
      let fitter = StrokeFitter(page: page!)
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
  
  // MARK: Actions
  
  @IBAction func googleSignOut(sender: AnyObject) {
    firebaseProvider.googleSignIn.signOut()
    firebaseProvider.getFirebaseRef().unauth()
  }
  
  @IBAction func clearPage(sender: AnyObject) {
    // TODO: if others are collaboratively editing this same page, it won't be cleared for them.
    ref!.removeValue()
    incomingStrokes.removeAll()
    page!.clear()
  }
}
