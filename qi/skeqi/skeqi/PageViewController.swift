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
  var observer: FirebasePageObserver? = nil
  
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

  // TODO: move ref into FirebasePageObserver?
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

    let observer = self.observer!
    for incoming in observer.incomingStrokes {
      let stroke = page.newStroke()
      stroke.fromFirebase = true
      page.setStrokePath(stroke, path: incoming)
      page.finalizeStroke(stroke)
    }
    observer.incomingStrokes.removeAll()
    
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
    observer = FirebasePageObserver(ref!)
    page!.addObserver(observer)
  }
  
  override func viewDidDisappear(animated: Bool) {
    super.viewDidDisappear(animated)
    observer!.destroy()
    observer = nil
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

class FirebasePageObserver : PageObserver {
  let ref: FIRDatabaseReference
  var incomingStrokes = [[StrokeSegment]]()
  var outgoingStrokes = [String: [StrokeSegment]]()
  
  var query: UInt = 0
  
  init(_ firebase: FIRDatabaseReference) {
    self.ref = firebase
    super.init()
    
    self.query = firebase.observeEventType(.ChildAdded, withBlock: { (snapshot: FIRDataSnapshot!) -> Void in
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

      // Detect if the stroke was drawn on this device.  If so, don't add a duplicate copy.
      if let outPath = self.outgoingStrokes[snapshot.key] {
        if outPath == path {
          self.outgoingStrokes.removeValueForKey(snapshot.key)
          return
        }
      }
      self.incomingStrokes.append(path)
    });
  }
  
  // TODO: what would be the idiomatic name for this?
  func destroy() {
    ref.removeObserverWithHandle(query)
    incomingStrokes.removeAll()
  }
  
  override func onFinalizeStroke(stroke: Stroke!) -> Void {
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
      let childRef = ref.childByAutoId()
      outgoingStrokes[childRef.key] = stroke.path
      childRef.setValue(path)
    }
  }
}
