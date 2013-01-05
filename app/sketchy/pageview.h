//
//  pageview.h
//  schwa::app::sketchy
//
//  Created by Josh Gargus on 9/16/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__app__sketchy__pageview__
#define __schwa__app__sketchy__pageview__

#include "view.h"
#include "page.h"


#include "stroke.h"
#include "strokeshader.h"
#include "mesh.h"



// namespace schwa::app::sketchy
namespace schwa {namespace app {namespace sketchy {

    
class Page;
    

// Base-class for views that are inherently 2d.  They:
// - have some notion of orientation (which way is up)
// - can pass 2d UI events to their model.
// TODO: promote to grfx once it works well.
class View2d : public grfx::View {

};


class PageView : public View2d
{
 public:
    PageView();

    virtual void render(uint64_t time);

 protected:
    // Called only by setRenderer().
    virtual void destroyRendererState(grfx::Renderer_ptr r);
    virtual void initializeRendererState(grfx::Renderer_ptr r);
    // TODO: we're shadowing the _renderer variable in grfx::View.
    shared_ptr<grfx::Renderer> _renderer;
    
    shared_ptr<Page> _page;

    Eigen::Affine2f _transform;
};


}}}  // namespace schwa::app::sketchy


#endif  // #ifndef __schwa__app__sketchy__pageview__
