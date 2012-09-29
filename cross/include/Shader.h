
//
//  Shader.h
//  Sketchy
//
//  Created by Joshua Gargus on 1/4/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Shader_h
#define Sketchy_Shader_h

#include "Eigen/Geometry"
using namespace Eigen;

namespace Sketchy {

class Shader
{
public:
	Shader();
	~Shader();

	void bind(void);

    // Hack... assumes shader has a "transform"
    void setTransform(const Affine2f& transform) { m_transformVal = transform; }

private:
	GLuint createShader(GLenum shaderType, const char *src);

	GLuint m_vertex;
	GLuint m_fragment;
	GLuint m_program;

	// hack
	GLfloat m_colorVal[4];
	GLuint m_color;
	GLfloat m_timeEtcVal[4];
	GLuint m_timeEtc;
    Affine2f m_transformVal;
    GLuint m_transform;
}; // class Shader

} // namespace Sketchy

#endif
