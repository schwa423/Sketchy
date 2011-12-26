//
//  Renderer.h
//  Sketchy
//
//  Created by Joshua Gargus on 12/24/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Renderer_h
#define Sketchy_Renderer_h

@class RenderTarget;
@class CAEAGLLayer;

class Renderer {
public:
	Renderer();
	
private:
	EAGLContext *m_context;
	CADisplayLink *m_displayLink;
};

#endif
