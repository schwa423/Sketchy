//
//  TouchHandler.cpp
//  schwa::input
//
//  Created by Joshua Gargus on 9/6/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

#include "TouchHandler.h"

// namespace schwa::input
namespace schwa {namespace input {

Touch::Touch(uint64_t ident, float xx, float yy, Phase ph, double time, unsigned taps) :
    _id(ident), _x(xx), _y(yy), _phase(ph), _timestamp(time), _tapCount(taps) {
}

}}  // namespace schwa::input
