//
//  view.cpp
//  schwa::grfx
//
//  Created by Josh Gargus on 9/15/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "view.h"
#include "renderer.h"

#include <exception>

// namespace schwa::grfx
namespace schwa {namespace grfx {

void View::setBounds(uint16_t width, uint16_t height) {
    _width = width;
    _height = height;
}


void View::setRenderer(const shared_ptr<Renderer>& newRenderer) {
    shared_ptr<Renderer> currentRenderer = _renderer.lock();
    if (currentRenderer == newRenderer) return; // no change

    if (currentRenderer.get()) throw std::logic_error("renderer already exists");
    if (!newRenderer.get()) throw std::logic_error("renderer is nullptr");

    _renderer = newRenderer;
    initializeRendererState(newRenderer);
}

void View::clearRenderer() {
    shared_ptr<Renderer> currentRenderer = _renderer.lock();

    _renderer.reset();
    if (currentRenderer.get())
        destroyRendererState(currentRenderer);
}


}}  // namespace schwa::grfx
