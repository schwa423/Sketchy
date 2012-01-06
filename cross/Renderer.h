//
//  Renderer.h
//  Sketchy
//
//  Created by Joshua Gargus on 12/24/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

// TODO: abstract out iOS-specific code

#ifndef Sketchy_Renderer_h
#define Sketchy_Renderer_h

#include "Event.h"
#include "Loop.h"

// TODO: remove these if unnecessary once we stop hacking around
#include "Geometry.h"
#include "Shader.h"

#include <OpenGLES/ES2/gl.h>

@class CADisplayLinkListener;
@class CAEAGLLayer;

class Renderer : public Loop<Event::ptr> {
public:
	Renderer(CAEAGLLayer *layer);
	~Renderer();

	void render(void) {
		Event::ptr render(new Render(this));
		addTask(render);
	}
	
	void pauseRendering(void);
	void unpauseRendering(void);
	
private:
	__strong EAGLContext *m_context;
	__strong CADisplayLinkListener *m_displayLinkListener;

	// TODO make a framebuffer class that encapsulates this state?
	GLuint m_framebuffer, m_renderbuffer;
	GLint m_framebufferWidth, m_framebufferHeight;

	Geometry *m_hackGeometry;
	Shader *m_hackShader;
	
	void handleInit(void);
	void handleCreateFramebuffer(CAEAGLLayer *layer);
	void handleRender(void);

	void deleteFramebuffer(void);

	// Many events processed by the renderer
	// will need a reference to the renderer.
	class RendererEvent : public Event {
	public:
		RendererEvent(Renderer *r) : m_renderer(r) { }
	protected:
		Renderer* m_renderer;
	};

	class Init : public RendererEvent {
	public:
		Init(Renderer *r) : RendererEvent(r) { };
		virtual void reallyRun() { m_renderer->handleInit(); }
	};

	class CreateFramebuffer : public RendererEvent {
	public:
		CreateFramebuffer(Renderer *r, CAEAGLLayer *lay) :
			RendererEvent(r),
			m_layer(lay)
			{ };
		virtual void reallyRun() { m_renderer->handleCreateFramebuffer(m_layer); }
	private:
		CAEAGLLayer *m_layer;
	};

	class Render : public RendererEvent {
	public:
		Render(Renderer *r) : RendererEvent(r) { };
		virtual void reallyRun() { m_renderer->handleRender(); }
	};
	
	
};

#endif
