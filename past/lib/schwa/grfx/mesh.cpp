//
//  mesh.cpp
//  schwa::grfx
//
//  Created by Josh Gargus on 10/14/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "mesh.h"

#include <sstream>

// namespace schwa::grfx
namespace schwa {namespace grfx {


Buffer::Buffer(GLenum target) : _target(target), _size(0) {

}


void Buffer::bind() {
    glBindBuffer(_target, glBuffer());
}


void Buffer::bindVertexAttrib(GLuint index, GLint size, GLenum type, GLsizei stride, GLsizeiptr offset) {
    // TODO: assert _target == GL_ARRAY_BUFFER
    glBindBuffer(_target, glBuffer());
    glVertexAttribPointer(index, size, type, false, stride, glAttribPointer(offset));
}


GpuBuffer::GpuBuffer(Renderer_ptr& renderer, GLenum target) : Buffer(target),
                                                              Renderer::Resource(renderer) {
    glGenBuffers(1, &_buffer);
}


GpuBuffer::~GpuBuffer() {
    // If renderer still exists, schedule deletion of OpenGL resources.
    // TODO: remove once Clang can capture instance variables by copy, not reference.
    auto _buffer = this->_buffer;
    finalize([=](){
        glDeleteBuffers(1, &_buffer);
    });
}


void GpuBuffer::load(GLvoid* data, GLsizeiptr size, GLenum usage) {
    // TODO: debugging code to assert that we properly hand off load/bind between contexts
    //       (i.e. that we unbind/flush in one context before binding in the next)
    _size = size;
    glBindBuffer(_target, _buffer);
    glBufferData(_target, size, data, usage);
}


std::ostream& operator<< (std::ostream& stream, const Buffer& buffer) {
    stream << "Buffer(type=";
    switch (buffer._target) {
        case GL_ARRAY_BUFFER:
            stream << "vertex";
            break;
        case GL_ELEMENT_ARRAY_BUFFER:
            stream << "vertex-index";
            break;
        default:
            stream << "unknown";
            break;
    }
    stream << " size=" << buffer._size << ")";
    return stream;
}



// TODO: do something with Mesh, or delete it.
/*
void MeshFormat::addAttribute(const std::string& name, const Attribute& attribute) {
    _attributes[name] = attribute;
}


const MeshFormat::Attribute* MeshFormat::matches(const std::string& matchName,
                                                 GLint matchSize,
                                                 GLenum matchType) const {
    auto it = _attributes.find(matchName);
    if (it == _attributes.end())
        return nullptr;

    auto attr = it->second;
    if (attr.size != matchSize || attr.type != matchType)
        return nullptr;

    // Return pointer to mapped value because I'm not sure
    // whether auto makes "attr" a reference or a copy.
    // TODO: learn answer to the above.
    return &(it->second);
}


Mesh::Mesh(const shared_ptr<const MeshFormat>& format,
           const shared_ptr<Buffer>& vertices,
           const shared_ptr<Buffer>& indices,
           int startIndex,
           int triangleCount) : _format(format),
                                _vertices(vertices),
                                _indices(indices),
                                _startIndex(startIndex),
                                _triangleCount(triangleCount) {

}


void Mesh::draw() {

}
*/


}}  // namespace schwa::grfx
