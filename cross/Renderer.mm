//
//  Renderer.mm
//  Sketchy
//
//  Created by Joshua Gargus on 12/24/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

// TODO: add mechanism for making Render events "high-priority", so that they
//       are processed ASAP instead waiting in the same queue as other events.


#import "Renderer.h"
#import "Fence.h"

#import <QuartzCore/QuartzCore.h>

#include <iostream>
using std::cerr;
using std::endl;

// This is a helper class that only exists to act as
// a callback target for a CADisplayLink.  It is wholly
// owned by a Renderer instance, and never exposed.
@interface CADisplayLinkListener : NSObject {
@private
	Renderer *m_renderer;
	NSThread *m_thread;
	NSRunLoop *m_loop;
}
- (void) render:(CADisplayLink*)link;
- (void) start;
- (void) stop;
- (void) dealloc;
@end
@implementation CADisplayLinkListener
- (id)initWithRenderer:(Renderer*)renderer
{
	self = [super init];
	if (!self) return nil;
	m_renderer = renderer;

	[self start];

	return self;
}
- (void)render:(CADisplayLink*)link
{
	if (m_renderer) { m_renderer->render(); }
}
- (void)displayLinkLoop
{
	@synchronized(m_thread) {
		cerr << "running display-link loop at priority: " << [NSThread threadPriority] << endl;

		// Create a display-link, but don't start it up yet.
		CADisplayLink *link;
		link = [[UIScreen mainScreen]
					 displayLinkWithTarget:self
					 selector:@selector(render:)];
		[link setFrameInterval:1];

		m_loop = [NSRunLoop currentRunLoop];
		[link addToRunLoop:m_loop forMode:NSRunLoopCommonModes];

		CFRunLoopRun();

		// TODO: what kind of cleanup do we need to do here?
		//       I think this should be sufficient.
		cerr << "stopping display-link loop" << endl;
		[link invalidate];
	}
}
- (void)start
{
	@synchronized(self) {
		if (m_thread) return; // already started

		m_thread = [[NSThread alloc]
						initWithTarget:self
						selector:@selector(displayLinkLoop)
						object:nil];
		[m_thread setThreadPriority: 1.0];
		[m_thread start];
	}
}
- (void)stop
{
	@synchronized(self) {
		// If thread has already been stopped, there is nothing to do.
		if (!m_thread) return;

		// Notify the run-loop to stop, and...
		CFRunLoopStop([m_loop getCFRunLoop]);

		// ... wait for thread to finish.
		@synchronized(m_thread) {
			m_thread = nil;
			m_loop = nil;
		}
	}
}
- (void)dealloc 
{
	// Ensure that we've stopped our worker-thread.
	[self stop];
	cerr << "dealloc called in CADisplayLinkListener" << endl;
}
@end

// TODO: pause/unpause that plumb down to pause/unpause display-link
Renderer::Renderer(CAEAGLLayer *layer) :
	Loop(),
	m_hackGeometry(NULL),
	m_hackShader(NULL),
	m_framebuffer(0),
	m_renderbuffer(0),
	m_framebufferHeight(0),
	m_framebufferWidth(0),
	m_context(NULL)
{
	cerr << "creating renderer" << endl;

	Event::ptr init(new Init(this));
	Event::ptr framebuf(new CreateFramebuffer(this, layer));
	Fence::ptr fence(new Fence());	
	
	addTask(init);
	addTask(framebuf);
	addTask(fence);
	
	// Wait until initialization completes.
	fence->get_future().wait();

	cerr << "finished renderer initialization" << endl;

	m_displayLinkListener = [[CADisplayLinkListener alloc] initWithRenderer:this];
	// TODO: what if result was nil?
}

Renderer::~Renderer()
{
	cerr << "destroying renderer" << endl;
	// Need to stop it first, otherwise it won't
	// be released because the display-link still
	// retains a reference to it.
	[m_displayLinkListener stop];
	
	if (m_hackGeometry) {
		delete m_hackGeometry;
	}
	if (m_hackShader) {
		delete m_hackShader;
	}
}

void
Renderer::pauseRendering()
{
	[m_displayLinkListener stop];
}

void
Renderer::unpauseRendering()
{
	[m_displayLinkListener start];
}

void
Renderer::handleInit()
{
	cerr << "initializing renderer" << endl;

	m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

	// TODO: what if we couldn't create the context?
	[EAGLContext setCurrentContext: m_context];
	
	m_hackGeometry = new Geometry();
	m_hackShader = new Shader();
}


void
Renderer::handleCreateFramebuffer(CAEAGLLayer *layer)
{
	deleteFramebuffer();

	GLenum err;
	err = glGetError();
	if (err != GL_NO_ERROR) { cerr << "Error before creating framebuffer: " << err << endl; }

	glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	glGenRenderbuffers(1, &m_renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);

	err = glGetError();
	if (err != GL_NO_ERROR) { cerr << "Error after creating framebuffer: " << err << endl; }

	[m_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderbuffer);

	// Read back width and height of renderbuffer.
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &m_framebufferWidth);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &m_framebufferHeight);

	// Check that everything is OK
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		// TODO: need logging library
		cerr << "Failed to make complete framebuffer object: " << status << endl;
		deleteFramebuffer();
	}
}

// TODO: change this from a placeholder to a more general scene-graph renderer.
void
Renderer::handleRender()
{
	static int renderCount = 0;

	if (!m_framebuffer) {
		cerr << "No default framebuffer" << endl;
		return;
	}

	// TODO: set viewport in some principled way
	glViewport(0, 0, 600, 600);
	
	GLfloat grey = renderCount / 255.0f;
	glClearColor(grey, grey, grey, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	if (glGetError() != GL_NO_ERROR) cerr << "error before drawing" << endl;
	m_hackGeometry->draw();
	if (glGetError() != GL_NO_ERROR) cerr << "error after drawing" << endl;
	
	glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);
	if (YES != [m_context presentRenderbuffer:GL_RENDERBUFFER]) {
		cerr << "failed to present renderbuffer" << endl;
	}

	glFlush();

	if (++renderCount % 256 == 0) { renderCount = 0; }
}

void
Renderer::deleteFramebuffer()
{
	if (m_framebuffer) {
		glDeleteFramebuffers(1, &m_framebuffer);
	}
	if (m_renderbuffer) {
		glDeleteRenderbuffers(1, &m_renderbuffer);
	}
	m_framebuffer = m_renderbuffer = 0;
}
