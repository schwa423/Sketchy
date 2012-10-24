//
//  strokeshader.cpp
//  schwa::app::sketchy
//
//  Created by Josh Gargus on 10/21/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include "strokeshader.h"


// namespace schwa::app::sketchy
namespace schwa {namespace app { namespace sketchy {


void StrokeShader::initializeProgram() {
    grfx::Shader::initializeProgram();
    if (!_program) return;

    glUseProgram(_program);
    CHECK_GL("StrokeShader::initializeProgram()  failed to set program before binding uniforms");

    setUniformLocation(_widthTimeLengths, "widthTimeLengths");
    setUniformLocation(_color, "colorIn");
    setUniformLocation(_transform, "transform");
}


void StrokeShader::bind() {
    this->Shader::bind();
    _widthTimeLengths.apply();
    CHECK_GL("StrokeShader::bind()... applied _widthTimeLengths");
    _color.apply();
    CHECK_GL("StrokeShader::bind()... applied _color");
    _transform.apply();
    CHECK_GL("StrokeShader::bind()... applied _transform");
}


const char* StrokeShader::vertexShaderSource() {
    return
     "// stroke vertex shader "
     "\n    attribute vec4 posAndNorm; "
     "\n    attribute vec4 lengthEtc; "
     "\n    uniform vec4 colorIn; "
     "\n    varying vec4 color; "
     "\n    uniform vec4 widthTimeLengths; "
     "\n    uniform mat3 transform; "
     "\n    void main() "
     "\n    { "
     "\n      // retrieve time and default stroke width "
     "\n      float width = widthTimeLengths.x; "
     "\n      float time = widthTimeLengths.y; "
     "\n      // modify width at each vertex "
     "\n      float speed = time * 12.0; "
     "\n      float freq = 0.1; "
     "\n      // per-vertex scale between 50% and 100% of default stroke width "
     "\n      float scale = cos(lengthEtc.x * freq + speed) / 4.0 + 0.75; "
     "\n      width *= scale; "
     "\n    	 vec2 pos = posAndNorm.xy; "
     "\n    	 vec2 norm = posAndNorm.zw * vec2(width); "
     "\n      vec3 vert = transform * vec3((pos + norm) / 150.0, 1.0); "
     "\n    	 gl_Position.xy = vert.xy; "
     "\n    	 gl_Position.z = 0.0; "
     "\n    	 gl_Position.w = 1.0; "
     "\n    	 color = colorIn; "
     "\n    } ";
}


const char* StrokeShader::fragmentShaderSource() {
    return
    "// solid-color fragment shader"
    "\n    varying lowp vec4 color; "
    "\n    void main() "
    "\n    { "
    "\n      gl_FragColor = color; "
    "\n    } ";
}


}}}  // namespace schwa::app::sketchy
