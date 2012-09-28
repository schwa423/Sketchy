//
//  renderbuffer_ios.h
//  schwa::grfx
//
//  Created by Josh Gargus on 9/23/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__grfx__renderbuffer_ios__
#define __schwa__grfx__renderbuffer_ios__

#include "renderbuffer.h"

#include <QuartzCore/QuartzCore.h>

// namespace schwa::grfx
namespace schwa {namespace grfx {


class Renderbuffer_iOS : public Renderbuffer {
 public:
    // "Constructors".
    // TODO: consider making these instance-methods of Renderer.
    static shared_ptr<Renderbuffer> NewFromLayer(shared_ptr<Renderer> renderer,
                                                 EAGLContext* context,
                                                 CAEAGLLayer* layer);
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__renderbuffer_ios__