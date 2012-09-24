//
//  SchwaGLView
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
	class Renderer_iOS;
	class Page;
}

// namespace schwa::grfx
namespace schwa { namespace grfx {
    class Renderer_iOS;
    class View;
}}  // namespace schwa::grfx
using namespace schwa;

@class EAGLContext;

@interface SchwaGLView : UIView {
@private
    shared_ptr<grfx::Renderer_iOS> _renderer;
    shared_ptr<grfx::View> _view;
}

- (void)startRendering;
- (void)stopRendering;

// Notify renderer that the orientation has changed.
- (void)updateOrientation:(UIInterfaceOrientation)orientation;

@end
