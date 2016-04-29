//
//  QiController
//  skeqi
//
//  Created by Josh Gargus on 4/10/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

import UIKit
import Metal

// This delegates to the C++ version of Skeqi, which will probably be deleted soon.
class QiController: GameViewController {
  var delegate : QiControllerDelegate

  required init?(coder: NSCoder) {
    delegate = Skeqi_iOS()

    super.init(coder: coder)
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
