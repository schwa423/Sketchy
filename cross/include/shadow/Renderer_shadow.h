//
//  Renderer_shadow.h
//  Sketchy
//
//  Created by Joshua Gargus on 1/12/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Renderer_shadow_h
#define Sketchy_Renderer_shadow_h

// TODO: these should probably be in Sketchy namespace
#include "Event.h"
#include "Loop.h"

#include <OpenGLES/ES2/gl.h>

#include <memory>
using std::shared_ptr;
using std::weak_ptr;

#include <vector>
using std::vector;

@class CAEAGLLayer;

namespace Sketchy {
namespace Shadow {

	// TODO: remove these if unnecessary once we stop hacking around
	class Framebuffer;
	class Page;

	class Renderer :
		public Loop<Event::ptr>,
	public std::enable_shared_from_this<Renderer>
	{
	public:
		Renderer(EAGLSharegroup *sharegroup);
		~Renderer();

		void render(void) {
			Event::ptr render(new Render(this));
			addTask(render);
		}

		void addPage(const shared_ptr<Page> &page) { m_pages.push_back(page); }

		// TODO: can we limit access to this a bit more?
		EAGLContext *getContext() { return m_context; }

		void hackSetDefaultFramebuffer(shared_ptr<Framebuffer> fb) { m_defaultFramebuffer = fb; }

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

		vector<weak_ptr<Page>> m_pages;

		shared_ptr<Framebuffer> m_defaultFramebuffer;

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

} // namespace Shadow
} // namespace Sketchy


#endif
