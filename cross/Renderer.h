//
//  Renderer.h
//  Sketchy
//
//  Created by Joshua Gargus on 12/24/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

// TODO: abstract out iOS-specific code

// TODO: verify that I didn't regress in terms of proper teardown in the commit
// immediately after e17ac8b6b5f322f42929e317f15a680cf6b960c0

#ifndef Sketchy_Renderer_h
#define Sketchy_Renderer_h

#include "Event.h"
#include "Loop.h"

// TODO: remove these if unnecessary once we stop hacking around
#include "Geometry.h"
#include "Shader.h"

#import "shadow/Renderer_shadow.h"

#include <OpenGLES/ES2/gl.h>

#include <memory>
using std::shared_ptr;
using std::weak_ptr;

#include <vector>
using std::vector;

@class CADisplayLinkListener;
@class CAEAGLLayer;

namespace Sketchy {
	class Framebuffer;
	class Page;
	
	class Renderer : public std::enable_shared_from_this<Renderer> {
	public:
		static shared_ptr<Renderer> New(CAEAGLLayer *layer);
		~Renderer();

		// Schedule one render-cycle.
		void render(void) { m_shadow->render(); }

		// Schedule the task on the shadow's event-loop.
		void addTask(Event::ptr event) { m_shadow->addTask(event); }

		void pauseRendering(void);
		void unpauseRendering(void);

		// TODO: shorter type name? typedef?
		// TODO: I don't like the idea of the shadow pointer being
		//       accessible via a public API.  What to do?  Perhaps
		//       an implicit coercsion from a non-shadow to its shadow
		//       counterpart where appropriate?
		shared_ptr<Shadow::Renderer> shadow() { return m_shadow; }

		shared_ptr<Framebuffer> defaultFramebuffer() { return m_defaultFramebuffer; }

		void addPage(shared_ptr<Page> page);

	private:
		__strong EAGLContext *m_context;
		__strong CADisplayLinkListener *m_displayLinkListener;

		// Create via "Renderer::New()" instead of "new Renderer()" in order
		// to avoid the this-isn't-valid-in-constructor problem.
		Renderer();

		shared_ptr<Shadow::Renderer> m_shadow;
		shared_ptr<Framebuffer> m_defaultFramebuffer;

		// Manage a list of pages to render.
		// TODO: renderer shouldn't directly be rendering pages,
		//       Instead, cameras.
		vector<weak_ptr<Page> > m_pages;
		class AddPage : public Event {
		public:
			AddPage(shared_ptr<Shadow::Renderer> shadow, shared_ptr<Shadow::Page> page);
			virtual void reallyRun();
		private:
			shared_ptr<Shadow::Renderer> m_shadow;
			shared_ptr<Shadow::Page> m_page;
		}; // class AddPage

	}; // class Renderer

} // namespace Sketchy

#endif
