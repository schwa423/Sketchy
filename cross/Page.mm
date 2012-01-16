//
//  Page.mm
//  Sketchy
//
//  Created by Joshua Gargus on 1/15/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <iostream>

#include "Page.h"
#include "Renderer.h"
#include "Framebuffer.h"
#include "shadow/Page_shadow.h"

namespace Sketchy {

	shared_ptr<Page>
	Page::New(shared_ptr<Renderer> renderer, shared_ptr<Framebuffer> framebuffer)
	{
		// Instantiate page and shadow, and connect them.
		shared_ptr<Page> page(new Page(renderer));
		shared_ptr<Shadow::Page> shadow(new Shadow::Page(renderer->shadow()));
		page->m_shadow = shadow;

		// Initialize the page with the hacked-in geometry and shader.
		shared_ptr<Event> init(new Shadow::Page::HackInit(shadow, framebuffer->shadow()));
		renderer->addTask(init);

		return page;
	}

	Page::~Page()
	{
		auto renderer = m_renderer.lock();
		if (!renderer) {
			cerr << "~Sketchy::Page() cannot obtain strong ref to Renderer" << endl;
			return;
		}
		shared_ptr<Event> destroy(new Shadow::Page::Destroy(m_shadow));
		renderer->addTask(destroy);
	}

} // namespace Sketchy
