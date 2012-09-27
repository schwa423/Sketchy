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
                         const shared_ptr<Renderbuffer>& color_renderbuffer,
                         const shared_ptr<Renderbuffer>& depth_renderbuffer) :
    Resource(renderer),
    _width(width), _height(height),
    _color(color_renderbuffer),
    _depth(depth_renderbuffer)
{
    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

    attach(_color, GL_COLOR_ATTACHMENT0);
    if (_depth) attach(_depth, GL_DEPTH_ATTACHMENT);

    cerr << "Checking Framebuffer completeness..." << endl;
    checkFramebufferCompleteness();
}


Framebuffer::~Framebuffer() {
    // If renderer still exists, schedule deletion of OpenGL resources.
    // TODO: remove once Clang can capture instance variables by copy, not reference.
    auto _framebuffer = this->_framebuffer;
    finalize([=](){
        glDeleteFramebuffers(1, &_framebuffer);
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


void Framebuffer::attach(const shared_ptr<Renderbuffer>& renderbuffer, GLenum attachment) {
    if (!renderbuffer) return;
    if (_width != renderbuffer->width() || _height != renderbuffer->height())
        cerr << "ERROR: framebuffer and renderbuffer dimensions do not match" << endl;
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer->_handle);
}


MultisampleFramebuffer::MultisampleFramebuffer(shared_ptr<Renderer> renderer,
                                               GLsizei width, GLsizei height,
                                               const shared_ptr<Renderbuffer>& color_rbuffer,
                                               const shared_ptr<Renderbuffer>& depth_rbuffer,
                                               const shared_ptr<Renderbuffer>& multi_color_rbuffer,
                                               const shared_ptr<Renderbuffer>& multi_depth_rbuffer) :
    Framebuffer(renderer, width, height, color_rbuffer, depth_rbuffer),
    _multi_color(multi_color_rbuffer),
    _multi_depth(multi_depth_rbuffer)
{
    glGenFramebuffers(1, &_multi_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _multi_framebuffer);

    attach(_multi_color, GL_COLOR_ATTACHMENT0);
    if (_multi_depth) attach(_multi_depth, GL_DEPTH_ATTACHMENT);

    cerr << "Checking multisample Framebuffer completeness..." << endl;
    checkFramebufferCompleteness();
}


MultisampleFramebuffer::~MultisampleFramebuffer() {
    // If renderer still exists, schedule deletion of OpenGL resources.
    // TODO: remove once Clang can capture instance variables by copy, not reference.
    auto _multi_framebuffer = this->_multi_framebuffer;
    finalize([=](){
        glDeleteFramebuffers(1, &_multi_framebuffer);
    });
}


void MultisampleFramebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, _multi_framebuffer);
}


}}  // namespace schwa::grfx