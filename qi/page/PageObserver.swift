//
//  PageObserver.swift
//  skeqi
//
//  Created by Josh Gargus on 8/19/16.
//  Copyright © 2016 Schwaftwarez. All rights reserved.
//

import Foundation

// TODO: document
class PageObserver : Equatable {
  func onFinalizeStroke(stroke: Stroke) -> Void {}
}

// PageObservers are equal only if they are identical.
func ==(lhs: PageObserver, rhs: PageObserver) -> Bool {
  return lhs === rhs
}