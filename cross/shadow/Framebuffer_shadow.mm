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
m_resolve(0),
m_resolveColor(0),
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
	// TODO: delete the buffers, yo.
	cerr << "destroying shadow framebuffer" << endl;
}

void
Framebuffer::createOnscreen(EAGLContext *context, CAEAGLLayer *layer, bool useDepth) {
	deleteBuffers();

	// Obtain layer dimensions
	CGRect rect = [layer bounds];
	m_width = (int)rect.size.width;
	m_height = (int)rect.size.height;

	GLenum status;

	glGenFramebuffers(1, &m_multi);
	glBindFramebuffer(GL_FRAMEBUFFER, m_multi);
	glGenRenderbuffers(1, &m_multiColor);
	glBindRenderbuffer(GL_RENDERBUFFER, m_multiColor);
	glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, 4, GL_RGBA8_OES, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_multiColor);

	// TODO: deal with depth
	if (useDepth) {
		cerr << "Shadow::Framebuffer does not handle depth yet" << endl;
	}

	// Check that everything is OK
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		// TODO: need logging library
		cerr << "Failed to make complete framebuffer object: " << status << endl;
		deleteBuffers();
	}


	glGenFramebuffers(1, &m_resolve);
	glBindFramebuffer(GL_FRAMEBUFFER, m_resolve);
	glGenRenderbuffers(1, &m_resolveColor);
	glBindRenderbuffer(GL_RENDERBUFFER, m_resolveColor);

	BOOL result = [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
	if (result != YES) {
		cerr << "Failed to allocate render-buffer storage from CAEAGLLayer" << endl;
		deleteBuffers();
		return;
	}
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_resolveColor);

	// Check that everything is OK
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
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
	if (m_multi) glDeleteFramebuffers(1, &m_multi);
	if (m_multiColor) glDeleteRenderbuffers(1, &m_multiColor);
	if (m_depth) glDeleteRenderbuffers(1, &m_depth);

	if (m_resolve) glDeleteFramebuffers(1, &m_resolve);
	if (m_resolveColor) glDeleteRenderbuffers(1, &m_resolveColor);

	m_resolve = m_resolveColor = m_depth = 0;
	m_width = m_height = -1;			
}

void
Framebuffer::bind() {
	if (!m_multi) {
		cerr << "invalid framebuffer" << endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, m_multi);
}

void
Framebuffer::present() {
	if (!m_resolveColor || !m_context) {
		cerr << "color and/or context not ready to present renderbuffer" << endl;
		return;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, m_resolve);
	glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, m_multi);
	glResolveMultisampleFramebufferAPPLE();

	// For efficiency, discard rendererbuffers now that we're done with them.
	GLenum e = GL_COLOR_ATTACHMENT0;
	glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE,1,&e);
	if (m_depth) {
		e = GL_DEPTH_ATTACHMENT;
		glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE,1,&e);
	}

	glBindRenderbuffer(GL_RENDERBUFFER, m_resolveColor);
	if (YES != [m_context presentRenderbuffer:GL_RENDERBUFFER]) {
		cerr << "failed to present renderbuffer" << endl;
	}
}

}} // namespace Sketchy::Shadow
