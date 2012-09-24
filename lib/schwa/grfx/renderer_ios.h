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

    // Start/stop the render-loop.
    virtual void pauseRendering();
    virtual void unpauseRendering();
    bool isPaused();

    // Initialize renderer with current layer.
    void initialize(CAEAGLLayer* glayer);

 protected:
    // Don't construct directly, instead use New().
    // 2-part initialization... Init() creates resources
    // that need a shared_ptr to the renderer, which we
    // can't obtain during the constructor.
    Renderer_iOS();

    
    // One of these is called by initialize(); currently we use multisampling.
    void initializeFramebuffer(CAEAGLLayer* glayer);
    void initializeMultisampleFramebuffer(CAEAGLLayer* glayer);

    virtual void swapBuffers();

 private:
    __strong EAGLContext* _renderContext;
    __strong EAGLContext* _loaderContext;
    __strong VsyncListener* _vsync;
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__renderer_ios__
