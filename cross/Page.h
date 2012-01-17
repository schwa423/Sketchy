//
//  Page.h
//  Sketchy
//
//  Created by Joshua Gargus on 1/15/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Page_h
#define Sketchy_Page_h

#include <memory>
using std::shared_ptr;
using std::weak_ptr;

#include "Shadow.h"
#include "Event.h"
#include "shadow/Page_shadow.h"

namespace Sketchy {

	// Forward declarations for our shadow, and for initialization parameters.
	namespace Shadow { class Page; }
	class Renderer;
	class Framebuffer;
	class Page;

	typedef Shadow::Object<Page, Shadow::Page, Renderer> PageParent;

	class Page : public PageParent, public std::enable_shared_from_this<Page>
	{
	public:
		static shared_ptr<Page> New(shared_ptr<Renderer> renderer, shared_ptr<Framebuffer> framebuffer);
		void setFramebuffer(shared_ptr<Framebuffer> fb);
		const char* className() { return "Page"; }
	protected:
		shared_ptr<Framebuffer> m_framebuffer;

		// Event: set the Page's framebuffer
		class SetFramebuffer : public Event
		{
		public:
			SetFramebuffer(shared_ptr<Shadow::Page> s, shared_ptr<Shadow::Framebuffer> fb)
				: m_shadow(s), m_framebuffer(fb) { };
			virtual void reallyRun();
		protected:
			shared_ptr<Shadow::Page> m_shadow;
			shared_ptr<Shadow::Framebuffer> m_framebuffer;
		}; // class Page::SetFramebuffer

		// Shadow-object support.
		friend PageParent;
		Page(shared_ptr<Renderer> r, shared_ptr<Shadow::Page> s) : PageParent(r,s) { }

		// TODO: try to avoid
		friend Renderer;
	}; // class Page

} // namespace Sketchy

#endif
