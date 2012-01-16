//
//  Geometry.h
//  Sketchy
//
//  Created by Joshua Gargus on 1/2/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Geometry_h
#define Sketchy_Geometry_h

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
	enum StrokeProps { POS_AND_NORM = 0, LEN_AND_TIME };
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
		float startRadians = 2 * M_PI / 3;
		float endRadians = M_PI / 3;
		float radius = 100.0;
		
		GLsizei vertexCount = vertexCountForArc(radius, startRadians-endRadians);
		m_count = (vertexCount-2) / 2 * 3;
		
		GLfloat *vertices = new GLfloat[vertexCount*sizeof(Geometry::format)/sizeof(GLfloat)];
		GLuint *indices = new GLuint[m_count]; 
	
		generateArc(radius, startRadians, endRadians, (Geometry::format*)vertices);
		
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
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count*sizeof(GLuint), indices, GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(POS_AND_NORM); 
		glEnableVertexAttribArray(LEN_AND_TIME);
		
#define BUFFER_OFFSET(i) ((char *)NULL + (i*sizeof(GLfloat)))
		glVertexAttribPointer(POS_AND_NORM, 4, GL_FLOAT, GL_FALSE, sizeof(Geometry::format), BUFFER_OFFSET(0));
		glVertexAttribPointer(LEN_AND_TIME, 1, GL_FLOAT, GL_FALSE, sizeof(Geometry::format), BUFFER_OFFSET(4));
#undef BUFFER_OFFSET
		
		glBindVertexArrayOES(0);
		
		delete[] vertices;
		delete[] indices;		
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
	
	
	int vertexCountForArc(float radius, float radians) {
		// vertex count must be divisible by 2
		return ((int)(radius * abs(radians) / M_PI)) * 2;
	}
	void generateArc(float radius, float startRadians, float endRadians, Geometry::format *out) {
		int count = vertexCountForArc(radius, startRadians-endRadians);
		// TODO: what if count == 2?  Divide by zero...
		float diff = (endRadians - startRadians) / (count/2-1);
		
		// We want to be able to use the same index-buffer to render
		// an arc-segment as clockwise-winding triangles, whether the
		// arc-segment itself is clockwise or widdershins.  Therefore,
		// we must account for this in the generation of the vertex
		// normals.
		float normalFlip = (startRadians > endRadians) ? -1.0f : 1.0f;
		
		for (int i=0; i < count; i+=2) {
			Geometry::format *ptr1 = &(out[i]);
			Geometry::format *ptr2 = &(out[i+1]);
			float rad = startRadians + i*diff;
			float nx = cos(rad);
			float ny = sin(rad);
			ptr1->x = ptr2->x = radius * nx;
			ptr1->y = ptr2->y = radius * ny;
			ptr1->nx = nx * normalFlip;
			ptr1->ny = ny * normalFlip;
			ptr2->nx = -nx * normalFlip;
			ptr2->ny = -ny * normalFlip;
			ptr1->length = ptr2->length = i*diff * M_2_PI;
		}		
	}
}; // class Geometry

} // namespace Sketchy

#endif
