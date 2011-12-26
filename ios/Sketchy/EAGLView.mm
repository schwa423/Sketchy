//
//  EAGLView.m
//  Sketchy
//
//  Created by Joshua Gargus on 12/24/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "EAGLView.h"

@implementation EAGLView

// Subclasses of UIView must override this to use a
// non-standard CALayer as a backing store.
+ (Class)layerClass
{
	return [CAEAGLLayer class];
}



- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) {
		CAEAGLLayer *glayer = (CAEAGLLayer *)self.layer;
		glayer.opaque = TRUE;
		glayer.drawableProperties = 
			[NSDictionary dictionaryWithObjectsAndKeys:
			 [NSNumber numberWithBool: FALSE], kEAGLDrawablePropertyRetainedBacking,
			 kEAGLDrawablePropertyRetainedBacking, kEAGLDrawablePropertyColorFormat,
			 nil];
		self.contentScaleFactor = [[UIScreen mainScreen] scale];
    }
    return self;
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/

@end
