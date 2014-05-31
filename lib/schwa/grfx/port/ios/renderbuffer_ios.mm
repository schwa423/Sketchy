//
//  renderbuffer_ios.mm
//  Sketchy
//
//  Created by Josh Gargus on 2/16/14.
//
//

#include "renderbuffer_ios.h"


// namespace schwa::grfx
namespace schwa {namespace grfx {

namespace {
// TODO: the only reason this exists is to expose a protected constructor... is there a better way?
class Renderbuffer_iOS : public Renderbuffer {
 public:
    Renderbuffer_iOS(const RendererPtr& renderer, GLuint handle,
                     uint width, uint height, int samples, GLenum format)
    : Renderbuffer(renderer, handle, width, height, samples, format) { }
};
}  // anonymous namespace
    
RenderbufferPtr NewRenderbufferFromLayer(const RendererPtr& renderer,
                                         EAGLContext* context,
                                         CAEAGLLayer* layer) {
    std::cerr << "!!!!!!!! NewRenderbufferFromLayer" << std::endl;
    
    CGRect rect = [layer bounds];
    CGFloat scale = [[UIScreen mainScreen] scale];
    int width = (int)rect.size.width * scale;
    int height = (int)rect.size.height * scale;
    
    GLuint handle;
    glGenRenderbuffers(1, &handle);
    glBindRenderbuffer(GL_RENDERBUFFER, handle);
    
    if (YES != [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer]) {
        std::cerr << "failed to create color_renderbuffer from drawable layer" << std::endl;
        return nullptr;
    }
    
    CHECK_GL("NewRenderBuffer() failed to create renderbuffer from iOS layer");
    
    return RenderbufferPtr(new Renderbuffer_iOS(renderer,
                                                handle,
                                                width, height,
                                                1, GL_RGBA8_OES));
}

}}  // namespace schwa::grfx
