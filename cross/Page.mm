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
		auto page = PageParent::New(renderer);
		page->setFramebuffer(framebuffer);
		return page;
	}

	void
	Page::setFramebuffer(shared_ptr<Framebuffer> fb)
	{
		if (m_framebuffer == fb) return; // no change
		m_framebuffer = fb;

		// TODO: better way than public accessor to obtain framebuffer's shadow?
		shared_ptr<Event> evt(new SetFramebuffer(m_shadow, fb->shadow()));
		m_loop->addTask(evt);
	}

	void
	Page::SetFramebuffer::reallyRun()
	{
		m_shadow->setFramebuffer(m_framebuffer);
	}

} // namespace Sketchy
