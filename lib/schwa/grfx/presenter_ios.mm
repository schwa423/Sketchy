//
//  presenter_ios.mm
//  schwa::grfx
//
//  Created by Josh Gargus on 10/3/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "presenter_ios.h"
#include "renderbuffer_ios.h"

#include <iostream>
using std::cerr;
using std::endl;


// namespace schwa::grfx
namespace schwa {namespace grfx {


LayerPresenter_iOS::LayerPresenter_iOS(const shared_ptr<Renderer>& renderer,
                                       EAGLContext* context)
    : Presenter(renderer), _context(context) {

}


void LayerPresenter_iOS::setFramebuffer(const shared_ptr<Framebuffer>& framebuffer) {
    Presenter::setFramebuffer(framebuffer);

    auto color = _framebuffer->colorAttachment();
    _renderbuffer = std::dynamic_pointer_cast<Renderbuffer_iOS>(color);

    if (!_renderbuffer)
        cerr << "LayerPresenter_iOS cannot extract layer-renderbuffer" << endl;
}


void LayerPresenter_iOS::present() {
    if (!_renderbuffer) {
        cerr << "WARNING!!  no renderbuffer, so cannot present layer!!!!!!!" << endl;
        return;
    }

    _renderbuffer->bind();
    CHECK_GL("failed to bind renderbuffer for present()");

    if (YES != [_context presentRenderbuffer:GL_RENDERBUFFER])
        cerr << "ERROR!!  failed to present renderbuffeer" << endl;
}


}}  // namespace schwa::grfx
