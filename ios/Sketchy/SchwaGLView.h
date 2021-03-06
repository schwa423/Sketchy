//
//  SchwaGLView
//  Sketchy
//
//  Created by Joshua Gargus on 12/24/11.
//  Copyright (c) 2011 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

#import <UIKit/UIKit.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#include <memory>
using std::shared_ptr;

namespace Sketchy {
	class Page;
}

// namespace schwa::grfx
namespace schwa { namespace grfx {
    class Renderer_iOS;
    class LayerPresenter_iOS;
}}  // namespace schwa::grfx
using namespace schwa;

@class EAGLContext;

@interface SchwaGLView : UIView {
@private
    shared_ptr<grfx::LayerPresenter_iOS> _presenter;

    // TODO: eventually the renderer will not be associated with any single view.
    shared_ptr<grfx::Renderer_iOS> _renderer;
}

- (const shared_ptr<grfx::LayerPresenter_iOS>&) presenter;
// TODO: this shouldn't be necessary once renderer is shared by all views in the app.
- (const shared_ptr<grfx::Renderer_iOS>&) renderer;

- (void)startRendering;
- (void)stopRendering;

// Notify renderer that the orientation has changed.
- (void)updateOrientation:(UIInterfaceOrientation)orientation;

@end
