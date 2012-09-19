
//
//  Shader.h
//  Sketchy
//
//  Created by Joshua Gargus on 1/4/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Shader_h
#define Sketchy_Shader_h

namespace Sketchy {

class Shader
{
public:
	Shader();
	~Shader();

	void bind(void);

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

}; // class Shader

} // namespace Sketchy

#endif