//
//  presenter.cpp
//  schwa::grfx
//
//  Created by Josh Gargus on 9/30/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "presenter.h"
#include "renderer.h"
#include "view.h"

using std::cerr;
using std::endl;

// namespace schwa::grfx
namespace schwa {namespace grfx {


Presenter::Presenter(const shared_ptr<Renderer>& r) : _renderer(r), _width(0), _height(0) {

}


void Presenter::setView(const shared_ptr<View>& view) {
    if (view == _view) return;

    _view = view;
    if (_view)
        _view->setBounds(_width, _height);
}


void Presenter::setBounds(uint16_t width, uint16_t height) {
    if (_width == width && _height == height) return;

    _width = width;
    _height = height;
    if (_view)
        _view->setBounds(_width, _height);
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
