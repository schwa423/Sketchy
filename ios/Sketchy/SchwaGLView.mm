//
//  SchwaGLView.mm
//  Sketchy
//
//  Created by Joshua Gargus on 12/24/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import "SchwaGLView.h"

#include "renderer_ios.h"
#include "view.h"
using schwa::grfx::Renderer_iOS;
using schwa::grfx::View;

// TODO: Un-hardwire this
#include "pageview.h"
using namespace schwa::app;

#import <iostream>
using std::cerr;
using std::endl;

// TODO: pause rendering when view is not visible,
//       and resume once it is visible again.

// A class extension to declare private methods
//@interface SchwaGLView (private)

//- (BOOL)createFramebuffer;
//- (void)destroyFramebuffer;

//@end


@implementation SchwaGLView

// Subclasses of UIView must override this to use a
// non-standard CALayer as a backing store.
+ (Class)layerClass
{
	return [CAEAGLLayer class];
}

- (BOOL)initRenderer
{
    _renderer = Renderer_iOS::New();
    if (!_renderer) return FALSE;

    _view = View::New<sketchy::PageView>(_renderer);
    _renderer->setView(_view);
    return TRUE;
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

   if (![self initRenderer]) {
        cerr << "Failed to initialize renderer" << endl;
        return nil;
    }
    cerr << "Successfully initialized renderer" << endl;
    return self;
}

- (void)dealloc
{
	// No need to tear down renderer here... Objective-C++ is 
	// smart enough to automatically destroy the shared_ptr to
	// the renderer.
	cerr << "dealloc SchwaGLView" << endl;
}

- (void)startRendering
{
    _renderer->startRendering();

}

- (void)stopRendering
{
	_renderer->stopRendering();
}

- (void)updateOrientation:(UIInterfaceOrientation)orientation
{
    cerr << "[SchwaGLView updateOrientation:]" << endl;

    // Stop rendering so that it's OK for the renderer
    // to use its OpenGL context in this thread.  Remember
    // whether we were rendering, so we can resume.
    bool wasRendering = _renderer->isRunning();
    _renderer->stopRendering();

    // Allow the renderer to update appropriately.
    _renderer->initialize((CAEAGLLayer*)self.layer);

    if (wasRendering) _renderer->startRendering();
}

@end
