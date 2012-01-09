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
							 
Framebuffer::Framebuffer(RendererPtr renderer, CAEAGLLayer *layer, bool useDepth) :
	m_renderer(renderer),
	m_shadow(new Shadow::Framebuffer(renderer)),
	m_useDepth(useDepth)
{
	CGRect rect = [layer bounds];
	m_width = (int)rect.size.width;
	m_height = (int)rect.size.height;
	
	Event::ptr task(new CreateOnscreen(renderer, m_shadow, layer, useDepth));
	renderer->addTask(task);
}
	
Framebuffer::~Framebuffer()
{
	Event::ptr task(new Destroy(m_shadow));
	m_renderer->addTask(task);
}
	
Framebuffer::CreateOnscreen::CreateOnscreen
(RendererPtr renderer, ShadowPtr shadow, CAEAGLLayer* layer, bool useDepth) :
	Renderer::RendererEvent(renderer.get()),
	m_shadow(shadow),
	m_layer(layer),
	m_useDepth(useDepth)
{ 
	m_shadow = shadow;
}

void
Framebuffer::CreateOnscreen::reallyRun() {
	m_shadow->createOnscreen(m_layer, m_useDepth);
}

} // namespace Sketchy