//
//  Framebuffer_shadow.mm
//  Sketchy
//
//  Created by Joshua Gargus on 1/8/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <QuartzCore/QuartzCore.h>
#include <iostream>

#include "Framebuffer_shadow.h"
#include "Renderer.h"

namespace Sketchy { namespace Shadow {
	
Framebuffer::Framebuffer(std::weak_ptr<Renderer> renderer, EAGLContext *context, CAEAGLLayer *layer, bool useDepth) :
m_renderer(renderer),
m_framebuffer(0),
m_color(0),
m_depth(0),
m_width(-1),
m_height(-1),
m_context(NULL)
{
	// Obtain strong reference to renderer
	auto strong = m_renderer.lock();
	if (!strong) {
		cerr << "Failed to obtain strong pointer to renderer" << endl;
		return;
	}
	// TODO: this is all very ugly.  We don't use this context in createOnscreen()...
	//       instead we use the one passed in from the non-shadow renderer (which is
	//       the currently-active OpenGL context).  That's because we're eagerly creating
	//       the framebuffer outside of the renderer loop.  This breaks our discipline of
	//       "all shadow stuff happens in the renderer loop", but on the other hand it also
	//       makes sense to do whatever work we can in the non-renderer thread (in fact, this
	//       is an explicit design goal. Ideas:
	//       - create the Framebuffer/Renderbuffers/etc in the non-shadow object, and pass
	//         them as arguments to this object
	m_context = strong->getContext();

	createOnscreen(context, layer, useDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFlush();

	cerr << "instantiated shadow framebuffer" << endl;
}

Framebuffer::~Framebuffer()
{
	cerr << "destroying shadow framebuffer" << endl;
}

void
Framebuffer::createOnscreen(EAGLContext *context, CAEAGLLayer *layer, bool useDepth) {
	deleteBuffers();

	// Obtain layer dimensions
	CGRect rect = [layer bounds];
	m_width = (int)rect.size.width;
	m_height = (int)rect.size.height;

	glGenFramebuffers(1, &m_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	glGenRenderbuffers(1, &m_color);
	glBindRenderbuffer(GL_RENDERBUFFER, m_color);

	BOOL result = [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
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
