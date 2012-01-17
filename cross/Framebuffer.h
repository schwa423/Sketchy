//
//  Framebuffer.h
//  Sketchy
//
//  Created by Joshua Gargus on 1/6/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

// TODO: when using this class to create a framebuffer from the
//       CAEAGLLayer, you should wait for the operation to finish
//       on the shadow, because you don't want the layer-pointer
//       to be invalid by the time that the shadow tries to do its
//       job.  How can we make this less error-prone?  An alternative
//       would be to create fences internally, and wait for the 
//       shadow to finish.  The problem with this is that it would
//       prevent the current thread from doing any other work.  But,
//       we create on-screen framebuffers very infrequently, so we 
//       can probably spare the few milliseconds.
//
//       Verdict: block until on-screen framebuffer is fully created.

#ifndef Sketchy_Framebuffer_h
#define Sketchy_Framebuffer_h

#include <memory>

#include "shadow/Framebuffer_shadow.h"
#include "Renderer.h"

namespace Sketchy {
	
class Renderer;

class Framebuffer : public std::enable_shared_from_this<Framebuffer>
{
public:
	// TODO: try to make this unnecessary
	friend class Renderer;

	Framebuffer(shared_ptr<Renderer> renderer, EAGLContext *context, CAEAGLLayer *layer, bool useDepth);
	// TODO: offscreen framebuffer
	// Framebuffer(shared_ptr<Renderer> renderer, int width, int height, bool useDepth);
	~Framebuffer();

	// TODO: shorter type name? typedef?
	// TODO: I don't like the idea of the shadow pointer being
	//       accessible via a public API.  What to do?  Perhaps
	//       an implicit coercsion from a non-shadow to its shadow
	//       counterpart where appropriate?
	shared_ptr<Shadow::Framebuffer> shadow() { return m_shadow; }

private:

	weak_ptr<Renderer> m_renderer;
	shared_ptr<Shadow::Framebuffer> m_shadow;
	
	int m_width, m_height;
	bool m_useDepth;
	
	// Task used to schedule framebuffer destruction within Renderer loop.
	// Doesn't need to do anything other than hold onto the reference to 
	// the shadow long enough to release it within the Renderer-loop so
	// that it gets destroyed there.
	// TODO: unit test this!
	class Destroy : public Event {
	public:
		Destroy(shared_ptr<Shadow::Framebuffer> shadow) : m_shadow(shadow) { };
		virtual void reallyRun() { };
	private:
		shared_ptr<Shadow::Framebuffer> m_shadow;
	}; // class Framebuffer::Destroy
	
}; // class Framebuffer


// TODO:
// class MultisampleFramebuffer { };
	
} // namespace Sketchy

#endif
