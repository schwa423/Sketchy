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


shared_ptr<Renderbuffer> Renderbuffer_iOS::NewFromLayer(const shared_ptr<Renderer>& renderer,
                                                    EAGLContext* context,
                                                    CAEAGLLayer* layer) {
    CGRect rect = [layer bounds];
    int width = (int)rect.size.width;
    int height = (int)rect.size.height;

    GLuint handle;
    glGenRenderbuffers(1, &handle);
    glBindRenderbuffer(GL_RENDERBUFFER, handle);

    // First detach from any previous renderbuffer.
    // TODO: Not sure if necessary, but seems prudent.
    if (YES != [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:nil]) {
        cerr << "failed to clear renderbuffer storage somehow (whaaaa??)" << endl;
    }

    if (YES != [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer]) {
        cerr << "failed to create color_renderbuffer from drawable layer" << endl;
        return nullptr;
    }

    return shared_ptr<Renderbuffer>(new Renderbuffer_iOS(renderer,
                                                         handle,
                                                         width, height,
                                                         1, GL_RGBA8_OES));
}


Renderbuffer_iOS::Renderbuffer_iOS(shared_ptr<Renderer> renderer, GLuint handle,
                                   uint width, uint height, int samples, GLenum format)
    : Renderbuffer(renderer, handle, width, height, samples, format) {

}


}}  // namespace schwa::grfx