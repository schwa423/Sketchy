//
//  renderer.cpp
//  schwa::grfx
//
//  Created by Josh Gargus on 9/8/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "renderer.h"
#include "framebuffer.h"
#include "view.h"
#include "presenter.h"

typedef std::lock_guard<std::mutex> lock_guard;

#include <iostream>
using std::cerr;
using std::endl;

// namespace schwa::grfx
namespace schwa {namespace grfx {

void Renderer::pauseRenderingDuring(core::Thunk action) {
    bool wasRendering = isRunning();
    if (wasRendering) stopRendering();
    try {
        action();
    } catch(...) {
        // TODO: something more informative
        cerr << "caught exception in pauseRenderingDuring";
    }
    if (wasRendering) startRendering();
}


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


void Renderer::render() {
    // TODO: is this the best approach?  The reason we do this is
    //       that Renderer_iOS grabs the mutex to stop rendering,
    //       then waits for the render-thread to finish... but
    //       the render-thread will deadlock if it tries to render
    //       once more.  Well, it does the job.
    if (!_mutex.try_lock()) return;
    lock_guard lock(_mutex, std::adopt_lock);

    // Run finalizers to potentially free memory.
    runFinalizers();

    // TODO: This is a decent hack, since if we are at 60fps, the inter-frame
    //       time is 16.67ms.  But we will need something better.
    static uint64_t time = 0;
    time += 16;

    // Tell all presenters to render.
    int count = 0;
    for (auto weak : _presenters) {
        auto p = weak.lock();
        if (p) {
            p->render(time);
            count++;
        }
    }
    if (count < _presenters.size()) cleanupPresenters();

    // TODO: probably shouldn't be necessary.  Is it?
    runFinalizers();
}


void Renderer::cleanupPresenters() {
    vector<weak_ptr<Presenter>> cleaned;
    for (auto weak : _presenters) {
        auto p = weak.lock();
        if (p)
            cleaned.push_back(p);
    }
    std::swap(cleaned, _presenters);
}


void Renderer::addPresenter(const shared_ptr<Presenter> &presenter) {
    _presenters.push_back(presenter);
}


}}  // namespace schwa::grfx
