//
//  framebuffer.h
//  schwa::grfx
//
//  Created by Josh Gargus on 9/8/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__grfx__framebuffer__
#define __schwa__grfx__framebuffer__

#include "renderer.h"

#include <iostream>

#include <memory>
using std::shared_ptr;

// namespace schwa::grfx
namespace schwa {namespace grfx {

// Framebuffer interface.
class Framebuffer {
 public:
    // Forward declaration of Framebuffer::Attachment... see below.
    class Attachment;

    // TODO: Revisit framebuffer-creation methods.  Would be nice if they were all in one place?
    //       Maybe, but MultisampleFramebuffer::New() is terse and expressive.  In any case, make
    //       the arguments to the various methods as similar as possible.
    static shared_ptr<Framebuffer> New(const shared_ptr<Renderer>& renderer,
                                       shared_ptr<Attachment> color,
                                       shared_ptr<Attachment> depth = nullptr,
                                       shared_ptr<Attachment> stencil = nullptr);

    uint width() const { return _width; }
    uint height() const { return _height; }

    // Called by Renderer when pushed onto Framebuffer stack.
    // TODO: do we want to enforce this by making this protected,
    //       and making Renderer a friend?
    virtual void bind(GLenum target = GL_FRAMEBUFFER) = 0;

    // No-op... only meaningful for multisample framebuffers,
    // but providing it here in the base class simplifies client code.
    virtual void resolve() { }

 protected:
    // Use static Framebuffer::New().
    Framebuffer() { }

    uint _width;
    uint _height;

 public:
    // Base-class for "framebuffer attachments", which can be either
    // renderbuffers, or a (specific mip-level of a) texture.
    class Attachment {
     public:
        Attachment() : _width(0), _height(0) { }

        virtual void attach(GLenum attachmentPoint) const = 0;

        uint width() const { return _width; }
        uint height() const { return _height; }

     protected:
        uint _width;
        uint _height;
    };
};


// MultisampleFramebuffer encapsulates two framebuffers, one with multisample
// renderbuffers attached (arbitrary content is renderered into this one), and
// one without.  The first is resolved into the second, which is then used to
// display the contents (either directly into a native OS surface, or as a texture
// used for later rendering.
//
// The renderbuffers bound to the resolve and multisample framebuffers do not
// need to match.  For example, the view being rendered might be require depth-
// culling to be enabled, and the multisample framebuffer must therefore have a
// depth attachment.  However, a typical use-case is to render into a texture;
// this is accomplished by attaching the texture as the "resolve" framebuffer's
// color attachment... there is not reason to spend the memory/cycles to also
// resolve the depth buffer if it will not be used.
// TODO: I'm not even sure if glResolveMultisampleFramebufferAPPLE() will resolve
//       a multisample depth buffer into the resolve buffer... would this even make
//       sense?  (would the downsampled depth values be useful, or detrimental?)
//
// TODO: can the "resolve" framebuffer be any type of Framebuffer, or must it
//       be a BasicFramebuffer.  If the latter, we should either make it work
//       both ways, or enforce this in the method interface.
//
// TODO: (this is maybe silly) Is it possible to use a non-multisample depth
//       buffer in _multi?  What are the costs in quality (and others)?  What
//       are the gains in memory-usage and cycles saved?
class MultisampleFramebuffer : public Framebuffer {
 public:
    static shared_ptr<MultisampleFramebuffer> New(const shared_ptr<Renderer>& renderer,
                                                  const shared_ptr<Framebuffer>& resolve,
                                                  bool useDepth = false,
                                                  bool useStencil = false);

    static shared_ptr<MultisampleFramebuffer> New(const shared_ptr<Renderer>& renderer,
                                                  const shared_ptr<Attachment>& color,
                                                  bool useDepth = false,
                                                  bool useStencil = false);

    // It's OK to change the target framebuffer, as long as it
    // has the same dimensions as the original.
    void setFramebuffer(const shared_ptr<Framebuffer>& resolve);

    // Called by Renderer when pushed onto Framebuffer stack.
    // TODO: Do we want to enforce this by making this protected,
    //       and making Renderer a friend?
    // TODO: Does any use case require _resolve to be bound instead?
    //       Could the user become confused?  If so, how can we better guide them?
    virtual void bind(GLenum target = GL_FRAMEBUFFER) { return _multi->bind(target); };

    // Resolving multisample framebuffer target is a platform-specific operation.
    virtual void resolve() = 0;

 protected:
    // Use static MultisampleFramebuffer::New().
    MultisampleFramebuffer(const shared_ptr<Renderer>& renderer,
                           const shared_ptr<Framebuffer>& resolve,
                           bool useDepth = false,
                           bool useStencil = false);

    // The framebuffer that the multisample-framebuffer's contents are resolved into.
    shared_ptr<Framebuffer> _resolve;

    // The multisample framebuffer that is rendered into.
    shared_ptr<Framebuffer> _multi;
};


// TODO: describe this class
class BasicFramebuffer : public Framebuffer, public Renderer::Resource {
 public:
    BasicFramebuffer(const shared_ptr<Renderer>& renderer,
                     shared_ptr<Attachment> color,
                     shared_ptr<Attachment> depth = nullptr,
                     shared_ptr<Attachment> stencil = nullptr);

    virtual ~BasicFramebuffer();

    // Called by Renderer when pushed onto Framebuffer stack.
    // TODO: do we want to enforce this by making this protected,
    //       and making Renderer a friend?
    virtual void bind(GLenum target = GL_FRAMEBUFFER);

 protected:
    // Check whether the currently-bound framebuffer is "complete".
    void checkFramebufferCompleteness();

    GLuint _fb;
    shared_ptr<Attachment> _color;
    shared_ptr<Attachment> _depth;
    shared_ptr<Attachment> _stencil;
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__framebuffer__
