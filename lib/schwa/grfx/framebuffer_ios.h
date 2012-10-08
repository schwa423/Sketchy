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

@class CAEAGLLayer;

// namespace schwa::grfx
namespace schwa {namespace grfx {


// TODO: can we do away with this class somehow, since its whole
//       reason for existance is to override resolve().  Just use #ifdefs somehow?
class MultisampleFramebuffer_iOS : public MultisampleFramebuffer {
 public:
    friend class Renderer_iOS;

    static shared_ptr<Framebuffer> NewFromLayer(const shared_ptr<Renderer>& renderer,
                                                EAGLContext* context,
                                                CAEAGLLayer* layer,
                                                bool useDepth = false,
                                                bool useStencil = false);

    MultisampleFramebuffer_iOS(const shared_ptr<Renderer>& renderer,
                               const shared_ptr<Framebuffer>& resolve,
                               bool useDepth = false,
                               bool useStencil = false);

    virtual void resolve();
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__framebuffer_ios__
