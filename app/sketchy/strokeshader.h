//
//  strokeshader.h
//  schwa::app::sketchy
//
//  Created by Josh Gargus on 10/21/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef __schwa__app__sketchy__strokeshader__
#define __schwa__app__sketchy__strokeshader__


#include "shader2.h"


// namespace schwa::app::sketchy
namespace schwa {namespace app { namespace sketchy {


class StrokeShader : public grfx::Shader {
 public:
    StrokeShader(grfx::Renderer_ptr renderer) : grfx::Shader(renderer) { }

    // Bind the shader, and set any uniform parameters which have changed.
    virtual void bind();

    inline void setTransform(const Eigen::Affine2f &transform) { _transform = transform; }
    inline void setColor(const GLfloat (&color)[4]) { _color = color; }

    inline void setWidth(GLfloat width)              { _widthTimeLengths.set(width, 0);       }
    inline void setTime(GLfloat time)                { _widthTimeLengths.set(time, 1);        }
    inline void setStartLength(GLfloat startLength)  { _widthTimeLengths.set(startLength, 2); }
    inline void setEndLength(GLfloat endLength)      { _widthTimeLengths.set(endLength, 3);   }

 protected:
    virtual void initializeProgram();
    virtual const char* vertexShaderSource();
    virtual const char* fragmentShaderSource();

    UniformFloat4  _widthTimeLengths;
    UniformFloat4  _color;
    UniformMatrix3 _transform;
};


}}}  // namespace schwa::app::sketchy


#endif  // #ifndef __schwa__app__sketchy__strokeshader__
