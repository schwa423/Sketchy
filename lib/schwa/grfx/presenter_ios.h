//
//  presenter_ios.h
//  schwa::grfx
//
//  Created by Josh Gargus on 10/3/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
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


class Renderbuffer;

// TODO: class description
class LayerPresenter_iOS : public Presenter {
 public:
    LayerPresenter_iOS(const shared_ptr<Renderer>& renderer,
                       EAGLContext* renderContext,
                       EAGLContext* defaultContext);

    void setLayer(CAEAGLLayer* layer);

 protected:
    // Uses iOS API to present renderbuffer to CoreAnimation.
    virtual void present();

    shared_ptr<Renderbuffer> _renderbuffer;
    EAGLContext* _renderContext;
    EAGLContext* _defaultContext;

    // TODO: allow these to be set.
    // Does layer support depth/stencil buffer?
    bool _hasDepth;
    bool _hasStencil;

};  // class RenderbufferPresenter_iOS


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__presenter_ios__
