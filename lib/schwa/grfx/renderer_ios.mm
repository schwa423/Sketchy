//
//  renderer_ios.mm
//  schwa::grfx
//
//  Created by Josh Gargus on 9/9/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "renderer_ios.h"
#include "framebuffer_ios.h"
#include "renderbuffer_ios.h"
#include "presenter_ios.h"
#include "view.h"

#import <QuartzCore/QuartzCore.h>

typedef std::lock_guard<std::mutex> lock_guard;

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
- (bool) isRunning;
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

        // This is a hack to avoid a race-condition that can occur if
        // stop() is called immediately after start(), before _loop is set.
        while (!_loop) {
            usleep(5000);
            cerr << "waiting for vsync loop to start" << endl;
        }
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
- (bool)isRunning
{
    @synchronized(self) {
        return _thread != nullptr;
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

shared_ptr<Renderer_iOS> Renderer_iOS::New() {
    return shared_ptr<Renderer_iOS>(new Renderer_iOS);
}


Renderer_iOS::Renderer_iOS() {

    // TODO: verify success of context creations.
    _renderContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    _loaderContext = [[EAGLContext alloc]
                      initWithAPI:kEAGLRenderingAPIOpenGLES2
                      sharegroup:[_renderContext sharegroup]];
    _defaultContext = [[EAGLContext alloc]
                      initWithAPI:kEAGLRenderingAPIOpenGLES2
                      sharegroup:[_renderContext sharegroup]];

    if (_renderContext == nil || _loaderContext == nil || _defaultContext == nil) {
        cerr << "Renderer_iOS could not create OpenGL contexts." << endl;
        _renderContext = _loaderContext = _defaultContext = nil;
        return;
    }

    if (YES != [EAGLContext setCurrentContext: _defaultContext]) {
        cerr << "Renderer_iOS could not set default OpenGL context." << endl;
        _renderContext = _loaderContext = _defaultContext = nil;
        return;
    }

    _vsync = [[VsyncListener alloc] initWithRenderer:this context:_renderContext];
}


Renderer_iOS::~Renderer_iOS() {
    cerr << "destroying Renderer_iOS" << endl;
}


void Renderer_iOS::startRendering() {
    lock_guard lock(_mutex);
    [_vsync start];
    cerr << "Renderer_iOS: started rendering" << endl;
}


void Renderer_iOS::stopRendering() {
    lock_guard lock(_mutex);
    [_vsync stop];
    cerr << "Renderer_iOS: stopped rendering" << endl;
}


bool Renderer_iOS::isRunning() {
    return [_vsync isRunning];
}


shared_ptr<Framebuffer> Renderer_iOS::NewFramebuffer(CAEAGLLayer* layer, bool multisample) {
    bool wasRendering = isRunning();
    if (wasRendering) stopRendering();
    core::ThunkAfterScope([&](){
        if (wasRendering) startRendering();
    });

    // You can't create two renderbuffers for the same layer; one must be
    // destroyed before the second created.  Ensure that finalizers have
    // a chance to run.  This is only safe if we're not rendering; otherwise
    // the render-context might be using the old framebuffer/renderbuffers at
    // the same time.
    runFinalizers();

    CGRect rect = [layer bounds];
    int width = (int)rect.size.width;
    int height = (int)rect.size.height;

    cerr << "Renderer_iOS creating framebuffer with width/height: "
         << width << "/" << height << endl;

    auto me = shared_from_this();

    // TODO: verify we're on the main thread (otherwise _defaultContext is wrong)
    shared_ptr<Framebuffer> framebuffer;
    if (multisample) {
        framebuffer = MultisampleFramebuffer_iOS::NewFromLayer(me,
                                                               _defaultContext,
                                                               layer,
                                                               true);
    } else {
        auto color_renderbuffer = Renderbuffer_iOS::NewFromLayer(me, _defaultContext, layer);
        auto depth_renderbuffer = Renderbuffer::NewDepth(me, width, height, 1);
        framebuffer = Framebuffer::New(me, color_renderbuffer, depth_renderbuffer);
    }

    glFlush();  // so that framebuffer can be used in _renderContext
    return framebuffer;
}


shared_ptr<LayerPresenter_iOS> Renderer_iOS::NewLayerPresenter() {
    return shared_ptr<LayerPresenter_iOS>(new LayerPresenter_iOS(shared_from_this(),
                                                                _renderContext,
                                                                _defaultContext));
}


}}  // namespace schwa::grfx