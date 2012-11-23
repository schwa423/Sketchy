//
//  framebuffer.cpp
//  schwa::grfx
//
//  Created by Josh Gargus on 9/8/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "framebuffer.h"
#include "renderbuffer.h"
#include "platform_gl.h"

#include <iostream>
using std::cerr;
using std::endl;

// namespace schwa::grfx
namespace schwa {namespace grfx {


shared_ptr<Framebuffer> Framebuffer::New(const shared_ptr<Renderer>& renderer,
                                         shared_ptr<Attachment> color,
                                         shared_ptr<Attachment> depth,
                                         shared_ptr<Attachment> stencil) {
    return shared_ptr<Framebuffer>(new BasicFramebuffer(renderer,
                                                        color,
                                                        depth,
                                                        stencil));
}


// We currently only implement multisample framebuffers on iOS.
// This is an explicit reminder for the first port to another platform.
#if !defined(__APPLE__)
#error MultisampleFramebuffer::New() only implemented for __APPLE__
#endif


MultisampleFramebuffer::MultisampleFramebuffer(const shared_ptr<Renderer>& renderer,
                                               const shared_ptr<Framebuffer>& resolve,
                                               bool useDepth, bool useStencil) {
    // Set our dimensions to match the "resolve" framebuffer.
    _width = resolve->width();
    _height = resolve->height();
    setFramebuffer(resolve);

    // Create color, and optionally depth and stencil renderbuffers
    // whose dimensions match that of the "resolve" framebuffer.
    shared_ptr<Renderbuffer> color, depth, stencil;
    color = Renderbuffer::NewColor(renderer, _width, _height, 4);
    if (useDepth)
        depth = Renderbuffer::NewDepth(renderer, _width, _height, 4);
    if (useStencil)
        stencil = Renderbuffer::NewStencil(renderer, _width, _height, 4);

    // Create new framebuffer with the new renderbuffers attached.
    _multi.reset(new BasicFramebuffer(renderer, color, depth, stencil));
}


void MultisampleFramebuffer::setFramebuffer(const shared_ptr<Framebuffer>& resolve) {
    if (resolve == _resolve) return;

    if (_width != resolve->width() || _height != resolve->height()) {
        // TODO: logging framework
        cerr << "ERROR: MultisampleFramebuffer: cannot change to target of different size" << endl;
        return;
    }

    _resolve = resolve;
}


BasicFramebuffer::BasicFramebuffer(const shared_ptr<Renderer>& renderer,
                                   shared_ptr<Framebuffer::Attachment> color,
                                   shared_ptr<Framebuffer::Attachment> depth,
                                   shared_ptr<Framebuffer::Attachment> stencil)
    : Renderer::Resource(renderer), _fb(0), _color(color), _depth(depth), _stencil(stencil) {

    _width = _color->width();
    _height = _color->height();

    // Ensure that all attachments have same dimensions.
    if (_depth && (_width != _depth->width() || _height != _depth->height())) {
        cerr << "ERROR: BasicFramebuffer: depth-buffer dimensions don't match ("
             << _width << "/" << _height << " vs. "
             << _depth->width() << "/" << _depth->height() << ")" << endl;
        _depth.reset();
    }
    if (_stencil && (_width != _stencil->width() || _height != _stencil->height())) {
        cerr << "ERROR: BasicFramebuffer: stencil-buffer dimensions don't match ("
             << _width << "/" << _height << " vs. "
             << _stencil->width() << "/" << _stencil->height() << ")" << endl;
        _stencil.reset();
    }

    glGenFramebuffers(1, &_fb);
    glBindFramebuffer(GL_FRAMEBUFFER, _fb);

    _color->attach(GL_COLOR_ATTACHMENT0);
    if (_depth) _depth->attach(GL_DEPTH_ATTACHMENT);
    if (_stencil) _stencil->attach(GL_STENCIL_ATTACHMENT);

    cerr << "Checking Framebuffer completeness..." << endl;
    checkFramebufferCompleteness();
}


BasicFramebuffer::~BasicFramebuffer() {
    // If renderer still exists, schedule deletion of OpenGL resources.
    // TODO: remove once Clang can capture instance variables by copy, not reference.
    auto _fb = this->_fb;
    finalize([=](){
        glDeleteFramebuffers(1, &_fb);
    });
}


void BasicFramebuffer::bind(GLenum target) {
    glBindFramebuffer(target, _fb);
}


void BasicFramebuffer::checkFramebufferCompleteness() {
    // Check that everything is OK
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        // TODO: need logging library
        cerr << "Failed to make complete framebuffer object: " << status << endl;
    }
}


}}  // namespace schwa::grfx