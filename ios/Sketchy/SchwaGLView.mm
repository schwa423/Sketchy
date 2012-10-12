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
#include "presenter_ios.h"
using schwa::grfx::Renderer_iOS;
using schwa::grfx::LayerPresenter_iOS;

#include "view.h"
using schwa::grfx::View;

#import <iostream>
using std::cerr;
using std::endl;


@implementation SchwaGLView

// Subclasses of UIView must override this to use a
// non-standard CALayer as a backing store.
+ (Class)layerClass
{
	return [CAEAGLLayer class];
}

- (const shared_ptr<grfx::Renderer_iOS>&) renderer
{
    return _renderer;
}

- (const shared_ptr<grfx::LayerPresenter_iOS>&) presenter
{
    return _presenter;
}

// Renderer must already be initialized.
- (BOOL)initPresenter
{
    _presenter = _renderer->NewLayerPresenter();
    if (!_presenter) return FALSE;
    _renderer->addPresenter(_presenter);
    return TRUE;
}

- (BOOL)initRenderer
{
    _renderer = Renderer_iOS::New();
    if (!_renderer) return FALSE;
    return [self initPresenter];
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

    _renderer->pauseRenderingDuring([&](){
        _presenter->setLayer((CAEAGLLayer*)self.layer);
    });
}

@end
