//
//  QiControllerDelegate.h
//  skeqi
//
//  Created by Josh Gargus on 5/17/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

#ifndef skeqi_QiControllerDelegate_h
#define skeqi_QiControllerDelegate_h

#import <Metal/Metal.h>
#import <UIKit/UIKit.h>

// TODO: document
@interface QiControllerDelegate : UIResponder
- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event;
- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event;
- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event;
- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event;

- (void)encodeDrawCalls:(id<MTLRenderCommandEncoder>)encoder;

@property (readonly) id<MTLDevice> metalDevice;
@property (readonly) id<MTLLibrary> metalLibrary;
@end

#endif
