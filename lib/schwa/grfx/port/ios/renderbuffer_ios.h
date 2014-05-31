//
//  renderbuffer_ios.h
//  schwa::grfx
//
//  Created by Josh Gargus on 2/16/14.
//  Copyright (c) 2014 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//


#ifndef __schwa__grfx__port__ios__renderbuffer_ios__
#define __schwa__grfx__port__ios__renderbuffer_ios__

#include "renderbuffer.h"

#include <QuartzCore/QuartzCore.h>

// namespace schwa::grfx
namespace schwa {namespace grfx {

RenderbufferPtr NewRenderbufferFromLayer(const RendererPtr& renderer,
                                         EAGLContext* context,
                                         CAEAGLLayer* layer);
}}  // namespace schwa::grfx

#endif  // #ifndef __schwa__grfx__port__ios__renderbuffer_ios__
