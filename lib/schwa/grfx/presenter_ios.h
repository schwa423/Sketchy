//
//  presenter_ios.h
//  schwa::grfx
//
//  Created by Josh Gargus on 10/3/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__grfx__presenter_ios__
#define __schwa__grfx__presenter_ios__

#include "presenter.h"

class Renderer;
@class EAGLContext;
@class CAEAGLLayer;

// namespace schwa::grfx
namespace schwa {namespace grfx {


class Renderbuffer_iOS;

// TODO: class description
class LayerPresenter_iOS : public Presenter {
 public:
    LayerPresenter_iOS(const shared_ptr<Renderer>& renderer, EAGLContext* context);

    // Extend superclass version to extract color renderbuffer.
    virtual void setFramebuffer(const shared_ptr<Framebuffer>& framebuffer);

 protected:
    // Uses iOS API to present renderbuffer to CoreAnimation.
    virtual void present();

    shared_ptr<Renderbuffer_iOS> _renderbuffer;
    EAGLContext* _context;

};  // class RenderbufferPresenter_iOS


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__presenter_ios__


