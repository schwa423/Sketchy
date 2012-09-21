//
//  pageview.h
//  schwa::app::sketchy
//
//  Created by Josh Gargus on 9/16/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__app__sketchy__pageview__
#define __schwa__app__sketchy__pageview__

#include "view.h"

// TODO: remove HACK!!
#include "Geometry.h"
#include "Shader.h"

// namespace schwa::app::sketchy
namespace schwa {namespace app {namespace sketchy {


class PageView : public grfx::View
{
 public:
    virtual void render();

 protected:
    // Called only by setRenderer().
    virtual void destroyRendererState(shared_ptr<grfx::Renderer> r);
    virtual void initializeRendererState(shared_ptr<grfx::Renderer> r);

    // TODO: remove HACK!!
    shared_ptr<Sketchy::Geometry> _geometry;
    shared_ptr<Sketchy::Shader> _shader;
};


}}}  // namespace schwa::app::sketchy

#endif  // #ifndef __schwa__app__sketchy__pageview__

