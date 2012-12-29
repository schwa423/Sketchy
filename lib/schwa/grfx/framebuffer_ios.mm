//
//  framebuffer_ios.mm
//  schwa::grfx
//
//  Created by Josh Gargus on 9/11/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "framebuffer_ios.h"
#include "renderbuffer_ios.h"

// namespace schwa::grfx
namespace schwa {namespace grfx {


#if defined(__APPLE__)
shared_ptr<MultisampleFramebuffer> MultisampleFramebuffer::New(const shared_ptr<Renderer>& renderer,
                                                               const shared_ptr<Framebuffer>& resolve,
                                                               bool useDepth,
                                                               bool useStencil) {
    return shared_ptr<MultisampleFramebuffer>(new MultisampleFramebuffer_iOS(renderer,
                                                                             resolve,
                                                                             useDepth,
                                                                             useStencil));
}


shared_ptr<MultisampleFramebuffer> MultisampleFramebuffer::New(const shared_ptr<Renderer>& renderer,
                                                               const shared_ptr<Attachment>& color,
                                                               bool useDepth,
                                                               bool useStencil) {
    auto fb = Framebuffer::New(renderer, color);
    return shared_ptr<MultisampleFramebuffer>(new MultisampleFramebuffer_iOS(renderer,
                                                                             fb,
                                                                             useDepth,
                                                                             useStencil));
}
#endif


shared_ptr<Framebuffer> MultisampleFramebuffer_iOS::NewFromLayer(const shared_ptr<Renderer>& renderer,
                                                                 EAGLContext* context,
                                                                 CAEAGLLayer* layer,
                                                                 bool useDepth,
                                                                 bool useStencil) {
    auto renderbuffer = Renderbuffer_iOS::NewFromLayer(renderer, context, layer);
    auto framebuffer = Framebuffer::New(renderer, renderbuffer);
    return shared_ptr<Framebuffer>(new MultisampleFramebuffer_iOS(renderer,
                                                                  framebuffer,
                                                                  useDepth,
                                                                  useStencil));
}


// Simply delegate to superclass.
MultisampleFramebuffer_iOS::MultisampleFramebuffer_iOS(const shared_ptr<Renderer>& renderer,
                                                       const shared_ptr<Framebuffer>& resolve,
                                                       bool useDepth,
                                                       bool useStencil)
    : MultisampleFramebuffer(renderer, resolve, useDepth, useStencil) {

}


void MultisampleFramebuffer_iOS::resolve() {
    _resolve->bind(GL_DRAW_FRAMEBUFFER_APPLE);
    _multi->bind(GL_READ_FRAMEBUFFER_APPLE);
    glResolveMultisampleFramebufferAPPLE();

    // For efficiency, discard rendererbuffers now that we're done with them.
	GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT };
	glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE, 2, attachments);
}


}}  // namespace schwa::grfx
