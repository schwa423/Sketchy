//
//  view.h
//  schwa::grfx
//
//  Created by Josh Gargus on 9/15/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__grfx__view__
#define __schwa__grfx__view__

#include <cstdint>
#include <memory>
using std::shared_ptr;
using std::weak_ptr;

// namespace schwa::grfx
namespace schwa {namespace grfx {

class Renderer;
class Framebuffer;

class View : public std::enable_shared_from_this<View> {
 public:
    template<class ConcreteView>
    static shared_ptr<View> New(shared_ptr<Renderer> renderer) {
        auto view = new ConcreteView();
        view->setRenderer(renderer);
        return shared_ptr<View>(view);
    }

    // Set/clear the renderer to be used.  This is done infrequently,
    // eg: when the OS notifies us that the app is being paused/resumed.
    // It is illegal to set a renderer if one is already set.
    void setRenderer(const shared_ptr<Renderer>& r);
    void clearRenderer();

    // Render the view
    virtual void render(uint64_t) = 0;

 protected:
    weak_ptr<Renderer> _renderer;

    // Called only by setRenderer().
    virtual void destroyRendererState(shared_ptr<Renderer> r) = 0;
    virtual void initializeRendererState(shared_ptr<Renderer> r) = 0;

    // Instantiate views via New().
    View() { };
};

}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__view__
