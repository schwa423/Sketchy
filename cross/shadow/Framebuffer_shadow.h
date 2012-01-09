//
//  Framebuffer_shadow.h
//  Sketchy
//
//  Created by Joshua Gargus on 1/8/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

//  This is a "shadow" class, meaning there is a 1-to-1 relationship between
//  each instance and an instance of the corresponding non-shadow class.  
//  Operations on shadow instances only happen in the Renderer thread.  This
//  means that is safe, for instance, to interact with the Renderer's OpenGL
//  context because we're running in the same thread.
//
//  I haven't formalized the rules for using shadows, but here are some thoughts:
//
//  1) Shadows don't have a reference back to their non-shadow object.  This 
//     would create a reference-loop that would have to be explicitly broken
//     in order to avoid leaking memory.  It would be possible to use a weak_ptr
//     but then we'd have to synchronize access to the non-shadow, since we don't
//     know what other threads might be accessing it.  Since the whole point of 
//     shadows it to provide a lockless synchronization mechanism with the render-
//     thread, this would be silly.
//
//  2) Non-shadows never directly modify their shadow, or even directly access
//     its properties.  Once created, the shadow is only accessed by wrapping it
//     in tasks that are executed in the Renderer-loop.  Even the ultimate destruction
//     of the shadow (triggered by the death of its non-shadow) doesn't happen 
//     immediately, but rather as the result of a Renderer-task.
//     TODO: does this happen explicitly, or as the result of the shadow's ref-count
//           dropping to zero within the Renderer-loop? 


#ifndef Sketchy_Framebuffer_shadow_h
#define Sketchy_Framebuffer_shadow_h

@class CAEAGLLayer;

#include <OpenGLES/ES2/gl.h>
#include <memory>


namespace Sketchy { 
	class Renderer;

namespace Shadow {

class Framebuffer {
public:
	Framebuffer(std::shared_ptr<Renderer>);
	
	virtual void createOnscreen(CAEAGLLayer *layer, bool useDepth);
	// TODO:		virtual void createOffscreen();	
	
	void bind();
	void present();

protected:
	// TODO: virtual void destroy();
	virtual void deleteBuffers();
	
	std::weak_ptr<Renderer> m_renderer;
	
	GLuint m_framebuffer, m_color, m_depth;
	GLint m_width, m_height;
	EAGLContext *m_context;
	
}; // class Framebuffer


// TODO:
// class MultisampleFramebuffer {};
	
}} // namespace Sketchy::Shadow

#endif
