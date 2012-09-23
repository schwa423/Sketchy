//
//  renderer.cpp
//  schwa::grfx
//
//  Created by Josh Gargus on 9/8/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "renderer2.h"
#include "framebuffer2.h"
#include "view.h"

typedef std::lock_guard<std::mutex> lock_guard;

#include <iostream>
using std::cerr;
using std::endl;

// namespace schwa::grfx
namespace schwa {namespace grfx {


// TODO: unit tests, using both nullptr FB, and not.
void Renderer::useFramebufferDuring(const shared_ptr<Framebuffer> &fb, core::Thunk thunk) {
    // Push and bind framebuffer, and execute thunk.
    if (fb) fb->bind();
    _framebufferStack.push(fb);

    // Execute 'thunk', and pop/resolve the framebuffer.
    try {
        thunk();
    } catch(...) {
        resolveAndPopFramebuffer(fb);
        throw;
    }
    resolveAndPopFramebuffer(fb);  // ... since we didn't execute the catch{}.
}


void Renderer::resolveAndPopFramebuffer(const shared_ptr<Framebuffer> &fb) {
    if (fb) fb->resolve();
    _framebufferStack.pop();
    if (!_framebufferStack.empty() && _framebufferStack.top())
        _framebufferStack.top()->bind();
}


void Renderer::setView(const shared_ptr<View> &view) {
    // TODO: enqueue set-view thunk instead of obtaining lock.
    lock_guard lock(_mutex);
    view->setFramebuffer(_framebuffer);
    _view = view;
}


void Renderer::render() {
    lock_guard lock(_mutex);

    // Render the view, if any.
    if (_view.get()) {
        _view->render();
    } else {
        // There was no view to render, so render default background.
        static int renderCount = 1;
        static int direction = -1;
        renderCount += direction;
        if (renderCount == 0 || renderCount == 255) direction *= -1;

        GLfloat red = renderCount / 255.0f;
        useFramebufferDuring(_framebuffer, [=](){
            glClearColor(red, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        });
    }
    glFlush();
    CHECK_GL("Renderer::render()... about to swap buffers");
    
    swapBuffers();
}


shared_ptr<Renderbuffer> Renderer::colorRenderbuffer() {
    return _framebuffer->_color;
}


}}  // namespace schwa::grfx
