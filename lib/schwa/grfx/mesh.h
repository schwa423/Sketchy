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


// TODO: FINISH THIS CLASS!
//       FINISH THIS CLASS!
//       FINISH THIS CLASS!
//       FINISH THIS CLASS!
//       FINISH THIS CLASS!
//       FINISH THIS CLASS!
//       FINISH THIS CLASS!
//       FINISH THIS CLASS!
//       FINISH THIS CLASS!
//       FINISH THIS CLASS!
//       FINISH THIS CLASS!


// namespace schwa::grfx
namespace schwa {namespace grfx {


class Mesh : public Renderer::Resource {
 public:
    class Format {
    public:
        //        struct {


    };

 public:
    Mesh(const shared_ptr<Renderer>& renderer, const shared_ptr<Format>& format);



    void draw();

 protected:
    shared_ptr<Format> _format;

    GLuint _vao;
    GLuint _vertices;
    GLuint _indices;
};


class StrokeFormat : public Mesh::Format {


};


/*

Shape {

 protected:
  Mesh _mesh;
  Shader _
 
  Material _
 
  MaterialBinding _binding;

}
 

MeshBinding {
  NewBinding(Mesh::Format mesh, Shader shader) {
    var binding = new Binding();
    for (varying : mesh.varyingParameters) {
      binding->
    }
  }
 
 MeshBinding(
 
}
 
 
 
 
 typedef struct format
 {
 GLfloat x;
 GLfloat y;
 GLfloat nx;
 GLfloat ny;
 GLfloat length;
 } format;
*/



}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__mesh__
