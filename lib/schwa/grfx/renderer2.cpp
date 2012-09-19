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

// namespace schwa::grfx
namespace schwa {namespace grfx {

void Renderer::useFramebufferDuring(shared_ptr<Framebuffer> fb, core::Thunk thunk) {
    // Push and bind framebuffer, and execute thunk.
    _framebufferStack.push(fb);
    fb->bind();

    // Execute 'thunk', and pop/resolve the framebuffer.
    try {
        thunk();
    } catch(...) {
        resolveAndPopFramebuffer();
        throw;
    }
    resolveAndPopFramebuffer();  // ... since we didn't execute the catch{}.
}


void Renderer::resolveAndPopFramebuffer() {
    _framebufferStack.top()->resolve();
    _framebufferStack.pop();
    if (!_framebufferStack.empty())
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
            glViewport(50, 50, 512, 512);

            glClearColor(red, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        });
    }
    swapBuffers();
}


GLuint Renderer::colorRenderbuffer() {
    return _framebuffer->_color;
}


}}  // namespace schwa::grfx
