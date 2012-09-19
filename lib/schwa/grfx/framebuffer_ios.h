//
//  framebuffer_ios.h
//  schwa::grfx
//
//  Created by Josh Gargus on 9/11/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__grfx__framebuffer_ios__
#define __schwa__grfx__framebuffer_ios__

#include "framebuffer2.h"

// namespace schwa::grfx
namespace schwa {namespace grfx {

class MultisampleFramebuffer_iOS : public MultisampleFramebuffer {
 public:
    friend class Renderer_iOS;

    MultisampleFramebuffer_iOS(shared_ptr<Renderer> renderer,
                               GLsizei width, GLsizei height,
                               GLuint color_renderbuffer,
                               GLuint depth_renderbuffer,
                               GLuint multisample_color_renderbuffer,
                               GLuint multisample_depth_renderbuffer);

    virtual void resolve();
};

}}  // namespace schwa::grfx

#endif  // #ifndef __schwa__grfx__framebuffer_ios__
