//
//  Skeqi_iOS.mm
//  skeqi
//
//  Created by Josh Gargus on 5/15/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

#import "Skeqi_iOS.h"

// Interface extensions that can't be exposed to Swift because they use C++.
#import "QiControllerDelegateImpl.h"

#import "qi/gfx/port_ios/device_ios.h"
#import "qi/ui/touchhandler.h"

#import "skeqi-Swift-Wrapper.h"

// TODO: remove <iostream>
#import <iostream>
#import <map>

namespace qi {
namespace ui {

class SkeqiStrokeFitter {
 public:
  SkeqiStrokeFitter() : fitter_id(s_next_fitter_id++) {}
  ~SkeqiStrokeFitter() {}

  void startStroke() {
    std::cerr << "FITTER-" << fitter_id << ":  STARTING STROKE" << std::endl;
  }
  void addSamplePoint(Pt2f pt) {
    std::cerr << "FITTER-" << fitter_id
              << ":  ADDING POINT ("  << pt.x << "," << pt.y << ")" << std::endl;
  }
  void finishStroke() {
    std::cerr << "FITTER-" << fitter_id << ":  FINISHING STROKE" << std::endl;
  }

  // Identify the stroke fitter.
  const int64 fitter_id;

 private:
  static int64 s_next_fitter_id;
};

int64 SkeqiStrokeFitter::s_next_fitter_id = 1;

class SkeqiTouchHandler : public TouchHandler {
 public:
  void TouchesBegan(const std::vector<Touch>* touches) override {
    for (auto touch : *touches) {
      ASSERT(fitters_.find(touch) == fitters_.end());
      auto& fitter = (fitters_[touch] = make_unique<SkeqiStrokeFitter>());
      fitter->startStroke();
      fitter->addSamplePoint(touch.position());
    }
  }

  void TouchesCancelled(const std::vector<Touch>* touches) override {
    ASSERT(false);  // not implemented
  }

  void TouchesMoved(const std::vector<Touch>* touches) override {
    for (auto touch : *touches) {
      auto it = fitters_.find(touch);
      ASSERT(it != fitters_.end());
      it->second->addSamplePoint(touch.position());
    }
  }

  void TouchesEnded(const std::vector<Touch>* touches) override {
    for (auto touch : *touches) {
      auto it = fitters_.find(touch);
      ASSERT(it != fitters_.end());
      it->second->addSamplePoint(touch.position());
      it->second->finishStroke();
      fitters_.erase(it);
    }
  }

 private:
  std::map<Touch, unique_ptr<SkeqiStrokeFitter>> fitters_;
};

}  // namespace ui
}  // namespace qi


@implementation Skeqi_iOS

- (id)init {
  self = [super init];
  if (self) {
    auto touch_handler = std::make_unique<qi::ui::SkeqiTouchHandler>();
    [self setTouchHandler: std::move(touch_handler)];
  }
  return self;
}

@end
