//
//  Shader.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 1/4/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include <iostream>
using std::cerr;
using std::endl;

#include <OpenGLES/ES2/gl.h>

#include "Shader.h"
#include "Geometry.h"

static const char *s_vertex_stroke = 
	"attribute vec4 posAndNorm;						\n"
	"attribute vec4 lengthEtc;						\n"
	"void main()									\n"
	"{												\n"
	"	vec2 pos = posAndNorm.xy;					\n"
	"	vec2 norm = posAndNorm.zw;					\n"
	"	gl_Position.xy = (pos + 10.0*norm) / 150.0;	\n"
	"   gl_Position.z = 0.0;						\n"
	"	gl_Position.w = 1.0;						\n"
	"}												\n";

static const char *s_vertex_tri = 
	"attribute vec4 pos;							\n"
	"void main()									\n"
	"{												\n"
	"   gl_Position = pos;							\n"
	"}												\n";

static const char *s_fragment =
	"void main()									\n"
	"{												\n"
	"	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);	\n"
	"}												\n";


typedef void (*glGetObjiv)(GLuint, GLenum, GLint*) ;
typedef void (*glGetObjInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*) ;

std::string getErrorLog(GLuint obj, glGetObjiv getLength, glGetObjInfoLog getLog);
std::string getErrorLog(GLuint obj, glGetObjiv getLength, glGetObjInfoLog getLog) {
	GLint len = 0;
	getLength(obj, GL_INFO_LOG_LENGTH, &len);
	if (len <= 1) {
		return "(unknown error)";
	}
	else {
		char *buf = new char[len];
		getLog(obj, len, NULL, buf);
		std::string result(buf);
		delete[] buf;
		return result;
	}
}

GLuint 
Shader::createShader(GLenum shaderType, const char *src) {
	GLuint shader;
	GLint compiled;
	
	shader = glCreateShader(shaderType);
	if (!shader) {
		cerr << "couldn't create shader" << endl;
		return 0;
	}
	
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	
	if (!compiled) {
		cerr 
			<< "error while compiling shader: " 
			<< getErrorLog(shader, glGetShaderiv, glGetShaderInfoLog)
			<< endl;
		glDeleteShader(shader);
		shader = 0;
	}
	return shader;
}


Shader::Shader() {
	m_vertex = createShader(GL_VERTEX_SHADER, s_vertex_stroke);
	m_fragment = createShader(GL_FRAGMENT_SHADER, s_fragment);
	
	m_program = glCreateProgram();
	if (!m_program) return;
	
	glAttachShader(m_program, m_vertex);
	glAttachShader(m_program, m_fragment);
	
//	glBindAttribLocation(m_program, 0, "pos");
	glBindAttribLocation(m_program, Geometry::POS_AND_NORM, "posAndNorm");
	glBindAttribLocation(m_program, Geometry::LEN_AND_TIME, "lengthEtc");
	
	GLint linked;
	glLinkProgram(m_program);
	glGetProgramiv(m_program, GL_LINK_STATUS, &linked);

	if (!linked) {
		cerr 
			<< "error while linking program: "
			<< getErrorLog(m_program, glGetProgramiv, glGetProgramInfoLog)
			<< endl;
		glDeleteProgram(m_program);
		m_program = 0;
	}
	else {
		cerr << "successfully linked GLSL program" << endl;
	}
	
	glUseProgram(m_program);
}

Shader::~Shader() {
	if (m_program) glDeleteProgram(m_program);
	if (m_vertex) glDeleteShader(m_vertex);
	if (m_fragment) glDeleteShader(m_fragment);
	m_program = m_vertex = m_fragment = 0;
}
