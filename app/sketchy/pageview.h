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
#include "stroke.h"
#include "strokeshader.h"
#include "mesh.h"


// namespace schwa::app::sketchy
namespace schwa {namespace app {namespace sketchy {


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

    std::vector<shared_ptr<Stroke>> _strokes;
    shared_ptr<StrokeShader> _shader;

    Eigen::Affine2f _transform;
};


}}}  // namespace schwa::app::sketchy


#endif  // #ifndef __schwa__app__sketchy__pageview__
