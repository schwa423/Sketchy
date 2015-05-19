#ifndef _QI_UI_TouchHandler_h_
#define _QI_UI_TouchHandler_h_

#include "qi.h"
#include <vector>

namespace qi {
namespace ui {

// TODO: move to shared header
struct Pt2f {
  float x;
  float y;
};

// See comment for "struct Touch", below.
typedef uint64 TouchId;

// When the user's finger touches the screen, moves, and is lifted it generates
// a sequence of Touches.  Since multiple fingers may simultaneously touch the
// screen, the |touch_id| field is used by a TouchHandler to identify which
// Touches belong to the same TouchesBegan/Moved/Ended() sequence.
//
// TODO: test Touch equality
struct Touch {
  const double x;
  const double y;

  const TouchId touch_id;

  // TODO oooogly static_cast
  Pt2f position() const { return Pt2f{static_cast<float>(x),static_cast<float>(y)}; }
};

// Allow Touch objects to be sorted, so that they can be added to a std::map.
inline bool operator<(const Touch& t1, const Touch& t2) {
  return t1.touch_id < t2.touch_id;
}

// Interface to allow app to handle simultaneous/overlapping sequences of touch-
// begin/moved/end events.
class TouchHandler {
 public:
  virtual void TouchesBegan(const std::vector<Touch>* touches) {}
  virtual void TouchesCancelled(const std::vector<Touch>* touches) {}
  virtual void TouchesMoved(const std::vector<Touch>* touches) {}
  virtual void TouchesEnded(const std::vector<Touch>* touches) {}

  // Return true if TouchHandler prefers touch-positions to be normalized
  // to the range [-1,-1],[1,1], and false if it wants raw device coordinates.
  virtual bool WantsNormalizedTouchPositions() { return true; }
};

}  // namespace ui
}  // namespace qi

#endif  // _QI_UI_TouchHandler_h_
