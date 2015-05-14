//
//  QiProtocols.swift
//  skeqi
//
//  Created by Josh Gargus on 5/14/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

import Foundation
import Metal

@objc protocol QiDrawable {
    func encodeDrawCalls(renderEncoder: MTLRenderCommandEncoder)
}
