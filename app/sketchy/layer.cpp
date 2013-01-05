//
//  layer.cpp
//  schwa::app::sketchy
//
//  Created by Josh Gargus on 1/5/13.
//  Copyright (c) 2013 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//


#include "layer.h"








void Layer::render(uint64_t time) {
    // TODO: We're not using a completely full-screen display (eg: the tab-bar at screen bottom
    //       takes some space), and so the width and height are not merely swapped upon device
    //       rotation.  As a result, the scale changes slightly between orientations.
    float ratio = (float)_width / (float)_height;
    auto viewScale = ((_width < _height)
                      ? Scaling(1.f, ratio)
                      : Scaling(1.f/ratio, 1.f));
    
    // Cause all strokes to rotate around center at fixed rate.
    const float TWOPI = 3.14159f * 2;
    static float angle = 0.0f;
    angle += 0.004f;
    if (angle > TWOPI) angle -= TWOPI;
    
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
