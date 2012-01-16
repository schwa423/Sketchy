//
//  Framebuffer.mm
//  Sketchy
//
//  Created by Joshua Gargus on 1/6/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <iostream>

#include <QuartzCore/QuartzCore.h>

#include "Framebuffer.h"
#include "Renderer.h"

namespace Sketchy {
							 
Framebuffer::Framebuffer(shared_ptr<Renderer> renderer, EAGLContext *context, CAEAGLLayer *layer, bool useDepth) :
	m_renderer(renderer),
	m_shadow(new Shadow::Framebuffer(renderer->shadow(), context, layer, useDepth)),
	m_useDepth(useDepth)
{
	CGRect rect = [layer bounds];
	m_width = (int)rect.size.width;
	m_height = (int)rect.size.height;

}
	
Framebuffer::~Framebuffer()
{
	cerr << "destroying Framebuffer by enqueuing task on Renderer" << endl;

	auto renderer = m_renderer.lock();
	if (!renderer) {
		cerr << "~Sketchy::Framebuffer() cannot obtain strong ref to Renderer" << endl;
		return;
	}
	Event::ptr task(new Destroy(m_shadow));
	renderer->addTask(task);
}
	
} // namespace Sketchy
