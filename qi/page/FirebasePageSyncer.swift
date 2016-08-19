//
//  FirebasePageSyncer.swift
//  skeqi
//
//  Created by Josh Gargus on 8/19/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Foundation

// Bidirectional syncer:
// - observes Page and writes new strokes to Firebase
// - observes Firebase and adds new strokes to the Page
class FirebasePageSyncer : PageObserver {
  let ref:FIRDatabaseReference
  let page:Page
  var incomingStrokes = [Stroke]()
  var outgoingStrokes = [String: [StrokeSegment]]()
  
  var query: UInt = 0
  
  init(_ firebase:FIRDatabaseReference, page:Page) {
    self.ref = firebase
    self.page = page
    super.init()
    
    self.query = firebase.observeEventType(.ChildAdded, withBlock: { (snapshot: FIRDataSnapshot!) -> Void in
      let array = snapshot.value as! NSArray
      assert(array.count % 8 == 0, "Array must be multiple of size of Bezier3")
      var path = [StrokeSegment]()
      path.reserveCapacity(array.count % 8)
      for i in 0.stride(to:array.count, by: 8) {
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
        assert(outPath == path)
        self.outgoingStrokes.removeValueForKey(snapshot.key)
        return
      }
      
      // Create a new stroke.  Remember that it originated from Firebase, so that we don't
      // resend it out to Firebase.
      let stroke = page.newStroke()
      self.incomingStrokes.append(stroke)
      page.setStrokePath(stroke, path: path)
      page.finalizeStroke(stroke)
    });
  }
  
  // TODO: what would be the idiomatic name for this?
  func destroy() {
    ref.removeObserverWithHandle(query)
  }
  
  override func onFinalizeStroke(stroke: Stroke) -> Void {
    if let index = incomingStrokes.indexOf(stroke) {
      // This stroke was received from Firebase, so don't send it back out.
      incomingStrokes.removeAtIndex(index)
      return
    }
    
    // Send the newly-finalized stroke to Firebase.
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
