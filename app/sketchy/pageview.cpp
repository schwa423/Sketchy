//
//  pageview.cpp
//  schwa::app::sketchy
//
//  Created by Josh Gargus on 9/16/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "pageview.h"
#include "renderer2.h"

#include <iostream>
using std::cerr;
using std::endl;

#include "Eigen/Dense"
#include "Eigen/Geometry"
using namespace Eigen;

// namespace schwa::app::sketchy
namespace schwa {namespace app {namespace sketchy {


void PageView::render() {
    // Hack to try out Eigen, and to add a transformation to the page's strokes.
    const float TWOPI = 3.14159f * 2;
    static float angle = 0.0f;
    angle += 0.02f;
    if (angle > TWOPI) angle -= TWOPI;
    Affine2f transform = Translation2f(0.0f, 0.0f) * Rotation2Df(angle);

    auto renderer = _renderer.lock();
    auto framebuffer = _framebuffer.lock();
    if (!renderer || !framebuffer) return;

    renderer->useFramebufferDuring(framebuffer, [=](){
        if (!_geometry.get()) _geometry.reset(new Sketchy::Geometry());
        if (!_shader.get()) _shader.reset(new Sketchy::Shader());

        _shader->setTransform(transform);

        glViewport(50, 50, 700, 700);

        glClearColor(0.27f, 0.45f, 0.58f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        _shader->bind();
        _geometry->draw();
    });
}


void PageView::destroyRendererState(shared_ptr<grfx::Renderer> renderer) {
    cerr << "XXXXX: MUST IMPLEMENT PageView destroyRendererState()" << endl;
}


void PageView::initializeRendererState(shared_ptr<grfx::Renderer> renderer) {
    cerr << "XXXXX: MUST IMPLEMENT PageView initializeRendererState()" << endl;
}


}}}  // namespace schwa::app::sketchy