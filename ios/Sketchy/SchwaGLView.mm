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

- (BOOL)initRenderer:(CAEAGLLayer *)glayer
{
    _renderer = std::dynamic_pointer_cast<Renderer_iOS>(Renderer_iOS::New(glayer));
    if (!_renderer) return FALSE;

    _view = grfx::View::New<sketchy::PageView>(_renderer);
    if (!_view) return FALSE;
    _view->setFramebuffer(_renderer->framebuffer());
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

   if (![self initRenderer:glayer]) {
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

- (void)pauseRendering
{
    _renderer->pauseRendering();

}

- (void)unpauseRendering
{
	_renderer->unpauseRendering();
}

// TODO: finish this!!!
- (void)updateOrientation
{
    bool wasPaused = _renderer->isPaused();
    _renderer->pauseRendering();

    CGRect rect = [self.layer bounds];
    int w = (int)rect.size.width;
    int h = (int)rect.size.height;

    cerr << "updateOrientation: layer bounds now: " << w << "/" << h << endl;
    // _renderer->initialize((CAEAGLLayer*)self.layer);

    if (!wasPaused) _renderer->unpauseRendering();
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
