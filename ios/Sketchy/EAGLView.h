//
//  EAGLView.h
//  Sketchy
//
//  Created by Joshua Gargus on 12/24/11.
//  Copyright (c) 2011 Schwaftwarez. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#include "Renderer.h"
#include <memory>

@class EAGLContext;

@interface EAGLView : UIView {
@private
	std::shared_ptr<Renderer> m_renderer;
}

- (void)pauseRendering;
- (void)unpauseRendering;
@end
