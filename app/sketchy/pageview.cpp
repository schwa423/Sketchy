//
//  pageview.cpp
//  schwa::app::sketchy
//
//  Created by Josh Gargus on 9/16/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "pageview.h"
#include "renderer.h"
#include "page.h"

#include <iostream>
using std::cerr;
using std::endl;

#include "Eigen/Dense"
#include "Eigen/Geometry"
using namespace Eigen;


// namespace schwa::app::sketchy
namespace schwa {namespace app {namespace sketchy {


PageView::PageView() : _page(new Page) {

}


void PageView::render(uint64_t time) {
    // TODO: We're not using a completely full-screen display (eg: the tab-bar at screen bottom
    //       takes some space), and so the width and height are not merely swapped upon device
    //       rotation.  As a result, the scale changes slightly between orientations.
    float ratio = (float)_width / (float)_height;
    auto viewScale = ((_width < _height)
                            ? Scaling(1.f, ratio)
                            : Scaling(1.f/ratio, 1.f));
    
    
    // TODO: HACK!!
    static GLfloat strokeTime = 0.0f;
    strokeTime += 1.f / 60.f;
    _shader->setTime(strokeTime);

    
    
    

    // TODO: use wall-clock time instead of "strokeTime" hack
    auto transform = Scaling(viewScale);
    _page->render(strokeTime, _renderer, transform);
    
    
    
    
    
    )


    
    
    
    // TODO: HACK!!
    static GLfloat strokeTime = 0.0f;
    strokeTime += 1.f / 60.f;
    _shader->setTime(strokeTime);

    glViewport(0, 0, _width, _height);
    glClearColor(0.19f, 0.19f, 0.35f, 1.0f);
    glClearColor(0.1f, 0.1f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    CHECK_GL("just bound stroke-shader; about to draw page-geometry");
    float angleOffset = 0.0f;
    float speed = 14.0f;
    float freq = 0.10f;
    for (auto& stroke : _strokes) {
        // Give each stroke a slightly different sine-wave frequency and speed.
        auto transform = viewScale * Rotation2Df(angle + angleOffset);
        angleOffset += 0.12;
        speed -= 1.0;
        freq += 0.02;
        _shader->setTransform(transform);
        _shader->setSpeed(speed);
        _shader->setFrequency(freq);

        stroke->draw(_renderer, time);
    }

    CHECK_GL("just drew page-geometry");
}


void PageView::destroyRendererState(grfx::Renderer_ptr renderer) {
    // TODO: our usage of destroyRendererState() conflates two use-cases:
    //         - view is being destroyed (eg: because OS tells us to free memory)
    //         - renderer is destroyed
    //       ... but this isn't quite right, because it can lead to destroying the
    //       shader too often (eg: if another view continues to use the same shader).
    //       Solution: perhaps add a weak-set in the renderer for shaders so that they'll
    //       be destroyed when nobody is using them, or explicitly when the renderer is
    //       destroyed.
    _shader->destroyRendererState(renderer);

    for (auto& stroke : _strokes)
        stroke->destroyRendererState(renderer);
}


void PageView::initializeRendererState(grfx::Renderer_ptr renderer) {
    if (_renderer == renderer) return;
    _renderer = renderer;

    // Shaders/programs can't be shared via OpenGL sharegroups (unfortunately),
    // but since Shaders are lazily initialized on the first call to bind(),
    // it's OK to instantiate it here.
    _shader->initializeRendererState(renderer);

    // TODO: this is a hack!  Just because this view is appearing
    //       (perhaps because it has just been instantiated) doesn't
    //       imply that the Renderer has been destroyed/recreated,
    //       and that therefore the index-buffer is invalid.
    //       See comment in destroyRendererState()... this might be
    //       another example of a resource shared between views, and
    //       which should therefore be held by the Renderer.
    Stroke::clearIndexBuffer();

    // TODO: kick off asynchronous stroke loading.
}


}}}  // namespace schwa::app::sketchy