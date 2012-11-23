 //
//  shader.h
//  schwa::grfx
//
//  Created by Josh Gargus on 10/14/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__grfx__shader__
#define __schwa__grfx__shader__


#include "renderer.h"
#include "platform_gl.h"

#include "Eigen/Geometry"

#include <cstring>


// namespace schwa::grfx
namespace schwa {namespace grfx {


class Shader : public Renderer::Resource {
 public:
    Shader();
    virtual ~Shader();

    virtual void bind();

    // Has the shader been sucessfully initialized (compiled/linked)?
    bool isValid() const { return _initialized && _program; }

    void initializeRendererState(Renderer_ptr renderer);
    void destroyRendererState(Renderer_ptr renderer);

 protected:
    GLuint _vertex;
	GLuint _fragment;
	GLuint _program;
    bool   _initialized;

    virtual void initializeProgram();
    virtual void bindAttribLocations() = 0;
    virtual void invalidateUniforms() = 0;
    virtual const char* vertexShaderSource() = 0;
    virtual const char* fragmentShaderSource() = 0;
    GLuint compileShader(GLenum shaderType, const char *src);

    // Base class for Uniform variables.  Only function is to allow
    // the owning shader to initialize the uniform's location.
    class UniformBase {
        friend class Shader;

     public:
        UniformBase() : _location(0) { }
        GLuint location() const { return _location; }

     protected:
        GLuint _location;

     private:
        void setLocation(GLuint location) { _location = location; }
    };
    // Look-up and set the location for the uniform with the specified name.
    void setUniformLocation(UniformBase& uniform, const char* name);

    // TODO: class comments for Uniform and children.
    template<typename T>
    class Uniform : public UniformBase {
     public:
        Uniform() : UniformBase(), _dirty(true) { }

        void invalidate() { _dirty = true; }
        bool dirty() const { return _dirty; }

        void operator= (const T& val) {
            if (equal(_value, val)) return;
            assign(val);
            _dirty = true;
        }

        void apply() {
            if (!_dirty) return;
            _dirty = false;
            apply(_value);
        }


     protected:
        bool   _dirty;
        T      _value;


        inline bool equal(const Eigen::Affine2f& val1, const Eigen::Affine2f &val2) {
            return val1.matrix() == val2.matrix();
        }

        template<typename T1, int N>
        inline bool equal(const T1 (&val1)[N], const T1 (&val2)[N]) {
            return 0 == memcmp(&val1, &val2, N * sizeof(T1));
        }

        template<typename T1>
        inline bool equal(const T1& val1, const T1& val2) { return val1 == val2; }

        template<typename T1, int N>
        inline void assign(const T1 (&val)[N]) {
            memcpy(_value, val, N * sizeof(T1));
        }

        template<typename T1>
        inline void assign(const T1& val) { _value = val; }

        inline void apply(GLfloat val) {
            glUniform1f(_location, val);
        }

        inline void apply(const GLfloat (&val)[4]) {
            glUniform4fv(_location, 1, val);
            CHECK_GL("glUniform4fv:  GLfloat[4] oops!");
        }

        inline void apply(const Eigen::Affine2f &val) {
            glUniformMatrix3fv(_location, 1, false, val.data());
            CHECK_GL("glUniformMatrix3fv:  Eigen transform oops!");
        }
    };


    class UniformFloat4 : public Uniform<GLfloat[4]> {
     public:
        UniformFloat4() : Uniform<GLfloat[4]>() { }

        // TODO: ugg... why is this necessary?  Shouldn't this already be declared/defined in
        //       the base-class (once it's specialized by GLfloat[4]) ?
        void operator= (const GLfloat (&val)[4]) {
            if (equal(_value, val)) return;
            assign(val);
            _dirty = true;
        }

        // TODO: this interface is a sucky hack
        void set(GLfloat value, int index) {
            if (index < 0 || index >= 4) {
                // TODO: assert
                std::cerr << "BAD INDEX" << std::endl;
                return;
            }
            if (_value[index] != value) {
                _value[index] = value;
                _dirty = true;
            }
        }
    };

    typedef Uniform<GLfloat> UniformFloat;
//    typedef Uniform<GLfloat[4]> UniformFloat4;
    typedef Uniform<Eigen::Affine2f> UniformMatrix3;
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__shader__
