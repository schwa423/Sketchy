//
//  renderer.h
//  schwa::grfx
//
//  Created by Josh Gargus on 9/8/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__grfx__renderer__
#define __schwa__grfx__renderer__


#include <iostream>
#include <mutex>

#include <stack>
#include <memory>
using std::shared_ptr;
using std::weak_ptr;
#include <vector>
using std::vector;

#include "platform_gl.h"
#include "thunk_list.h"


// namespace schwa::grfx
namespace schwa {namespace grfx {

class Framebuffer;
class View;
class Presenter;

// Save typing... we use this a lot (probably more than we should).
class Renderer;
typedef const shared_ptr<Renderer>& Renderer_ptr;

class Renderer : public std::enable_shared_from_this<Renderer> {
 public:
    virtual ~Renderer() { }

    // Return the default framebuffer.
    shared_ptr<Framebuffer> framebuffer() { return _framebuffer; }

    // Bind the framebuffer during the execution of 'thunk', and resolve
    // it afterward.  If 'thunk' throws an exception, it will be caught
    // and rethrown, after first popping/resolving the framebuffer.
    void useFramebufferDuring(const shared_ptr<Framebuffer> &fb, core::Thunk thunk);

    // Render the currently-set view (if any);
    // TODO: probably shouldn't be public, since it's typically called by a subclass render-loop.
    void render();

    // Start/stop the render-loop.  Must only be called from the main thread.
    // TODO: verify precondition?
    virtual void startRendering() = 0;
    virtual void stopRendering() = 0;
    virtual bool isRunning() = 0;
    // TODO: comment
    void pauseRenderingDuring(core::Thunk action);

    // TODO: comment
    void addPresenter(const shared_ptr<Presenter>& presenter);

    // Represents one or more OpenGL resources that work together as a unit.
    class Resource {
     public:
        Resource(shared_ptr<Renderer> r) : _renderer(r) { }
        virtual ~Resource() { }

        shared_ptr<Renderer> renderer() const { return _renderer.lock(); }

     protected:
        // Register arbitrary lambda to perform clean-up within the renderer thread.
        void finalize(const core::Thunk &finalizer) {
            auto r = _renderer.lock();
            if (r.get()) r->addFinalizer(finalizer);
        }

        weak_ptr<Renderer> _renderer;
    };
    friend class Resource;

 protected:
    // Default framebuffer.
    shared_ptr<Framebuffer> _framebuffer;

    // Presenters to be renderered each frame.
    vector<weak_ptr<Presenter>> _presenters;
    // Remove nullptrs.  _mutex must be locked.
    void cleanupPresenters();

    // Stack of framebuffers used during rendering... top one is current render-target.
    std::stack<shared_ptr<Framebuffer>> _framebufferStack;
    void resolveAndPopFramebuffer(const shared_ptr<Framebuffer> &fb);

    // Resources must be destroyed in the thread which "owns"
    // the OpenGL context, so we defer resource-finalizer thunks
    // until that thread is running.
    void addFinalizer(core::Thunk finalizer) { _finalizers.add(finalizer); }
    // Run all finalizers, and clear list.
    void runFinalizers() { _finalizers.drain(); }
    core::ThunkList _finalizers;

    std::mutex _mutex;
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__renderer__

