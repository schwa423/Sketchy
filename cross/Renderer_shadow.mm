//
//  Renderer_shadow.mm
//  Sketchy
//
//  Created by Joshua Gargus on 1/12/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <iostream>
using std::cerr;
using std::endl;

#import "Renderer_shadow.h"
#import "Framebuffer_shadow.h"
#import "Page_shadow.h"

namespace Sketchy {
namespace Shadow {

	// TODO: pause/unpause that plumb down to pause/unpause display-link
	Renderer::Renderer(EAGLSharegroup *sharegroup) :
		Loop(),
		m_context(NULL)
	{
		cerr << "creating shadow renderer" << endl;
		m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup: sharegroup];
		Event::ptr init(new Init(this));
		addTask(init);
	}

	Renderer::~Renderer()
	{
		cerr << "destroying shadow renderer" << endl;
	}

	void
	Renderer::handleInit()
	{
		cerr << "initializing shadow renderer" << endl;

		// We only need to make the context current once.
		[EAGLContext setCurrentContext: m_context];
	}

	// TODO: change this from a placeholder to a more general scene-graph renderer.
	void
	Renderer::handleRender()
	{
		shared_ptr<Page> page;
		// TODO: would be nicer to use a lambda here, but this is temporary code anyhow
		int i = 0;
		while (!page && i < m_pages.size()) {
			page = m_pages[i].lock();
		}
		if (page) {
			page->draw();
		}
		else {
			cerr << "no page to draw" << endl;
		}
	}

} // namespace Shadow
} // namespace Sketchy
