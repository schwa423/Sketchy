//
//  view.h
//  schwa::grfx
//
//  Created by Josh Gargus on 9/15/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
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
typedef const shared_ptr<Renderer>& Renderer_ptr;


class View : public std::enable_shared_from_this<View> {
 public:
    template<class ConcreteView>
    static shared_ptr<View> New(Renderer_ptr renderer) {
        auto view = new ConcreteView();
        view->setRenderer(renderer);
        return shared_ptr<View>(view);
    }

    void setBounds(uint16_t width, uint16_t height);

    // Set/clear the renderer to be used.  This is done infrequently,
    // eg: when the OS notifies us that the app is being paused/resumed.
    // It is illegal to set a renderer if one is already set.
    void setRenderer(Renderer_ptr r);
    void clearRenderer();

    // Render the view
    virtual void render(uint64_t) = 0;

 protected:
    weak_ptr<Renderer> _renderer;
    uint16_t _width, _height;

    // Called only by setRenderer().
    virtual void destroyRendererState(Renderer_ptr r) = 0;
    virtual void initializeRendererState(Renderer_ptr r) = 0;

    // Instantiate views via New().
    View() { };
};


}}  // namespace schwa::grfx


#endif  // #ifndef __schwa__grfx__view__
