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

#include "platform_gl.h"
#include "thunk_list.h"


// namespace schwa::grfx
namespace schwa {namespace grfx {

class Framebuffer;
class Renderbuffer;
class View;

class Renderer : public std::enable_shared_from_this<Renderer> {
 public:
    virtual ~Renderer() { }

    // Return the default framebuffer.
    shared_ptr<Framebuffer> framebuffer() { return _framebuffer; }

    // Bind the framebuffer during the execution of 'thunk', and resolve
    // it afterward.  If 'thunk' throws an exception, it will be caught
    // and rethrown, after first popping/resolving the framebuffer.
    void useFramebufferDuring(const shared_ptr<Framebuffer> &fb, core::Thunk thunk);

    // Set the view to be rendered.
    void setView(const shared_ptr<View>& view);

    // Render the currently-set view (if any);
    // TODO: probably shouldn't be public, since it's typically called by a subclass render-loop.
    void render();

    // Start/stop the render-loop.
    virtual void startRendering() = 0;
    virtual void stopRendering() = 0;

    // Represents one or more OpenGL resources that work together as a unit.
    class Resource {
     public:
        Resource(shared_ptr<Renderer> r) : _renderer(r) { }
        virtual ~Resource() { }

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

    // The view to render.
    shared_ptr<View> _view;

    // Stack of framebuffers used during rendering... top one is current render-target.
    std::stack<shared_ptr<Framebuffer>> _framebufferStack;
    void resolveAndPopFramebuffer(const shared_ptr<Framebuffer> &fb);

    // Subclasses override to display the frame after rendering.
    virtual void swapBuffers() = 0;

    // Resources must be destroyed in the thread which "owns"
    // the OpenGL context, so we defer resource-finalizer thunks
    // until that thread is running.
    void addFinalizer(core::Thunk finalizer) { _finalizers.add(finalizer); }
    // Run all finalizers, and clear list.
    void runFinalizers() { _finalizers.drain(); }
    core::ThunkList _finalizers;

    // Handle of the default-framebuffer's color-renderbuffer.
    shared_ptr<Renderbuffer> colorRenderbuffer();

    std::mutex _mutex;
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__renderer__

