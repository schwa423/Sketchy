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


void PageView::render(uint64_t time) {
    // TODO: We're not using a completely full-screen display (eg: the tab-bar at screen bottom
    //       takes some space), and so the width and height are not merely swapped upon device
    //       rotation.  As a result, the scale changes slightly between orientations.
    float ratio = (float)_width / (float)_height;
    auto viewScale = ((_width < _height)
                            ? Scaling(1.f, ratio)
                            : Scaling(1.f/ratio, 1.f));

    const float TWOPI = 3.14159f * 2;
    static float angle = 0.0f;
    angle += 0.02f;
    if (angle > TWOPI) angle -= TWOPI;

    auto transform = viewScale * Rotation2Df(angle);

    if (!_geometry.get()) _geometry.reset(new Sketchy::Geometry());

    // TODO: HACK!!
    static GLfloat strokeTime = 0.0f;
    strokeTime += 1.f / 60.f;

    _shader->setTime(strokeTime);
    _shader->setTransform(transform);

    glViewport(0, 0, _width, _height);

    glClearColor(0.27f, 0.68f, 0.45f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    _shader->bind();
    CHECK_GL("just bound stroke-shader; about to draw page-geometry");
    _geometry->draw();
    CHECK_GL("just drew page-geometry");
}


void PageView::destroyRendererState(grfx::Renderer_ptr renderer) {
    _shader.reset();
}


void PageView::initializeRendererState(grfx::Renderer_ptr renderer) {
    // Shaders/programs can't be shared via OpenGL sharegroups (unfortunately),
    // but since Shaders are lazily initialize on the first call to bind(),
    // it's OK to instantiate it here.
    _shader.reset(new StrokeShader(renderer));

    GLfloat color[4] {0.87f, 0.5f, 0.5f, 1.0f};
    _shader->setColor(color);

    _shader->setTime(0.f);
    _shader->setWidth(5.f);
    _shader->setStartLength(0);
    _shader->setEndLength(0);
}


}}}  // namespace schwa::app::sketchy