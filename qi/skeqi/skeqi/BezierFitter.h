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

@protocol QiStrokeFitter<NSObject>
- (void)startStroke;
- (void)addSamplePoint:(CGPoint)point;
- (void)finishStroke;
@end

@interface BezierFitter : NSObject<QiStrokeFitter>
- (id)initWithPage:(Page*) page;
- (void)startStroke;
- (void)addSamplePoint:(CGPoint)point;
- (void)finishStroke;

+ (void)benchmark;

@end
