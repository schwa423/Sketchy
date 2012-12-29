//
//  renderbuffer.h
//  schwa::grfx
//
//  Created by Josh Gargus on 9/23/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__grfx__renderbuffer__
#define __schwa__grfx__renderbuffer__

#include "renderer.h"
#include "framebuffer.h"
#include "platform_gl.h"

// namespace schwa::grfx
namespace schwa {namespace grfx {

class Renderbuffer : public Renderer::Resource, public Framebuffer::Attachment {
    friend class Framebuffer;
 public:
    // "Constructors".
    // TODO: consider making these instance-methods of Renderer.
    static shared_ptr<Renderbuffer> NewColor(shared_ptr<Renderer> renderer,
                                             uint width, uint height, int samples=1);
    static shared_ptr<Renderbuffer> NewDepth(shared_ptr<Renderer> renderer,
                                             uint width, uint height, int samples=1);
    static shared_ptr<Renderbuffer> NewStencil(shared_ptr<Renderer> renderer,
                                               uint width, uint height, int samples=1);

    virtual ~Renderbuffer();

    // Bind this as the current renderbuffer.
    void bind() const;

    // Access renderbuffer properties.
    int width() const { return _width; }
    int height() const { return _height; }
    int samples() const { return _samples; }

    // What type of renderbuffer is this, color/depth/stencil?
    bool isColor() const;
    bool isDepth() const;
    bool isStencil() const;

 protected:
    static shared_ptr<Renderbuffer> New(shared_ptr<Renderer> renderer,
                                        uint width, uint height,
                                        int samples, GLenum format);

    Renderbuffer(shared_ptr<Renderer> renderer, GLuint handle,
                 uint width, uint height, int samples, GLenum format);

    // TODO: comment
    virtual void attach(GLenum attachmentPoint) const;

    // OpenGL renderbuffer ID
    GLuint _handle;

    // Number of multisamples.
    int _samples;

    // Pixel-format.
    GLenum _format;
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__renderbuffer__
