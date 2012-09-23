//
//  renderer_ios.mm
//  schwa::grfx
//
//  Created by Josh Gargus on 9/9/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "renderer_ios.h"
#include "framebuffer_ios.h"
#include "renderbuffer_ios.h"

#import <QuartzCore/QuartzCore.h>

#include <iostream>
using std::cerr;
using std::endl;

// This is a helper class that only exists to act as
// a callback target for a CADisplayLink.  It is wholly
// owned by a Renderer instance, and never exposed.
@interface VsyncListener : NSObject {
    @private
    schwa::grfx::Renderer_iOS *_renderer;
    EAGLContext *_context;
	NSThread *_thread;
	NSRunLoop *_loop;
}
- (void) render:(CADisplayLink*)link;
- (void) start;
- (void) stop;
- (bool) isPaused;
- (void) dealloc;
@end
@implementation VsyncListener
- (id)initWithRenderer:(schwa::grfx::Renderer_iOS*)renderer context:(EAGLContext*)context
{
	self = [super init];
	if (self) {
        _renderer = renderer;
        _context = context;
    }
	return self;
}
- (void)render:(CADisplayLink*)link
{
	_renderer->render();
}
- (void)displayLinkLoop
{
	@synchronized(_thread) {
		cerr << "running vsync loop at priority: " << [NSThread threadPriority] << endl;
        BOOL success = [EAGLContext setCurrentContext: _context];
        if (success != YES)
            cerr << "VsyncListener could not set OpenGL context." << endl;

		// Create a display-link, but don't start it up yet.
		CADisplayLink *link;
		link = [[UIScreen mainScreen]
                displayLinkWithTarget:self
                selector:@selector(render:)];
		[link setFrameInterval:1];

		_loop = [NSRunLoop currentRunLoop];
		[link addToRunLoop:_loop forMode:NSRunLoopCommonModes];

		CFRunLoopRun();

		// TODO: what kind of cleanup do we need to do here?
		//       I think this should be sufficient.
		cerr << "stopping vsync loop" << endl;
		[link invalidate];
        [EAGLContext setCurrentContext: nil];
	}
}
- (void)start
{
	@synchronized(self) {
		if (_thread) return; // already started

		_thread = [[NSThread alloc]
					initWithTarget:self
					selector:@selector(displayLinkLoop)
					object:nil];
		[_thread setThreadPriority: 1.0];
		[_thread start];
	}
}
- (void)stop
{
	@synchronized(self) {
		// If thread has already been stopped, there is nothing to do.
		if (!_thread) return;

		// Notify the run-loop to stop, and...
		CFRunLoopStop([_loop getCFRunLoop]);

		// ... wait for thread to finish.
		@synchronized(_thread) {
			_thread = nil;
			_loop = nil;
		}
	}
}
- (bool)isPaused
{
    @synchronized(self) {
        return !_thread;
    }
}
- (void)dealloc
{
	// Ensure that we've stopped our worker-thread.
	[self stop];
	cerr << "dealloc called in VsyncListener" << endl;
}
@end


// namespace schwa::grfx
namespace schwa {namespace grfx {

shared_ptr<Renderer> Renderer_iOS::New(CAEAGLLayer* glayer) {

    shared_ptr<Renderer> renderer;
    auto ios = new Renderer_iOS;
    if (ios) {
        renderer.reset(ios);
        ios->initialize(glayer);
    }
    return renderer;
}


Renderer_iOS::Renderer_iOS() {

    // TODO: verify success of context creations.
    _renderContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    _loaderContext = [[EAGLContext alloc]
                        initWithAPI:kEAGLRenderingAPIOpenGLES2
                        sharegroup:[_renderContext sharegroup]];
    if (_renderContext == nil || _loaderContext == nil)
        cerr << "Renderer_iOS could not create OpenGL contexts." << endl;

    _vsync = [[VsyncListener alloc] initWithRenderer:this context:_renderContext];
}


void Renderer_iOS::pauseRendering() {
    [_vsync stop];
}


void Renderer_iOS::unpauseRendering() {
    [_vsync start];
}


bool Renderer_iOS::isPaused() {
    return [_vsync isPaused];
}


// TODO: if re-initializing, need to destroy previous renderbuffers.
void Renderer_iOS::initialize(CAEAGLLayer* glayer) {

    // Set render context to be current, so that we can
    // create framebuffers/renderbuffers, etc.
    BOOL success = [EAGLContext setCurrentContext: _renderContext];
    if (success != YES)
        cerr << "Renderer_iOS couldnot set OpenGL context." << endl;

    initializeMultisampleFramebuffer(glayer);

    // We're finished initializing OpenGL resources, so flush and
    // unset the current context... from now on, the _renderContext
    // will only be used in the vsync thread.
    glFlush();
    [EAGLContext setCurrentContext: nil];
}


void Renderer_iOS::initializeFramebuffer(CAEAGLLayer* layer) {
    // We will replace it with a new one.
    _framebuffer.reset();

    CGRect rect = [layer bounds];
    int width = (int)rect.size.width;
    int height = (int)rect.size.height;

    cerr << "Renderer_iOS initializing framebuffer with width/height: "
        << width << "/" << height << endl;

    auto me = shared_from_this();

    auto color_renderbuffer = Renderbuffer_iOS::NewFromLayer(me, _renderContext, layer);
    auto depth_renderbuffer = Renderbuffer::NewDepth(me, width, height, 1);

    if (!color_renderbuffer || !depth_renderbuffer) {
        cerr << "Renderer_iOS failed to create renderbuffers for default framebuffer" << endl;
        color_renderbuffer.reset();
        depth_renderbuffer.reset();
        return;
    }

    _framebuffer.reset(new Framebuffer(me,
                                       width, height,
                                       color_renderbuffer,
                                       depth_renderbuffer));
}


void Renderer_iOS::initializeMultisampleFramebuffer(CAEAGLLayer* layer) {
    // We will replace it with a new one.
    _framebuffer.reset();

    CGRect rect = [layer bounds];
    int width = (int)rect.size.width;
    int height = (int)rect.size.height;

    cerr << "Renderer_iOS initializing multisample framebuffer with width/height: "
         << width << "/" << height << endl;

    auto me = shared_from_this();

    auto color_renderbuffer = Renderbuffer_iOS::NewFromLayer(me, _renderContext, layer);
    auto depth_renderbuffer = Renderbuffer::NewDepth(me, width, height);
    auto multisample_color_renderbuffer = Renderbuffer::NewColor(me, width, height, 4);
    auto multisample_depth_renderbuffer = Renderbuffer::NewDepth(me, width, height, 4);

    if (!color_renderbuffer || !depth_renderbuffer ||
        !multisample_color_renderbuffer || !multisample_depth_renderbuffer) {
        cerr << "Renderer_iOS failed to create renderbuffers for default framebuffer" << endl;
        color_renderbuffer.reset();
        depth_renderbuffer.reset();
        multisample_color_renderbuffer.reset();
        multisample_depth_renderbuffer.reset();
        return;
    }

    _framebuffer.reset(
        new MultisampleFramebuffer_iOS(me,
                                       width, height,
                                       color_renderbuffer,
                                       depth_renderbuffer,
                                       multisample_color_renderbuffer,
                                       multisample_depth_renderbuffer));
}


void Renderer_iOS::swapBuffers() {
    colorRenderbuffer()->bind();
    CHECK_GL("filed to bind renderbuffer for swapBuffers()");
    if (YES != [_renderContext presentRenderbuffer:GL_RENDERBUFFER]) {
        cerr << "failed to present renderbuffeer" << endl;
    }
}


}}  // namespace schwa::grfx