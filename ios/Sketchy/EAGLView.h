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

#include <memory>

namespace Sketchy {
	class Renderer;
}

@class EAGLContext;

@interface EAGLView : UIView {
@private
	std::shared_ptr<Sketchy::Renderer> m_renderer;
}

- (void)pauseRendering;
- (void)unpauseRendering;
@end
