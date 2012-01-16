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
using std::shared_ptr;

namespace Sketchy {
	class Renderer;
	class Page;
}

@class EAGLContext;

@interface EAGLView : UIView {
@private
	shared_ptr<Sketchy::Renderer> m_renderer;
	shared_ptr<Sketchy::Page> m_page;
}

- (void)pauseRendering;
- (void)unpauseRendering;
@end
