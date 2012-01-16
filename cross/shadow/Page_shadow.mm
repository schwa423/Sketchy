//
//  Page_shadow.mm
//  Sketchy
//
//  Created by Joshua Gargus on 1/15/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <iostream>

#include "Page_shadow.h"

// TODO: remove these if unnecessary once we stop hacking around
#include "Geometry.h"
#include "Shader.h"
#include "Framebuffer_shadow.h"

namespace Sketchy {
namespace Shadow {

	Page::Page(shared_ptr<Renderer> r) : m_renderer(r), m_geometry(NULL), m_shader(NULL)
	{

	}

	// TODO: assert that this destructor always happens in the Renderer loop.
	Page::~Page() 
	{
		if (m_geometry) {
			delete m_geometry;
			m_geometry = NULL;
		}

		if (m_shader) {
			delete m_shader;
			m_shader = NULL;
		}
	}

	void
	Page::handleHackInit(shared_ptr<Framebuffer> framebuffer)
	{
		m_framebuffer = framebuffer;
		m_geometry = new Geometry();
		m_shader = new Shader();
	}

	void
	Page::draw()
	{
		static int renderCount = 0;
		if (++renderCount % 256 == 0) { renderCount = 0; }

		if (!m_framebuffer || !m_geometry || !m_shader) return;

		m_framebuffer->bind();

		glViewport(0, 0, 600, 600);

		GLfloat grey = renderCount / 255.0f;
		glClearColor(grey, grey, grey, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		if (glGetError() != GL_NO_ERROR) cerr << "error before drawing" << endl;
		m_shader->bind();
		m_geometry->draw();
		if (glGetError() != GL_NO_ERROR) cerr << "error after drawing" << endl;

		glFlush();
		m_framebuffer->present();
	}

	Page::HackInit::HackInit(shared_ptr<Page> p, shared_ptr<Framebuffer> fb) :
		m_page(p), m_framebuffer(fb)
	{

	}

	void
	Page::HackInit::reallyRun() {
		m_page->handleHackInit(m_framebuffer);
	}

	Page::Destroy::Destroy(shared_ptr<Page> p) : m_page(p)
	{

	}

	void
	Page::Destroy::reallyRun()
	{
		// Sanity check.
		if (m_page.use_count() != 1) {
			cerr << "WTF... Shadow::Page.use_count() != 1" << endl;
		}
	}

} // namespace Shadow
} // namespace Sketchy
