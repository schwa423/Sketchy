//
//  presenter.cpp
//  schwa::grfx
//
//  Created by Josh Gargus on 9/30/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "presenter.h"
#include "renderer2.h"
#include "view.h"

using std::cerr;
using std::endl;

// namespace schwa::grfx
namespace schwa {namespace grfx {


void Presenter::setView(const shared_ptr<View>& view) {
    if (view == _view) return;

    // TODO: view needs to be updated for framebuffer dimensions, etc.
    cerr << "WARNING!!! Presenter::setView() is not completely implemented" << endl;

    _view = view;
}


void Presenter::setFramebuffer(const shared_ptr<Framebuffer>& framebuffer) {
    if (framebuffer == _framebuffer) return;
    _framebuffer = framebuffer;

    // TODO: if there is a view, update it for the new framebuffer dimensions, etc.
    cerr << "WARNING!!! Presenter::setFramebuffer() is not completely implemented" << endl;
}


void Presenter::render(uint64_t time) {
    _renderer->useFramebufferDuring(_framebuffer, [&](){
        if (_view) {
            _view->render(time);
        } else {
            // If there is no view, render pulsating red screen.
            int red = time % 512;
            if (red > 255)
                red = 511 - red;
            glClearColor(red / 255.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    });

    present();
}


}}  // namespace schwa::grfx