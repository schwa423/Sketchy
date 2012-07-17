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

#include "Shadow.h"
#include "Renderer_shadow.h"
#include "Event.h"

namespace Sketchy {
	class Geometry;
	class Shader;
	class Page;
	class Renderer;

namespace Shadow {
	class Framebuffer;

	// Localized verbosity.
	typedef Shadow::Object<Sketchy::Page, Page, Sketchy::Renderer> PageParent;
	typedef Shadow::ShadowObject<Sketchy::Page, Page, Sketchy::Renderer> PageShadowParent;

	class Page : public PageShadowParent, public std::enable_shared_from_this<Page>
	{
	public:
		virtual ~Page();
		const char* className() { return "Shadow::Page"; }

		// TODO: this hack is a place holder for a camera/scene/framebuffer combo...
		//       once everything is working, reify those concepts.
		void draw();

		void setFramebuffer(shared_ptr<Framebuffer> fb);

	protected:
		Page();
		shared_ptr<Framebuffer> m_framebuffer;

		// Shadow-object support.
		friend PageParent;

		// Sent by Shadow::Object::New()
		virtual Event* Init(shared_ptr<Sketchy::Renderer> renderer, shared_ptr<Sketchy::Page> page) {
			return new InitEvent(shared_from_this());
		};
		// TODO: can we avoid this boilerplate?
		void handleInit();
		class InitEvent : public Event {
		public:
			InitEvent(shared_ptr<Page> page) : m_page(page) { };
			virtual void reallyRun() { m_page->handleInit(); }
		protected:
			shared_ptr<Page> m_page;
		};

		// TODO: use shared_ptr, but more importantly don't just hard-code these into the page.
		Geometry *m_geometry;
		Shader *m_shader;

	}; // class Page

} // namespace Shadow
} // namespace Sketchy

#endif
