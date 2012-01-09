//
//  Renderer.h
//  Sketchy
//
//  Created by Joshua Gargus on 12/24/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

// TODO: Renderer doesn't currently follow the "shadow" architecture, and I haven't
// yet decided whether to split it into a shadow and a non-shadow component.

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

#include <OpenGLES/ES2/gl.h>

#include <memory>

@class CADisplayLinkListener;
@class CAEAGLLayer;

namespace Sketchy {
	
	// TODO: remove these if unnecessary once we stop hacking around
	class Framebuffer;
	
	class Renderer : public Loop<Event::ptr> {
	public:
		typedef std::shared_ptr<Renderer> RendererPtr;
		
		Renderer(CAEAGLLayer *layer);
		~Renderer();

		void render(void) {
			Event::ptr render(new Render(this));
			addTask(render);
		}
		
		void pauseRendering(void);
		void unpauseRendering(void);
		
		// TODO: can we limit access to this a bit more?
		EAGLContext *getContext() { return m_context; }
		
		// Many events processed by the renderer will need a reference it.
		class RendererEvent : public Event {
		public:
			RendererEvent(Renderer *r) : m_renderer(r) { }
		protected:
			// TODO: early on, I tried to make this a shared_ptr, and I
			// don't remember what went wrong (it was either a deadlock
			// or a memory leak).  However, it seems like I should be able
			// to make it work, and thereby avoid the:
			// - unsafety of assuming that the renderer is non-null, and/or
			// - the hassle of using weak_ptr, and always testing if we can
			//   obtain a shared_ptr
			Renderer* m_renderer;
		};
		
	private:
		__strong EAGLContext *m_context;
		__strong CADisplayLinkListener *m_displayLinkListener;

		// TODO make a framebuffer class that encapsulates this state?
		GLuint m_framebuffer, m_renderbuffer;
		GLint m_framebufferWidth, m_framebufferHeight;

		Geometry *m_hackGeometry;
		Shader *m_hackShader;
		std::shared_ptr<Framebuffer> m_hackFramebuffer;
		
		void handleInit(void);		
		void handleRender(void);

		void deleteFramebuffer(void);

		class Init : public RendererEvent {
		public:
			Init(Renderer *r) : RendererEvent(r) { };
			virtual void reallyRun() { m_renderer->handleInit(); }
		};

		class Render : public RendererEvent {
		public:
			Render(Renderer *r) : RendererEvent(r) { };
			virtual void reallyRun() { m_renderer->handleRender(); }
		};
		
		
	};
} // namespace Sketchy

#endif
