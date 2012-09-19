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

// namespace schwa::grfx
namespace schwa { namespace grfx {
    class Renderer;
    class View;
}}  // namespace schwa::grfx
using namespace schwa;

@class EAGLContext;

@interface EAGLView : UIView {
@private

    // New shit.  Yeah!
    shared_ptr<grfx::Renderer> _renderer;
    shared_ptr<grfx::View> _view;

    // Old shit.  Booo.
	shared_ptr<Sketchy::Renderer> _renderer2;
	shared_ptr<Sketchy::Page> _page2;
}

- (void)pauseRendering;
- (void)unpauseRendering;

// Hacky way to get at the page from the controller.
- (shared_ptr<Sketchy::Page>)page2;
- (shared_ptr<Sketchy::Page>)page;

@end
