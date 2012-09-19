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

#include <iostream>

#include <memory>
using std::shared_ptr;

// namespace schwa::grfx
namespace schwa {namespace grfx {


class Framebuffer : public Renderer::Resource {
    friend class Renderer;
 public:
    Framebuffer(shared_ptr<Renderer> renderer,
                GLsizei width, GLsizei height,
                GLuint color_renderbuffer,
                GLuint depth_renderbuffer);
    virtual ~Framebuffer();
    virtual void resolve() { }

 protected:
    // Called by Renderer when pushed onto Framebuffer stack.
    virtual void bind();

    // Check whether the currently-bound framebuffer is "complete".
    void checkFramebufferCompleteness();

    GLuint _framebuffer, _color, _depth;
    int _width, _height;
};


class MultisampleFramebuffer : public Framebuffer {
    friend class Renderer;
 public:
    MultisampleFramebuffer(shared_ptr<Renderer> renderer,
                           GLsizei width, GLsizei height,
                           GLuint color_renderbuffer,
                           GLuint depth_renderbuffer,
                           GLuint multisample_color_renderbuffer,
                           GLuint multisample_depth_renderbuffer);
    virtual ~MultisampleFramebuffer();
    virtual void resolve() = 0;

 protected:
    // Called by Renderer when pushed onto Framebuffer stack.
    virtual void bind();

    GLuint _multi_framebuffer, _multi_color, _multi_depth;
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__framebuffer__
