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


shared_ptr<Renderbuffer> Renderbuffer::NewColor(shared_ptr<Renderer> renderer,
                                                int width, int height,
                                                int samples) {
    return Renderbuffer::New(renderer, width, height, samples, false);
}


shared_ptr<Renderbuffer> Renderbuffer::NewDepth(shared_ptr<Renderer> renderer,
                                                int width, int height,
                                                int samples) {
    return Renderbuffer::New(renderer, width, height, samples, true);
}


shared_ptr<Renderbuffer> Renderbuffer::New(shared_ptr<Renderer> renderer,
                                           int width, int height,
                                           int samples, bool depth) {
    GLuint handle;
    glGenRenderbuffers(1, &handle);
    glBindRenderbuffer(GL_RENDERBUFFER, handle);

    GLenum format = depth ? GL_DEPTH_COMPONENT16 : GL_RGBA8_OES;

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
                                                     samples, depth));
}


Renderbuffer::Renderbuffer(shared_ptr<Renderer> renderer, GLuint handle,
                           int width, int height,
                           int samples, bool depth) :
    Renderer::Resource(renderer), _handle(handle),
    _width(width), _height(height),
    _samples(samples), _depth(depth) {

}


Renderbuffer::~Renderbuffer() {
    // If renderer still exists, schedule deletion of OpenGL resources.
    cerr << "scheduling deletion of renderbuffer: " << _handle << endl;
    finalize([=](){
        cerr << "deleting renderbuffer: " << _handle << endl;
        glDeleteRenderbuffers(1, &_handle);
    });
}


void Renderbuffer::bind() const {
    glBindRenderbuffer(GL_RENDERBUFFER, _handle);
}


}}  // namespace schwa::grfx