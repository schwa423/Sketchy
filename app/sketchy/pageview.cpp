//
//  pageview.cpp
//  schwa::app::sketchy
//
//  Created by Josh Gargus on 9/16/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "pageview.h"
#include "renderer2.h"

#include <iostream>
using std::cerr;
using std::endl;

// namespace schwa::app::sketchy
namespace schwa {namespace app {namespace sketchy {

void PageView::render() {
    glClearColor(0.5f, 0.5f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void PageView::destroyRendererState(shared_ptr<grfx::Renderer> renderer) {
    cerr << "XXXXX: MUST IMPLEMENT PageView destroyRendererState()" << endl;
}

void PageView::initializeRendererState(shared_ptr<grfx::Renderer> renderer) {
    cerr << "XXXXX: MUST IMPLEMENT PageView initializeRendererState()" << endl;
}

}}}  // namespace schwa::app::sketchy