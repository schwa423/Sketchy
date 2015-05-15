//
//  Skeqi_iOS.h
//  skeqi
//
//  Created by Josh Gargus on 5/15/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

@protocol QiControllerDelegate<NSObject>
// TODO remove
- (void)ping;
@end

@interface Skeqi_iOS : NSObject<QiControllerDelegate>
- (void)ping;
@property (readonly) id<MTLDevice> metalDevice;
@property (readonly) id<MTLLibrary> metalLibrary;
@end
