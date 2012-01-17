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

namespace Sketchy {

	// Forward declarations for our shadow, and for initialization parameters.
	namespace Shadow { class Page; }
	class Renderer;
	class Framebuffer;

	class Page : public std::enable_shared_from_this<Page>
	{
	public:
		static shared_ptr<Page> New(shared_ptr<Renderer> renderer, shared_ptr<Framebuffer> framebuffer);
		~Page();

	private:
		// Create via "Page::New()" instead of "new Page()" in order
		// to avoid the this-isn't-valid-in-constructor problem.
		Page(shared_ptr<Renderer> r) : m_renderer(r) { };

		shared_ptr<Shadow::Page> m_shadow;

		// Used only to schedule shadow Page destruction
		// within Renderer event-loop.
		weak_ptr<Renderer> m_renderer;

		// TODO: no friendz pleeze
		friend class Renderer;

	}; // class Page

} // namespace Sketchy

#endif
