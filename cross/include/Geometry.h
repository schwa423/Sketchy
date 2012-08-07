//
//  Geometry.h
//  Sketchy
//
//  Created by Joshua Gargus on 1/2/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Geometry_h
#define Sketchy_Geometry_h

#include "geom/Arc.h"

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <cmath>
#include <iostream>
using std::cerr;
using std::endl;

namespace Sketchy {

class Geometry
{
public:
	enum StrokeProps { POS_AND_NORM = 0, LEN_AND_TIME, COLOR };
	Geometry() :
		m_vao(0),
		m_vboGeom(0),
		m_vboIndex(0) 
	{
		generateStroke();
	} 
	~Geometry() {

	}
	void draw(void) {
		glBindVertexArrayOES(m_vao);
		glDrawElements(GL_TRIANGLES, m_count, GL_UNSIGNED_INT, 0);
		glBindVertexArrayOES(0);
	}

private:
    void generateStroke() {
        int vertex_count = 1250;
        m_count = (vertex_count-2) * 3;

        float start_angle = M_PI*7/6;
        Geom::Point2d start_pos(-110.0f, -110.0);
        std::vector<float> radii{150.0, 90.0, 30.0, 45.0, 120.0};
        std::vector<float> end_angles{M_PI/2, M_PI/-2, M_PI*5/-4, M_PI*-1, 0};

        auto path = Geom::Arc::piecewisePath(start_angle, start_pos, radii, end_angles);

        std::unique_ptr<Geometry::format[]> vertices(new Geometry::format[vertex_count]);
        tesselate(vertices.get(), vertex_count, path);

        generateIndicesAndVAO(vertex_count, reinterpret_cast<GLfloat*>(vertices.get()));
    }

    void generateIndicesAndVAO(int vertexCount, GLfloat * vertices) {
        std::unique_ptr<GLuint[]> indices(new GLuint[m_count]);

        for (int i = 0; i < m_count/6; i++) {
            indices[i*6 + 0] = i*2 + 0;
            indices[i*6 + 1] = i*2 + 1;
            indices[i*6 + 2] = i*2 + 2;
            indices[i*6 + 3] = i*2 + 1;
            indices[i*6 + 4] = i*2 + 3;
            indices[i*6 + 5] = i*2 + 2;
        }

        m_vao = 0;

        glGenVertexArraysOES(1, &m_vao);
        glGenBuffers(1, &m_vboGeom);
        glGenBuffers(1, &m_vboIndex);

        glBindVertexArrayOES(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vboGeom);
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Geometry::format), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboIndex);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count*sizeof(GLuint), indices.get(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(POS_AND_NORM);
        glEnableVertexAttribArray(LEN_AND_TIME);

#define BUFFER_OFFSET(i) ((char *)NULL + (i*sizeof(GLfloat)))
        glVertexAttribPointer(POS_AND_NORM, 4, GL_FLOAT, GL_FALSE, sizeof(Geometry::format), BUFFER_OFFSET(0));
        glVertexAttribPointer(LEN_AND_TIME, 1, GL_FLOAT, GL_FALSE, sizeof(Geometry::format), BUFFER_OFFSET(4));
#undef BUFFER_OFFSET

        glBindVertexArrayOES(0);
	}

    void generateTriangle() {
        GLfloat vertices[] = {
            0.0f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f
        };
        GLuint indices[] = { 0, 1, 2 };

        glGenVertexArraysOES(1, &m_vao);
        glGenBuffers(1, &m_vboGeom);
        glGenBuffers(1, &m_vboIndex);

        glBindVertexArrayOES(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vboGeom);
        glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboIndex);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*sizeof(GLuint), indices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);

#define BUFFER_OFFSET(i) ((char *)NULL + (i*sizeof(GLfloat)))
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
#undef BUFFER_OFFSET

        glBindVertexArrayOES(0);

        m_count = 3;
    }

    typedef struct format
    {
        GLfloat x;
        GLfloat y;
        GLfloat nx;
        GLfloat ny;
        GLfloat length;
    } format;

    GLuint m_vao;
    GLuint m_vboGeom;
    GLuint m_vboIndex;
    GLsizei m_count;

}; // class Geometry

} // namespace Sketchy

#endif
