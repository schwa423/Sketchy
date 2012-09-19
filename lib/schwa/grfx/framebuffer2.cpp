//
//  framebuffer.cpp
//  schwa::grfx
//
//  Created by Josh Gargus on 9/8/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "framebuffer2.h"
#include "platform_gl.h"

#include <iostream>
using std::cerr;
using std::endl;

// namespace schwa::grfx
namespace schwa {namespace grfx {


Framebuffer::Framebuffer(shared_ptr<Renderer> renderer,
                         GLsizei width, GLsizei height,
                         GLuint color_renderbuffer,
                         GLuint depth_renderbuffer) :
    Resource(renderer),
    _width(width), _height(height),
    _color(color_renderbuffer),
    _depth(depth_renderbuffer)
{
    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _color);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth);
    cerr << "Checking Framebuffer completeness..." << endl;
    checkFramebufferCompleteness();
}


Framebuffer::~Framebuffer() {
    // If renderer still exists, schedule deletion of OpenGL resources.
    finalize([=](){
        glDeleteFramebuffers(1, &_framebuffer);
        glDeleteRenderbuffers(1, &_color);
        glDeleteRenderbuffers(1, &_depth);
    });
}


void Framebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
}


void Framebuffer::checkFramebufferCompleteness() {
    // Check that everything is OK
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		// TODO: need logging library
		cerr << "Failed to make complete framebuffer object: " << status << endl;
	}
}


MultisampleFramebuffer::MultisampleFramebuffer(shared_ptr<Renderer> renderer,
                                               GLsizei width, GLsizei height,
                                               GLuint color_renderbuffer,
                                               GLuint depth_renderbuffer,
                                               GLuint multisample_color_renderbuffer,
                                               GLuint multisample_depth_renderbuffer) :
    Framebuffer(renderer, width, height, color_renderbuffer, depth_renderbuffer),
    _multi_color(multisample_color_renderbuffer),
    _multi_depth(multisample_depth_renderbuffer)
{
    glGenFramebuffers(1, &_multi_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _multi_framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _multi_color);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _multi_depth);
    cerr << "Checking multisample Framebuffer completeness..." << endl;
    checkFramebufferCompleteness();
}


MultisampleFramebuffer::~MultisampleFramebuffer() {
    // If renderer still exists, schedule deletion of OpenGL resources.
    finalize([=](){
        glDeleteFramebuffers(1, &_multi_framebuffer);
        glDeleteRenderbuffers(1, &_multi_color);
        glDeleteRenderbuffers(1, &_multi_depth);
    });
}


void MultisampleFramebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, _multi_framebuffer);
}


}}  // namespace schwa::grfx