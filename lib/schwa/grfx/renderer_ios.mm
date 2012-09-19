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
- (void) dealloc;
@end
@implementation VsyncListener
- (id)initWithRenderer:(schwa::grfx::Renderer_iOS*)renderer context:(EAGLContext*)context
{
	self = [super init];
	if (self) _renderer = renderer;
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
        [EAGLContext setCurrentContext: _context];

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

    _vsync = [[VsyncListener alloc] initWithRenderer:this context:_renderContext];
}


void Renderer_iOS::pauseRendering() {
    [_vsync stop];
}


void Renderer_iOS::unpauseRendering() {
    [_vsync start];
}


void Renderer_iOS::initialize(CAEAGLLayer* glayer) {

    // Set render context to be current, so that we can
    // create framebuffers/renderbuffers, etc.
    [EAGLContext setCurrentContext: _renderContext];


    initializeFramebuffer(glayer);


    // We're finished initializing OpenGL resources, so flush and
    // unset the current context... from now on, the _renderContext
    // will only be used in the vsync thread.
    glFlush();
    [EAGLContext setCurrentContext: nil];
}


void Renderer_iOS::initializeFramebuffer(CAEAGLLayer* glayer) {
    CGRect rect = [glayer bounds];
    int w = (int)rect.size.width;
    int h = (int)rect.size.height;

    cerr << "Renderer_iOS initializing framebuffer with width/height: " << w << "/" << h << endl;

    GLuint color_renderbuffer, multisample_color_renderbuffer,
           depth_renderbuffer, multisample_depth_renderbuffer;

    glGenRenderbuffers(1, &color_renderbuffer);
    glGenRenderbuffers(1, &depth_renderbuffer);
    glGenRenderbuffers(1, &multisample_color_renderbuffer);
    glGenRenderbuffers(1, &multisample_depth_renderbuffer);

    glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);
    BOOL result = [_renderContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:glayer];
    if (result != YES) {
        cerr << "failed to create color_renderbuffer from drawable layer" << endl;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
    CHECK_GL("failed to create depth_renderbuffer");

    // TODO: pick one
    /*
    _framebuffer.reset(new Framebuffer(shared_from_this(),
                                       color_renderbuffer,
                                       depth_renderbuffer,
                                       w, h));
    */

    glBindRenderbuffer(GL_RENDERBUFFER, multisample_color_renderbuffer);
    glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, 4, GL_RGBA8_OES, w, h);
    CHECK_GL("failed to create multisample_color_renderbuffer");

    glBindRenderbuffer(GL_RENDERBUFFER, multisample_depth_renderbuffer);
    // TODO: pick one
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
//    glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT16, w, h);
    CHECK_GL("failed to create multisample_depth_renderbuffer");

    _framebuffer.reset(new MultisampleFramebuffer_iOS(shared_from_this(),
                                                      color_renderbuffer,
                                                      depth_renderbuffer,
                                                      multisample_color_renderbuffer,
                                                      multisample_depth_renderbuffer,
                                                      w, h));
}


void Renderer_iOS::swapBuffers() {
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer());
    if (YES != [_renderContext presentRenderbuffer: GL_RENDERBUFFER]) {
        cerr << "failed to present renderbuffeer" << endl;
    }
}


}}  // namespace schwa::grfx