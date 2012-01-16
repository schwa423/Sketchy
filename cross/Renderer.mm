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
#import "Framebuffer.h"
#import "Page.h"

#import <QuartzCore/QuartzCore.h>

#include <iostream>
using std::cerr;
using std::endl;

// This is a helper class that only exists to act as
// a callback target for a CADisplayLink.  It is wholly
// owned by a Renderer instance, and never exposed.
@interface CADisplayLinkListener : NSObject {
@private
	Sketchy::Renderer *m_renderer;
	NSThread *m_thread;
	NSRunLoop *m_loop;
}
- (void) render:(CADisplayLink*)link;
- (void) start;
- (void) stop;
- (void) dealloc;
@end
@implementation CADisplayLinkListener
- (id)initWithRenderer:(Sketchy::Renderer*)renderer
{
	self = [super init];
	if (self) {
		m_renderer = renderer;
	}
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

// Can't enclose the display-link management code above, because Objective-C
// declarations must appear in the global scope.
namespace Sketchy {
	shared_ptr<Renderer>
	Renderer::New(CAEAGLLayer *layer)
	{
		// Instantiate renderer and shadow, and connect them.
		shared_ptr<Renderer> renderer(new Renderer());
		shared_ptr<Shadow::Renderer> shadow(new Shadow::Renderer([renderer->m_context sharegroup]));
		renderer->m_shadow = shadow;

		// Initialize framebuffer and shadow.
		// TODO: this is a hack... we shouldn't directly bash
		//       the shadow framebuffer into the shadow renderer.
		shared_ptr<Framebuffer> framebuffer(new Framebuffer(renderer, renderer->m_context, layer, false));
		renderer->m_defaultFramebuffer = framebuffer;
		renderer->m_shadow->hackSetDefaultFramebuffer(framebuffer->m_shadow);

		return renderer;

// TODO: clean up
//		shared_ptr<Event> init(new Shadow::Page::HackInit(shadow, framebuffer->shadow()));
//		renderer->addTask(init);
//		return page;
	}


	Renderer::Renderer() : m_context(NULL)
	{
		cerr << "creating renderer" << endl;

		// TODO: verify success
		m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		[EAGLContext setCurrentContext: m_context];

		m_displayLinkListener = [[CADisplayLinkListener alloc] initWithRenderer:this];
		// TODO: what if result was nil?

		cerr << "finished renderer initialization" << endl;
	}

	Renderer::~Renderer()
	{
		cerr << "destroying renderer" << endl;
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
	Renderer::addPage(shared_ptr<Page> page) 
	{
		m_pages.push_back(page);

		shared_ptr<Event> add(new AddPage(m_shadow, page->m_shadow));
		m_shadow->addTask(add);
	}

	// Just initialize variables.
	Renderer::AddPage::AddPage(shared_ptr<Shadow::Renderer> shadow, shared_ptr<Shadow::Page> page) : m_shadow(shadow), m_page(page)
	{

	}

	void
	Renderer::AddPage::reallyRun()
	{
		m_shadow->addPage(m_page);
	}

} // namespace Sketchy
