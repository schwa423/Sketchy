//
//  shader.cpp
//  schwa::grfx
//
//  Created by Josh Gargus on 10/14/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "shader.h"

#include <iostream>
using std::cerr;
using std::endl;


// namespace schwa::grfx
namespace schwa {namespace grfx {


// Helper function for obtaining error-log from shader-compilation and program-linking.
typedef void (*glGetObjiv)(GLuint, GLenum, GLint*);
typedef void (*glGetObjInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
static std::string getErrorLog(GLuint obj, glGetObjiv getLength, glGetObjInfoLog getLog);
static std::string getErrorLog(GLuint obj, glGetObjiv getLength, glGetObjInfoLog getLog) {
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


Shader::Shader() : Renderer::Resource(nullptr),
                   _initialized(false),
                   _vertex(0), _fragment(0), _program(0) {

}


void Shader::initializeRendererState(Renderer_ptr renderer) {
    // TODO: assert current renderer is NULL
    // TODO: not quite sure why we can't just do this in destroyRendererState() ?
    _renderer = renderer;
    _vertex = _fragment = _program = 0;
    _initialized = false;

    // It would be "nicer" to do this in destroyRendererState(), but
    // we call that from the destructor, after subclass destructors
    // have already run.
    invalidateUniforms();
}


void Shader::destroyRendererState(Renderer_ptr renderer) {
    // TODO: remove once Clang can capture instance variables by copy, not reference.
    auto _vertex   = this->_vertex;
    auto _fragment = this->_fragment;
    auto _program  = this->_program;
    finalize([=](){
        glDeleteProgram(_program);
        glDeleteShader(_vertex);
        glDeleteShader(_fragment);
    });
    _initialized = false;
    _renderer.reset();
}


Shader::~Shader() {
    destroyRendererState(nullptr);
}


void Shader::bind() {
    if (!_initialized)
        initializeProgram();

    glUseProgram(_program);
    // TODO: macro allowing << _program
    CHECK_GL("Shader::bind()... failed to use program");
}


void Shader::initializeProgram() {
    _initialized = true;

    CHECK_GL("about to compile shaders");

    _vertex = compileShader(GL_VERTEX_SHADER, vertexShaderSource());
    _fragment = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource());

    CHECK_GL("shaders compiled; about to link");

    _program = glCreateProgram();
    if (!_program) {
        glDeleteShader(_vertex);
        glDeleteShader(_fragment);
        _vertex = _fragment = 0;
        return;
    }

    glAttachShader(_program, _vertex);
    glAttachShader(_program, _fragment);

    // Allow subclasses to specify which attribs are bound to which locations.
    bindAttribLocations();

    GLint linked;
    glLinkProgram(_program);
    glGetProgramiv(_program, GL_LINK_STATUS, &linked);

    if (!linked) {
        cerr
            << "error while linking program: "
            << getErrorLog(_program, glGetProgramiv, glGetProgramInfoLog)
            << endl;
		glDeleteProgram(_program);
        glDeleteShader(_vertex);
        glDeleteShader(_fragment);
		_vertex = _fragment = _program = 0;
	} else {
        cerr << "successfully linked shader program" << endl;
    }
}


GLuint Shader::compileShader(GLenum shaderType, const char* src) {
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


void Shader::setUniformLocation(UniformBase& uniform, const char* name) {
    GLuint location;
    location = glGetUniformLocation(_program, name);
    // TODO: error-checking first
    uniform.setLocation(location);
}


}}  // namespace schwa::grfx
