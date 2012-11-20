//
//  stroke.h
//  schwa::app::sketchy
//
//  Created by Josh Gargus on 10/27/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__app__sketchy__stroke__
#define __schwa__app__sketchy__stroke__


#include "strokeshader.h"
#include "mesh.h"

// TODO: move this into schwa::
#include "Arc.h"


// namespace schwa::app::sketchy
namespace schwa {namespace app {namespace sketchy {


class StrokeShader;
using grfx::Renderer_ptr;

class StrokeMesh {
 public:
    StrokeMesh() : _vaoValid(false), _vao(0) { }
    ~StrokeMesh();

    // Bind VAO and draw elements.
    void draw(Renderer_ptr renderer);

    void setBuffers(unique_ptr<grfx::Buffer> vertices,
                    shared_ptr<grfx::Buffer> indices,
                    uint32_t startIndex,
                    uint32_t triangleCount);

 protected:
    void bindVAO(Renderer_ptr renderer);

    unique_ptr<grfx::Buffer> _vertices;
    shared_ptr<grfx::Buffer> _indices;
    uint32_t _startIndex;
    uint32_t _triangleCount;

    GLuint _vao;
    bool _vaoValid;
};


class Stroke {
 public:
    // TODO: a shader should probably not be required to instantiate a stroke.
    Stroke(const std::vector<Sketchy::Geom::Arc>& path,
           const shared_ptr<StrokeShader>& shader);

    // Defines the layout of stroke data passed to the GPU.
    struct Vertex {
        GLfloat x;
        GLfloat y;
        GLfloat nx;
        GLfloat ny;
        GLfloat length;
    };

    // Return a vertex-index buffer of at least the requested size.
    // Since all strokes have the same structure, all can share a single index-buffer.
    // TODO: feels ugly to have to pass renderer in.
    static const shared_ptr<grfx::Buffer>& indexBuffer(Renderer_ptr renderer, int triangleCount);
    static void clearIndexBuffer();

    // Set the shader to render this stroke with.
    void setShader(const shared_ptr<StrokeShader>& shader);

    // Set various properties of the stroke (width, color, and so forth)
    void setWidth(float width);
    void setColor(float red, float green, float blue, float alpha = 1.f);
    void setColor(float color[4]) { setColor(color[0], color[1], color[2], color[3]); }

    void draw(Renderer_ptr renderer, uint64_t time);

    void destroyRendererState(grfx::Renderer_ptr r);

 public:
    // Procedurally generate example Strokes.
    static shared_ptr<Stroke> example1(const shared_ptr<StrokeShader>&);
    static vector<shared_ptr<Stroke>> example2(const shared_ptr<StrokeShader>&);

 protected:
    void setPath(const std::vector<Sketchy::Geom::Arc>& path);

    std::vector<Sketchy::Geom::Arc> _path;
    double _pathLength;

    // There will eventually be multiple meshes for different LODs, but for now a single one is OK.
    shared_ptr<StrokeMesh> _mesh;
    shared_ptr<StrokeShader> _shader;
    bool _dirty;

    // TODO: instead of being static, this should probably be a weakly-referenced
    //       Renderer resource so that:
    //       - so that we can explicitly invalidate it when the renderer is destroyed
    //       - it can be auto-destroyed if nobody is using it
    static shared_ptr<grfx::Buffer> _indexBuffer;

    // Stroke properties to be bound as shader-uniforms.
    float _width;
    float _color[4];
};


}}}  // namespace schwa::app::sketchy


#endif  // #ifndef __schwa__app__sketchy__stroke__
