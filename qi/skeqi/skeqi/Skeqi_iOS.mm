//
//  Skeqi_iOS.mm
//  skeqi
//
//  Created by Josh Gargus on 5/15/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

#import "Skeqi_iOS.h"
#import "qi/gfx/port_ios/device_ios.h"

#import "skeqi-Swift-Wrapper.h"

// TODO: for ping
#import <iostream>

@implementation Skeqi_iOS
{
    std::shared_ptr<qi::gfx::port::Device_iOS> device_;
}

- (id)init {
    self = [super init];
    if (self) {
        self->_metalDevice = MTLCreateSystemDefaultDevice();
        self->_metalLibrary = [self.metalDevice newDefaultLibrary];
        device_ = std::make_shared<qi::gfx::port::Device_iOS>(self.metalDevice);
    }
    return self;
}

- (void)ping {
    std::cerr << "PING!!!!!!!" << std::endl;
}

@end
