//
//  renderer_ios.h
//  schwa::grfx
//
//  Created by Josh Gargus on 9/9/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__grfx__renderer_ios__
#define __schwa__grfx__renderer_ios__

#include "renderer2.h"

// Ensure that we're on iOS.
#if defined(__APPLE__)
#include "TargetConditionals.h"
#if !TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
#error "this file should only be compiled on iOS"
#endif
#else
#error "this file should only be compiled on iOS"
#endif

#include "platform_gl.h"

@class VsyncListener;
@class CAEAGLLayer;


// namespace schwa::grfx
namespace schwa {namespace grfx {


class Renderer_iOS : public Renderer {
 public:
    static shared_ptr<Renderer_iOS> New();

    virtual ~Renderer_iOS();

    // Start/stop the render-loop.  Must only be called from the main thread.
    // TODO: verify precondition?
    virtual void startRendering();
    virtual void stopRendering();
    virtual bool isRunning();

    // TODO: always creates a depth buffer... make this configurable?
    // TODO: delete?
    // Must call from main thread.  TODO: enforce
    shared_ptr<Framebuffer> NewFramebuffer(CAEAGLLayer* layer, bool multisample = true);

    // Must call from main thread.  TODO: enforce
    shared_ptr<Presenter> NewLayerPresenter();

 protected:
    // Don't construct directly, instead use New().
    // 2-part initialization... Init() creates resources
    // that need a shared_ptr to the renderer, which we
    // can't obtain during the constructor.
    Renderer_iOS();

 private:
    // TODO: comment
    __strong EAGLContext* _renderContext;
    __strong EAGLContext* _loaderContext;
    __strong EAGLContext* _defaultContext;
    __strong VsyncListener* _vsync;
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__renderer_ios__
