//
//  Framebuffer_shadow.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 1/8/12.
//  Copyright (c) 2012 Schwafwarez. All rights reserved.
//

#include <QuartzCore/QuartzCore.h>
#include <iostream>

#include "Framebuffer_shadow.h"
#include "Renderer.h"

namespace Sketchy { namespace Shadow {
	
Framebuffer::Framebuffer(std::shared_ptr<Renderer> renderer) :
m_renderer(renderer),
m_framebuffer(0),
m_color(0),
m_depth(0),
m_width(-1),
m_height(-1),
m_context(NULL)
{
	cerr << "instantiated shadow framebuffer" << endl;
}

void
Framebuffer::createOnscreen(CAEAGLLayer *layer, bool useDepth) {
	deleteBuffers();

	// Obtain strong reference to renderer
	std::shared_ptr<Renderer> renderer(m_renderer);
	if (!renderer.get()) {
		cerr << "Failed to obtain strong pointer to renderer" << endl;
		return;
	}
	
	glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	glGenRenderbuffers(1, &m_color);
	glBindRenderbuffer(GL_RENDERBUFFER, m_color);

	m_context = renderer->getContext();
	BOOL result = [m_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
	if (result != YES) {
		cerr << "Failed to allocate render-buffer storage from CAEAGLLayer" << endl;
		deleteBuffers();
		return;
	}
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_color);
	
	// TODO: deal with depth
	if (useDepth) {
		cerr << "Shadow::Framebuffer does not handle depth yet" << endl;
	}

	// Check that everything is OK
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		// TODO: need logging library
		cerr << "Failed to make complete framebuffer object: " << status << endl;
		deleteBuffers();
	}
	
	// Read back width and height of renderbuffer.
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &m_width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &m_height);
	
	cerr << "successfully initialized framebuffer with dimensions: " << m_width << " " << m_height << endl;
}

void 
Framebuffer::deleteBuffers() {
	if (m_framebuffer) glDeleteFramebuffers(1, &m_framebuffer);
	if (m_color) glDeleteRenderbuffers(1, &m_color);
	if (m_depth) glDeleteRenderbuffers(1, &m_depth);

	m_framebuffer = m_color = m_depth = 0;
	m_width = m_height = -1;			
}
	
void
Framebuffer::bind() {
	if (!m_framebuffer) {
		cerr << "invalid framebuffer" << endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
}

void
Framebuffer::present() {
	if (!m_color || !m_context) {
		cerr << "color and/or context not ready to present renderbuffer" << endl;
		return;
	}
	
	glBindRenderbuffer(GL_RENDERBUFFER, m_color);
	if (YES != [m_context presentRenderbuffer:GL_RENDERBUFFER]) {
		cerr << "failed to present renderbuffer" << endl;
	}
}
	
}} // namespace Sketchy::Shadow
