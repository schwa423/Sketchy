//
//  framebuffer.h
//  schwa::grfx
//
//  Created by Josh Gargus on 9/8/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__grfx__framebuffer__
#define __schwa__grfx__framebuffer__

#include "renderer2.h"
#include "renderbuffer.h"

#include <iostream>

#include <memory>
using std::shared_ptr;

// namespace schwa::grfx
namespace schwa {namespace grfx {


class Framebuffer : public Renderer::Resource {
    friend class Renderer;
 public:
    // TODO: not sure I like passing in the width/height.
    Framebuffer(shared_ptr<Renderer> renderer,
                GLsizei width, GLsizei height,
                const shared_ptr<Renderbuffer>& color_renderbuffer,
                const shared_ptr<Renderbuffer>& depth_renderbuffer);

    virtual ~Framebuffer();
    virtual void resolve() { }

 protected:
    // Called by Renderer when pushed onto Framebuffer stack.
    virtual void bind();

    GLuint _framebuffer;
    int _width, _height;

    shared_ptr<Renderbuffer> _color;
    shared_ptr<Renderbuffer> _depth;

    // Check whether the currently-bound framebuffer is "complete".
    void checkFramebufferCompleteness();

    // Attach renderbuffer to the currently-bound framebuffer
    // at the specified attachment point.
    void attach(const shared_ptr<Renderbuffer>& renderbuffer, GLenum attachment);
};


class MultisampleFramebuffer : public Framebuffer {
    friend class Renderer;
 public:
    // TODO: not sure I like passing in the width/height.
    MultisampleFramebuffer(shared_ptr<Renderer> renderer,
                           GLsizei width, GLsizei height,
                           const shared_ptr<Renderbuffer>& color_renderbuffer,
                           const shared_ptr<Renderbuffer>& depth_renderbuffer,
                           const shared_ptr<Renderbuffer>& multisample_color_renderbuffer,
                           const shared_ptr<Renderbuffer>& multisample_depth_renderbuffer);

    virtual ~MultisampleFramebuffer();
    virtual void resolve() = 0;

 protected:
    // Called by Renderer when pushed onto Framebuffer stack.
    virtual void bind();

    GLuint _multi_framebuffer;
    shared_ptr<Renderbuffer> _multi_color;
    shared_ptr<Renderbuffer> _multi_depth;
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__framebuffer__
