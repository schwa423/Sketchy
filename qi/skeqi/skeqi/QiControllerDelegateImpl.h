//
//  QiControllerDelegateImpl.h
//  skeqi
//
//  Created by Josh Gargus on 5/17/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

#import "QiControllerDelegate.h"

#import "qi/ui/touchhandler.h"

// Private extension of QiControllerDelegate.  Uses C++, therefore must not be
// exposed to Swift.
// TODO: rename file to QiControllerDelegateNonSwift?
@interface QiControllerDelegate ()
- (void)setTouchHandler:(std::unique_ptr<qi::ui::TouchHandler>)touchHandler;
@end
