//
//  mesh.h
//  schwa::grfx
//
//  Created by Josh Gargus on 10/14/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__grfx__mesh__
#define __schwa__grfx__mesh__


#include "renderer2.h"

#include <memory>
using std::shared_ptr;

#include <map>


using std::unique_ptr;

// namespace schwa::grfx
namespace schwa {namespace grfx {


// TODO: all of this buffer shit needs polishing.  And testing, lots of testing.
class Buffer {
 public:
    Buffer(GLenum target);
    virtual ~Buffer() { }

    // Buffer size in bytes.
    uint size() const { return _size; }

    // Replace the data currently in the buffer.
    template<typename T>
    void load(unique_ptr<T> data, GLsizeiptr size, GLenum usage) {
        load(data.get(), size, usage);
    }

    // TODO: think about desired external API
    void bind();
    void bindVertexAttrib(GLuint index, GLint size, GLenum type, GLsizei stride, GLsizeiptr offset);

 protected:
    virtual void load(GLvoid* data, GLsizeiptr size, GLenum usage) = 0;
    virtual GLuint glBuffer() = 0;
    virtual GLvoid* glAttribPointer(GLsizeiptr offset) = 0;

    GLenum _target;
    GLsizeiptr _size;

    friend std::ostream& operator<< (std::ostream& stream, const Buffer& buffer);
};


class CpuBuffer : public Buffer {
 protected:
    virtual GLuint glBuffer() { return 0; }
    virtual GLvoid* glAttribPointer(GLsizeiptr offset) {
        // TODO: assert ptr not-null
        return static_cast<char*>(_ptr) + offset;
    }

    GLvoid* _ptr;
};


class GpuBuffer : public Buffer, public Renderer::Resource {
 public:
    GpuBuffer(Renderer_ptr& renderer, GLenum target);
    virtual ~GpuBuffer();

    bool isVertexBuffer() const { return GL_ARRAY_BUFFER == _target; }
    bool isIndexBuffer() const { return GL_ELEMENT_ARRAY_BUFFER == _target; }

    friend std::ostream& operator<< (std::ostream& stream, const Buffer& buffer);

 protected:
    virtual void load(GLvoid* data, GLsizeiptr size, GLenum usage);
    virtual GLuint glBuffer() { return _buffer; }
    virtual GLvoid* glAttribPointer(GLsizeiptr offset) {
        // TODO: assert ptr not-null
        return static_cast<char*>(nullptr) + offset;
    }

    GLuint _buffer;
};


class VertexBuffer : public GpuBuffer {
 public:
    VertexBuffer(Renderer_ptr& renderer) : GpuBuffer(renderer, GL_ARRAY_BUFFER) { }
};


// TODO: do something with Mesh, or delete it.  Perhaps for now, it's best
//       to leave mesh-management to the app (i.e. StrokeMesh) vs the core library.
/*
class MeshFormat {
 public:
    struct Attribute {
        GLint size;
        GLenum type;
        GLboolean normalized;
        GLsizei stride;
        GLvoid* pointer;
    };

    // Set a named-attribute.
    // TODO: should check whether an attribute with that name already exists.
    void addAttribute(const std::string& name, const Attribute& attribute);

    // Return a pointer to the first matching attribute,
    // or nullptr if none exists.
    const Attribute* matches(const std::string& matchName, GLint matchSize, GLenum matchType) const;

 private:
    std::map<std::string, Attribute> _attributes;
};


class Mesh {
 public:
    // TODO: shouldn't be necessary to specify data at time of instantiation
    Mesh(const shared_ptr<const MeshFormat>& format,
         const shared_ptr<Buffer>& vertices,
         const shared_ptr<Buffer>& indices,
         int startIndex,
         int triangleCount);

    const shared_ptr<const MeshFormat>& format() const { return _format; }

 protected:
    shared_ptr<Shader> _shader;
    shared_ptr<const MeshFormat> _format;




    shared_ptr<Buffer> _vertices;
    shared_ptr<Buffer> _indices;
    uint32_t _startIndex;
    uint32_t _triangleCount;
};
*/


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__mesh__
