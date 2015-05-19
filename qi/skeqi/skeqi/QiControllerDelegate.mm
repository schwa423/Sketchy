//
//  QiControllerDelegate.mm
//  skeqi
//
//  Created by Josh Gargus on 5/17/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

#import "QiControllerDelegateImpl.h"

#import <vector>
#import <map>

@implementation QiControllerDelegate
{
  @protected
  std::unique_ptr<qi::ui::TouchHandler> touch_handler_;

  @private
  std::shared_ptr<qi::gfx::port::Device_iOS> device_;

  // Reduce the number of allocations/deallocations by reusing this vector
  // instead of making each invocation of extractTouches: allocate a new vector.
  std::vector<qi::ui::Touch> touches_;

  // To allow the app to distinguish between multiple simultaneous touches, iOS
  // uses the same UITouch* object throughout a touchesBegan/Moved/Ended cycle.
  // Each time we encounter a new UITouch*, we map it to |next_touch_id_|, which
  // is then incremented.
  std::map<UITouch*, qi::ui::TouchId> touch_map_;
  qi::ui::TouchId next_touch_id_;
}

- (id)init {
    self = [super init];
    if (self) {
        self->_metalDevice = MTLCreateSystemDefaultDevice();
        self->_metalLibrary = [self.metalDevice newDefaultLibrary];
        self->_device = std::make_shared<qi::gfx::port::Device_iOS>(self.metalDevice);

        // Default handler ignores touch events.
        touch_handler_ = std::make_unique<qi::ui::TouchHandler>();
        next_touch_id_ = 0;
    }
    return self;
}

- (void)setTouchHandler:(std::unique_ptr<qi::ui::TouchHandler>)touchHandler {
  touch_handler_ = std::move(touchHandler);
}

// Implement UIResponder method.
- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
  // When a touch begins, add a mapping from the UITouch* to a newly-generated
  // qi::ui::TouchId.
  for (UITouch* touch in touches) {
    ASSERT(touch_map_.find(touch) == touch_map_.end());
    touch_map_[touch] = next_touch_id_++;
  }

  [self extractTouches: touches];
  touch_handler_->TouchesBegan(&touches_);
  touches_.clear();
}

// Implement UIResponder method.
- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
  [self extractTouches: touches];
  touch_handler_->TouchesCancelled(&touches_);
  touches_.clear();
}

// Implement UIResponder method.
- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
  [self extractTouches: touches];
  touch_handler_->TouchesMoved(&touches_);
  touches_.clear();
}

// Implement UIResponder method.
- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
  [self extractTouches: touches];
  touch_handler_->TouchesEnded(&touches_);
  touches_.clear();

  // When a touch ends, remove the mapping from the UITouch to the corresponding
  // qi::ui::TouchId.
  for (UITouch* touch in touches) {
    auto it = touch_map_.find(touch);
    ASSERT(it != touch_map_.end());
    touch_map_.erase(it);
  }
}

// Populate |touches_| with a new qi::ui::Touch for each touch-event in the set,
// and set its x/y position and touch-id.
- (void)extractTouches:(NSSet*)touches {
  ASSERT(touches_.empty());
  for (UITouch* touch in touches) {
    CGPoint pt = [touch locationInView: touch.view];
    if (touch_handler_->WantsNormalizedTouchPositions()) {
      pt.x = (pt.x / CGRectGetWidth(touch.view.bounds) * 2.0) - 1.0;
      pt.y = (pt.y / CGRectGetHeight(touch.view.bounds) * -2.0) + 1.0;
    }
    ASSERT(touch_map_.find(touch) != touch_map_.end());
    touches_.push_back(qi::ui::Touch{pt.x, pt.y, touch_map_[touch]});
  }
}

// No-op.  Subclasses should override if they have something to draw.
- (void)encodeDrawCalls:(id<MTLRenderCommandEncoder>)encoder {

}

@end
