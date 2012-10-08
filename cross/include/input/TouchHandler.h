//
//  TouchHandler.h
//  schwa::input
//
//  Created by Joshua Gargus on 9/6/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//


#ifndef __schwa__input__touch_handler__
#define __schwa__input__touch_handler__

#include <stdint.h>

#include <vector>

// namespace schwa::input
namespace schwa {namespace input {

class Touch {
 public:
    // These are adapted from iOS touch-handling;
    // maybe Android differs, and must be reconciled?
    enum Phase {
        Began,
        Moved,
        Stationary,
        Ended,
        Cancelled
    };

    Touch(uint64_t ident, float xx, float yy, Phase ph, double time, unsigned taps);

    uint64_t id() const { return _id; }
    float x() const { return _x; }
    float y() const { return _y; }
    Phase phase() const { return _phase; }
    unsigned timestamp() const { return _timestamp; }
    unsigned tapCount() const { return _tapCount; }

 protected:
    uint64_t _id;
    float _x, _y;
    Phase _phase;

    double _timestamp;
    unsigned _tapCount;
};


class TouchHandler {
 public:
    virtual bool touchesBegan(const std::vector<Touch>& touches) = 0;
    virtual bool touchesMoved(const std::vector<Touch>& touches) = 0;
    virtual bool touchesEnded(const std::vector<Touch>& touches) = 0;
    virtual bool touchesCancelled(const std::vector<Touch>& touches) = 0;
};


}}  // namespace schwa::input


#endif  // #ifndef __schwa__input__touch_handler__