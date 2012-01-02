//
//  EAGLView.mm
//  Sketchy
//
//  Created by Joshua Gargus on 12/24/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import "EAGLView.h"

#import <iostream>
using std::cerr;
using std::endl;

// TODO pause rendering when view is not visible, and 
// resume once it is visible again.

// A class extension to declare private methods
//@interface EAGLView (private)

//- (BOOL)createFramebuffer;
//- (void)destroyFramebuffer;

//@end


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
	if (!self) return nil;

	CAEAGLLayer *glayer = (CAEAGLLayer *)self.layer;
	glayer.opaque = TRUE;
	glayer.drawableProperties = 
		[NSDictionary dictionaryWithObjectsAndKeys:
		 [NSNumber numberWithBool: FALSE], kEAGLDrawablePropertyRetainedBacking,
		 kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
		 nil];
	self.contentScaleFactor = [[UIScreen mainScreen] scale];

	m_renderer.reset(new Renderer(glayer));
	if (!m_renderer) return nil;
	// TODO what if renderer can't be instantiated?

    return self;
}

- (void)dealloc
{
	// TODO need to tear down renderer here?  Probably.
	cerr << "dealloc in EAGLView" << endl;
}

- (void)pauseRendering
{
	m_renderer->pauseRendering();
}

- (void)unpauseRendering
{
	m_renderer->unpauseRendering();
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
