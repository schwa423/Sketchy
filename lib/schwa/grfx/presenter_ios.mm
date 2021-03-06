//
//  presenter_ios.mm
//  schwa::grfx
//
//  Created by Josh Gargus on 10/3/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "presenter_ios.h"
#include "renderer_ios.h"
#include "port/ios/renderbuffer_ios.h"

#include <iostream>
using std::cerr;
using std::endl;


// namespace schwa::grfx
namespace schwa {namespace grfx {


LayerPresenter_iOS::LayerPresenter_iOS(const shared_ptr<Renderer>& renderer,
                                       EAGLContext* renderContext, EAGLContext* defaultContext)
    :   Presenter(renderer),
        _renderContext(renderContext), _defaultContext(defaultContext),
        _hasDepth(true), _hasStencil(false) {

}


void LayerPresenter_iOS::setLayer(CAEAGLLayer *layer) {
    // TODO: can only be called from main thread; enforce.  Furthermore,
    //       rendering must be stopped (so we can release the old renderbuffer).

    if (_renderbuffer) {
        // If presenter already has a renderbuffer, nuke it, else
        // creating the new one would fail.
        _renderbuffer->bind();
        [_defaultContext renderbufferStorage:GL_RENDERBUFFER fromDrawable: nil];
    }

    // TODO: obtain this renderbuffer from the renderer, so we don't need _defaultContext?
    _renderbuffer = NewRenderbufferFromLayer(_renderer, _defaultContext, layer);

    // Having difficulty creating a multisample framebuffer on retina display.
#if 0
    _framebuffer = MultisampleFramebuffer::New(_renderer, _renderbuffer, _hasDepth, _hasStencil);
#else
    uint width = _renderbuffer->width();
    uint height = _renderbuffer->height();
    shared_ptr<Renderbuffer> depth;
    shared_ptr<Renderbuffer> stencil;
    if (_hasDepth)
        depth = Renderbuffer::NewDepth(_renderer, width, height, 1);
    if (_hasStencil)
        stencil = Renderbuffer::NewStencil(_renderer, width, height, 1);
    _framebuffer = Framebuffer::New(_renderer, _renderbuffer, depth, stencil);
#endif
    
    setBounds(_framebuffer->width(), _framebuffer->height());
}


void LayerPresenter_iOS::present() {
    // TODO: can only be called from render thread; enforce.

    if (!_renderbuffer) {
        cerr << "WARNING!!  no renderbuffer, so cannot present layer!!!!!!!" << endl;
        return;
    }

    _renderbuffer->bind();
    CHECK_GL("failed to bind renderbuffer for present()");

    if (YES != [_renderContext presentRenderbuffer:GL_RENDERBUFFER])
        cerr << "ERROR!!  failed to present renderbuffer" << endl;
}


}}  // namespace schwa::grfx
