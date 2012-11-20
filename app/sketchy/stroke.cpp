//
//  stroke.cpp
//  schwa::app::sketchy
//
//  Created by Josh Gargus on 10/27/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "stroke.h"
#include "strokeshader.h"


// namespace schwa::app::sketchy
namespace schwa {namespace app {namespace sketchy {


// static
shared_ptr<grfx::Buffer> Stroke::_indexBuffer;


StrokeMesh::~StrokeMesh() {
    // TODO: clean up VAO... destroying this mesh doesn't imply that the Renderer will also be.
    if (_vao)
        cerr << "WARNING: must clean up VAOs when destroying stroke-mesh" << endl;
}


void StrokeMesh::draw(Renderer_ptr renderer) {
    bindVAO(renderer);
    if (!_vaoValid) return;
    glDrawElements(GL_TRIANGLES, _triangleCount * 3 / 2, GL_UNSIGNED_INT, 0);

    // TODO: necessary?
    glBindVertexArrayOES(0);
}


void StrokeMesh::setBuffers(unique_ptr<grfx::Buffer> vertices,
                            shared_ptr<grfx::Buffer> indices,
                            uint32_t startIndex,
                            uint32_t triangleCount) {
    _vertices = std::move(vertices);
    _indices = indices;
    _startIndex = startIndex;
    _triangleCount = triangleCount;
}


// TODO: move this into schwa::util
template<typename T1, typename T2>
static inline GLsizeiptr BYTE_OFFSET(const T1& base, const T2& offset) {
    const uint8_t* base_ptr = reinterpret_cast<const uint8_t*>(&base);
    const uint8_t* offset_ptr = reinterpret_cast<const uint8_t*>(&offset);
    return offset_ptr - base_ptr;
}


void StrokeMesh::bindVAO(Renderer_ptr& renderer) {
    // If we have a valid VAO, simply bind it.  Done.
    if (_vaoValid) {
        glBindVertexArrayOES(_vao);
        return;
    }

    ALWAYS_CHECK_GL("StrokeMesh::bindVAO().. about to initialize VAO");

    // If we don't have a VAO, generate one.
    if (!_vao) {
        glGenVertexArraysOES(1, &_vao);
        if (!_vao) {
            cerr << "WARNING: glGenVertexArraysOES() failed in StrokeMesh::bindVAO" << endl;
            return;
        }
    }

    // Bind vertex attributes.
    glBindVertexArrayOES(_vao);
    Stroke::Vertex v;  // for offset/stride calculations.
    GLuint attrib = StrokeShader::POS_AND_NORM;
    _vertices->bindVertexAttrib(attrib, 4, GL_FLOAT, sizeof(v), 0);
    glEnableVertexAttribArray(attrib);
    attrib = StrokeShader::LENGTH_ETC;
    _vertices->bindVertexAttrib(attrib, 1, GL_FLOAT, sizeof(v), BYTE_OFFSET(v.x, v.length));
    glEnableVertexAttribArray(attrib);

    // Bind indices.
    Stroke::indexBuffer(renderer, _triangleCount)->bind();

    // Unbind buffer.  Not sure why this is necessary, but the Apple example
    // does this.  I suppose that if the bound buffer is always 0 for every VAO
    // that we create, then binding a VAO never needs to change the bound buffer.
    glBindBuffer(GL_ARRAY_BUFFER,0);

    // Check whether we were successful, and set validity accordingly.
    _vaoValid = ALWAYS_CHECK_GL("StrokeMesh::bindVAO()... failed to initialize VAO");
}


Stroke::Stroke(const std::vector<Sketchy::Geom::Arc>& path,
               const shared_ptr<StrokeShader>& shader) : _pathLength(0.f), _dirty(true) {
    setWidth(5.f);
    setColor(0.f, 0.f, 0.f);
    setPath(path);
    setShader(shader);
}


const shared_ptr<grfx::Buffer>& Stroke::indexBuffer(Renderer_ptr& renderer, int triangleCount) {
    // TODO: assert this happens on the render-thread.  This shouldn't be a
    //       problem, since we only need it to create VAOs at render-time.

    // Check if we can re-use existing index-buffer, or if a new one is required.
    if (!_indexBuffer || (_indexBuffer->size() < (triangleCount * 3 * sizeof(GLuint)))) {
        // Make the new index-buffer twice as large as requested, since it's quite
        // likely that a larger index buffer will be requested soon.  This also guarantees
        // that the index-count is divisible by 6, which we otherwise would need to check.
        int indexCount = triangleCount * 3 * 2;

        std::unique_ptr<GLuint[]> indices(new GLuint[indexCount]);
        for (int i = 0; i < indexCount/6; i++) {
            indices[i*6 + 0] = i*2 + 0;
            indices[i*6 + 1] = i*2 + 1;
            indices[i*6 + 2] = i*2 + 2;
            indices[i*6 + 3] = i*2 + 1;
            indices[i*6 + 4] = i*2 + 3;
            indices[i*6 + 5] = i*2 + 2;
        }

        if (!_indexBuffer)
            _indexBuffer.reset(new grfx::GpuBuffer(renderer, GL_ELEMENT_ARRAY_BUFFER));

        _indexBuffer->load(std::move(indices), indexCount * sizeof(GLuint), GL_STATIC_DRAW);
    }

    return _indexBuffer;
}


void Stroke::clearIndexBuffer() {
    _indexBuffer.reset();
}


// Create a single example stroke.
shared_ptr<Stroke> Stroke::example1(const shared_ptr<StrokeShader>& shader) {
    float start_angle = M_PI*7/6;
    Sketchy::Geom::Point2d start_pos(-110.0f, -110.0);
    std::vector<float> radii{150.0, 90.0, 30.0, 45.0, 120.0};
    std::vector<float> end_angles{M_PI/2, M_PI/-2, M_PI*5/-4, M_PI*-1, 0};

    auto path = Sketchy::Geom::Arc::piecewisePath(start_angle, start_pos, radii, end_angles);

    auto stroke = new Stroke(path, shader);
    GLfloat color[4] {0.5f, 0.87f, 0.5f, 0.08f};
    stroke->setColor(color);
    stroke->setWidth(10.f);

    return shared_ptr<Stroke>(stroke);
}


// Create a list of example strokes.  The path of each one consists as a single arc;
// each arc corresponds to an arc in the stroke-path from example1().
vector<shared_ptr<Stroke>> Stroke::example2(const shared_ptr<StrokeShader>& shader) {
    float start_angle = M_PI*7/6;
    Sketchy::Geom::Point2d start_pos(-110.0f, -110.0);
    std::vector<float> radii{150.0, 90.0, 30.0, 45.0, 120.0};
    std::vector<float> end_angles{M_PI/2, M_PI/-2, M_PI*5/-4, M_PI*-1, 3.};

    auto path = Sketchy::Geom::Arc::piecewisePath(start_angle, start_pos, radii, end_angles);

    // Use distinct colors for each stroke.
    vector<GLfloat> rgbs { 0.0f, 0.0f, 0.0f,
                           1.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f,
                           0.0f, 0.0f, 1.0f,
                           1.0f, 1.0f, 0.0f,
                           1.0f, 0.0f, 1.0f,
                           0.0f, 1.0f, 1.0f,
                           1.0f, 1.0f, 1.0f };
    GLfloat color[4];
    color[3] = 1.0f;

    std::vector<shared_ptr<Stroke>> results;
    for (auto& arc : path) {
        std::vector<Sketchy::Geom::Arc> oneArcPath;
        oneArcPath.push_back(arc);
        auto rawStroke = new Stroke(oneArcPath, shader);
        shared_ptr<Stroke> stroke(rawStroke);
        color[0] = rgbs.back(); rgbs.pop_back();
        color[1] = rgbs.back(); rgbs.pop_back();
        color[2] = rgbs.back(); rgbs.pop_back();
        stroke->setColor(color);
        stroke->setWidth(5.f);
        results.push_back(stroke);
    }

    return results;
}


void Stroke::setPath(const std::vector<Sketchy::Geom::Arc>& path) {
    // TODO: why can't we copy the contents from one vector to another (avoiding push_back() below)?
    //       _path.assign(path.begin(), path.end());
    _dirty = true;
    _path.clear();
    _pathLength = 0.f;
    for (auto& arc : path) {
        _pathLength += arc.length();
        _path.push_back(arc);
    }
}


void Stroke::setShader(const shared_ptr<StrokeShader>& shader) {
    // TODO: assert not nullptr
    _shader = shader;
}


void Stroke::setWidth(float width) {
    _width = width;
}


void Stroke::setColor(float red, float green, float blue, float alpha) {
    _color[0] = red;
    _color[1] = green;
    _color[2] = blue;
    _color[3] = alpha;
}


void Stroke::draw(Renderer_ptr renderer, uint64_t time) {
    _shader->setWidth(_width);
    _shader->setColor(_color);
    _shader->setStartLength(0.0);
    _shader->setEndLength(_pathLength);

    _shader->bind();

    // TODO: make this async, instead of tesselating at render-time.
    if (_dirty) {
        // TODO: need better LOD calculation.
        int vertexCount = static_cast<int>(_pathLength);
        int triangleCount = (vertexCount - 2) * 2;

        std::unique_ptr<Vertex[]> vertices(new Vertex[vertexCount]);

        // TODO: this should be a static Arc function;
        tesselate(vertices.get(), vertexCount, _path);

        unique_ptr<grfx::Buffer> vertexBuffer(new grfx::GpuBuffer(renderer, GL_ARRAY_BUFFER));
        vertexBuffer->load(std::move(vertices), vertexCount * sizeof(Vertex), GL_STATIC_DRAW);

        _mesh.reset(new StrokeMesh());
        _mesh->setBuffers(std::move(vertexBuffer),
                          Stroke::indexBuffer(renderer, triangleCount),
                          0, triangleCount);
        _dirty = false;
    }
    _mesh->draw(renderer);
}


void Stroke::destroyRendererState(grfx::Renderer_ptr r) {
    _mesh.reset();
    _dirty = true;
}


}}}  // namespace schwa::app::sketchy