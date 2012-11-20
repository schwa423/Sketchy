//
//  platform_gl.h
//  schwa::grfx
//
//  Created by Josh Gargus on 9/11/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__grfx__platform_gl__
#define __schwa__grfx__platform_gl__

#if defined(__APPLE__)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#error UNKNOWN PLATFORM
#endif

#include <iostream>


// Check for GL error in both Debug and Release mode.
inline bool ALWAYS_CHECK_GL(const char* msg) {
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) return true;
    std::cerr << "GLError(" << err << "): " << msg << std::endl;
    return false;
}

inline bool CHECK_GL(const char* msg) {
#if defined(DEBUG)
    return ALWAYS_CHECK_GL(msg);
#else
    return true;
#endif
}

#endif  // #ifndef __schwa__grfx__platform_gl__
