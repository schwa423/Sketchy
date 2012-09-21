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


// TODO: Un-hardwire this
#include "pageview.h"
using namespace schwa::app;

// Old shit. =================
#include "Renderer.h"
#include "Framebuffer.h"
#include "Page.h"
using Sketchy::Renderer;
using Sketchy::Framebuffer;
using Sketchy::Page;
// End old shit. =============

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

- (BOOL)initOldRenderer:(CAEAGLLayer *)glayer
{
    _renderer2 = Renderer::New(glayer);
    _page2 = Page::New(_renderer2, _renderer2->defaultFramebuffer());

    // Error detection and reporting.
	if (!_renderer2 || !_page2) {
		if (!_renderer2) { cerr << "could not instantiate renderer in SchwaGLView" << endl; }
		if (!_page2) { cerr << "could not instantiate page in SchwaGLView" << endl; }
		_page2.reset();
		_renderer2.reset();
		return FALSE;
	}

	// Hack something in for the renderer to draw.
	_renderer2->addPage(_page2);
    return TRUE;
}

- (BOOL)initRenderer:(CAEAGLLayer *)glayer
{
    _renderer = schwa::grfx::Renderer_iOS::New(glayer);
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

//   if (![self initOldRenderer:glayer]) {
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
    if (_renderer.get()) _renderer->pauseRendering();
    if (_renderer2.get()) _renderer2->pauseRendering();
}

- (void)unpauseRendering
{
	if (_renderer.get()) _renderer->unpauseRendering();
    if (_renderer2.get()) _renderer2->unpauseRendering();
}

- (shared_ptr<Sketchy::Page>)page2
{
    return _page2;
}

- (shared_ptr<Sketchy::Page>)page
{
    return _page2;
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
