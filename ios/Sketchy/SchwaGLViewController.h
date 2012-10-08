//
//  SchwaGLViewController.h
//  Sketchy
//
//  Created by Joshua Gargus on 12/30/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "TouchHandler.h"

namespace schwa {
namespace input {
    class TouchHandler;
}
namespace grfx {
    class View;
}
}  // namespace schwa

#include <memory>
using std::shared_ptr;

@interface SchwaGLViewController : UIViewController {

@private
	shared_ptr<schwa::input::TouchHandler> _touchHandler;

    // Most recent orientation, and a flag to specify whether it is valid.
    bool _orientationValid;
    UIInterfaceOrientation _orientation;
}

- (void)setTouchHandler:(shared_ptr<schwa::input::TouchHandler>)handler;
- (void)updateOrientation;
@end
