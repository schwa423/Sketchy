//
//  renderbuffer.h
//  schwa::grfx
//
//  Created by Josh Gargus on 9/23/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__grfx__renderbuffer__
#define __schwa__grfx__renderbuffer__

#include "renderer2.h"
#include "platform_gl.h"

// namespace schwa::grfx
namespace schwa {namespace grfx {


class Framebuffer;

class Renderbuffer : public Renderer::Resource {
    friend class Framebuffer;
 public:
    // "Constructors".
    // TODO: consider making these instance-methods of Renderer.
    static shared_ptr<Renderbuffer> NewColor(shared_ptr<Renderer> renderer,
                                             int width, int height, int samples=1);
    static shared_ptr<Renderbuffer> NewDepth(shared_ptr<Renderer> renderer,
                                             int width, int height, int samples=1);

    virtual ~Renderbuffer();

    // Bind this as the current renderbuffer.
    void bind() const;

    // Access renderbuffer properties.
    int width() const { return _width; }
    int height() const { return _height; }
    int samples() const { return _samples; }

    // What type of renderbuffer is this, color or depth?
    bool isDepth() const { return _depth; }
    bool isColor() const { return !_depth; }

 protected:
    static shared_ptr<Renderbuffer> New(shared_ptr<Renderer> renderer,
                                        int width, int height,
                                        int samples, bool depth);

    Renderbuffer(shared_ptr<Renderer> renderer, GLuint handle,
                 int width, int height, int samples, bool depth);

    // OpenGL renderbuffer ID
    GLuint _handle;

    // Dimensions of renderbuffer.
    int _width;
    int _height;

    // Number of multisamples.
    int _samples;

    // Is this a depth-buffer?  If not, it's a color-buffer.
    bool _depth;
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__renderbuffer__
