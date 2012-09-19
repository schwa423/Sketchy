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
#else
#error UNKNOWN PLATFORM
#endif

#define CHECK_GL(msg) { GLenum err = glGetError(); \
if (err != GL_NO_ERROR) cerr << "GLError(" << err << "): " << msg << endl; }

#endif  // #ifndef __schwa__grfx__platform_gl__
