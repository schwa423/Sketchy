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

#include "Renderer.h"
#include "Framebuffer.h"
#include "Page.h"
using Sketchy::Renderer;
using Sketchy::Framebuffer;
using Sketchy::Page;

#import <iostream>
using std::cerr;
using std::endl;

// TODO: pause rendering when view is not visible,
//       and resume once it is visible again.

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

	m_renderer = Renderer::New(glayer);
	m_page = Page::New(m_renderer, m_renderer->defaultFramebuffer());

	// Error detection and reporting.
	if (!m_renderer || !m_page) {
		if (!m_renderer) { cerr << "could not instantiate renderer in EAGLView" << endl; }
		if (!m_page) { cerr << "could not instantiate page in EAGLView" << endl; }
		m_page.reset();
		m_renderer.reset();
		return nil;
	}

	// Hack something in for the renderer to draw.
	m_renderer->addPage(m_page);

    return self;
}

- (void)dealloc
{
	// No need to tear down renderer here... Objective-C++ is 
	// smart enough to automatically destroy the shared_ptr to
	// the renderer.
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
