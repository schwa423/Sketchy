//
//  Page_shadow.h
//  Sketchy
//
//  Created by Joshua Gargus on 1/15/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Page_shadow_h
#define Sketchy_Page_shadow_h

#include <memory>
using std::shared_ptr;
using std::weak_ptr;

#include "Renderer_shadow.h"
#include "Event.h"

namespace Sketchy {
	class Geometry;
	class Shader;

namespace Shadow {
	class Framebuffer;

	class Page {
	public:
		Page(shared_ptr<Renderer> r);
		~Page();

		// TODO: this hack is a place holder for a camera/scene/framebuffer combo...
		//       once everything is working, reify those concepts.
		void draw();

		// TODO: this is not the final word on how to set up a Page.
		void handleHackInit(shared_ptr<Framebuffer> fb);
		class HackInit : public Event {
		public:
			HackInit(shared_ptr<Page> p, shared_ptr<Framebuffer> fb);
			virtual void reallyRun();
		private:
			shared_ptr<Page> m_page;
			shared_ptr<Framebuffer> m_framebuffer;
		};

		// TODO: shoudldn't we put these events in the non-shadow class?
		// Does nothing but hold a reference to the Page long enough 
		// to release it in the Renderer event-loop.
		class Destroy : public Event {
		public:
			Destroy(shared_ptr<Page> p);
			virtual void reallyRun();
		private:
			shared_ptr<Page> m_page;
		}; // class Destroy

	protected:
		weak_ptr<Renderer> m_renderer;
		shared_ptr<Framebuffer> m_framebuffer;

		// TODO: use shared_ptr, but more importantly don't just hard-code these into the page.
		Geometry *m_geometry;
		Shader *m_shader;

	}; // class Page

} // namespace Shadow
} // namespace Sketchy

#endif
