//
//  renderbuffer_ios.mm
//  schwa::grfx
//
//  Created by Josh Gargus on 9/23/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "renderbuffer_ios.h"

#include <iostream>
using std::cerr;
using std::endl;

// namespace schwa::grfx
namespace schwa {namespace grfx {


shared_ptr<Renderbuffer> Renderbuffer_iOS::NewFromLayer(shared_ptr<Renderer> renderer,
                                                    EAGLContext* context,
                                                    CAEAGLLayer* layer) {
    CGRect rect = [layer bounds];
    int width = (int)rect.size.width;
    int height = (int)rect.size.height;

    GLuint handle;
    glGenRenderbuffers(1, &handle);
    glBindRenderbuffer(GL_RENDERBUFFER, handle);
    BOOL result = [context
                   renderbufferStorage:GL_RENDERBUFFER
                   fromDrawable:layer];
    if (result != YES) {
        cerr << "failed to create color_renderbuffer from drawable layer" << endl;
        return nullptr;
    }

    return shared_ptr<Renderbuffer>(new Renderbuffer(renderer, handle,
                                                     width, height,
                                                     1, false));
}


}}  // namespace schwa::grfx