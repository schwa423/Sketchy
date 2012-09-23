//
//  framebuffer_ios.mm
//  schwa::grfx
//
//  Created by Josh Gargus on 9/11/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "framebuffer_ios.h"

// namespace schwa::grfx
namespace schwa {namespace grfx {

// Simply delegate to superclass.
MultisampleFramebuffer_iOS::MultisampleFramebuffer_iOS(
                                    shared_ptr<Renderer> renderer,
                                    GLsizei width, GLsizei height,
                                    const shared_ptr<Renderbuffer>& color_renderbuffer,
                                    const shared_ptr<Renderbuffer>& depth_renderbuffer,
                                    const shared_ptr<Renderbuffer>& multisample_color_renderbuffer,
                                    const shared_ptr<Renderbuffer>& multisample_depth_renderbuffer)
    : MultisampleFramebuffer(renderer,
                             width, height,
                             color_renderbuffer,
                             depth_renderbuffer,
                             multisample_color_renderbuffer,
                             multisample_depth_renderbuffer) { }


void MultisampleFramebuffer_iOS::resolve() {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, _framebuffer);
	glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, _multi_framebuffer);
	glResolveMultisampleFramebufferAPPLE();

    // For efficiency, discard rendererbuffers now that we're done with them.
	GLenum attachments[2] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT };
	glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE, 2, attachments);
}

}}  // namespace schwa::grfx
