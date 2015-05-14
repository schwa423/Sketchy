//
//  BezierFitter.h
//  skeqi
//
//  Created by Josh Gargus on 5/10/15.
//  Copyright (c) 2015 Schwaftwarez. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

@class Page;

@interface BezierFitter : NSObject

- (void)reset:(Page*) page;
- (void)add:(CGPoint) point;
- (void)finish;

+ (void)benchmark;

@end
