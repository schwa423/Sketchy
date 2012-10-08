//
//  renderbuffer.cpp
//  schwa::grfx
//
//  Created by Josh Gargus on 9/23/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "renderbuffer.h"

#include <iostream>
using std::cerr;
using std::endl;

// namespace schwa::grfx
namespace schwa {namespace grfx {


static const int RENDERBUFFER_COLOR_FORMAT = GL_RGBA8_OES;
static const int RENDERBUFFER_DEPTH_FORMAT = GL_DEPTH_COMPONENT16;
static const int RENDERBUFFER_STENCIL_FORMAT = GL_DEPTH24_STENCIL8_OES;


shared_ptr<Renderbuffer> Renderbuffer::NewColor(shared_ptr<Renderer> renderer,
                                                uint width, uint height,
                                                int samples) {
    auto rb = Renderbuffer::New(renderer, width, height, samples, RENDERBUFFER_COLOR_FORMAT);
    CHECK_GL("failed to instantiate color renderbuffer");
    return rb;
}


shared_ptr<Renderbuffer> Renderbuffer::NewDepth(shared_ptr<Renderer> renderer,
                                                uint width, uint height,
                                                int samples) {
    auto rb = Renderbuffer::New(renderer, width, height, samples, RENDERBUFFER_DEPTH_FORMAT);
    CHECK_GL("failed to instantiate depth renderbuffer");
    return rb;
}


// TODO: seems like we can only create stencil buffer using combined depth/stencil
//       format... the current APIs need to be adjusted to take this into account
shared_ptr<Renderbuffer> Renderbuffer::NewStencil(shared_ptr<Renderer> renderer,
                                                uint width, uint height,
                                                int samples) {
    auto rb =  Renderbuffer::New(renderer, width, height, samples, RENDERBUFFER_STENCIL_FORMAT);
    CHECK_GL("failed to instantiate stencil renderbuffer");
    return rb;
}


shared_ptr<Renderbuffer> Renderbuffer::New(shared_ptr<Renderer> renderer,
                                           uint width, uint height,
                                           int samples, GLenum format) {
    GLuint handle;
    glGenRenderbuffers(1, &handle);
    glBindRenderbuffer(GL_RENDERBUFFER, handle);

    if (samples == 1) {
        glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
    } else {
#if defined(__APPLE__)
        glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER,
                                              samples,
                                              format,
                                              width, height);
#else
#error multisample renderbuffers currently only supported on iOS.
#endif
    }

    if (!CHECK_GL("failed to create renderbuffer")) return nullptr;

    return shared_ptr<Renderbuffer>(new Renderbuffer(renderer, handle,
                                                     width, height,
                                                     samples, format));
}


Renderbuffer::Renderbuffer(shared_ptr<Renderer> renderer, GLuint handle,
                           uint width, uint height,
                           int samples, GLenum format)
    : Renderer::Resource(renderer), _handle(handle), _samples(samples), _format(format) {
    _width = width;
    _height = height;
}


Renderbuffer::~Renderbuffer() {
    // If renderer still exists, schedule deletion of OpenGL resources.
    // TODO: remove once Clang can capture instance variables by copy, not reference.
    auto _handle = this->_handle;
    finalize([=](){
        glDeleteRenderbuffers(1, &_handle);
    });
}


bool Renderbuffer::isColor() const {
    return RENDERBUFFER_COLOR_FORMAT == _format;
}


bool Renderbuffer::isDepth() const {
    return RENDERBUFFER_DEPTH_FORMAT == _format;
}


bool Renderbuffer::isStencil() const {
    return RENDERBUFFER_STENCIL_FORMAT == _format;
}


void Renderbuffer::bind() const {
    glBindRenderbuffer(GL_RENDERBUFFER, _handle);
}


void Renderbuffer::attach(GLenum attachmentPoint) const {
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentPoint, GL_RENDERBUFFER, _handle);
}


}}  // namespace schwa::grfx