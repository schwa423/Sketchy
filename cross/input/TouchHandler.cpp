//
//  TouchHandler.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 9/6/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include "TouchHandler.h"

// namespace Sketchy::grfx
namespace Sketchy {namespace Input {

Touch::Touch(uint64_t ident, float xx, float yy, Phase ph, double time, unsigned taps) :
    _id(ident), _x(xx), _y(yy), _phase(ph), _timestamp(time), _tapCount(taps) {
}

}}  // namespace Sketchy::Input
