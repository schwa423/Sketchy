//
//  presenter.h
//  schwa::grfx
//
//  Created by Josh Gargus on 9/30/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__grfx__presenter__
#define __schwa__grfx__presenter__

#include <cstdint>
#include <memory>
using std::shared_ptr;

// namespace schwa::grfx
namespace schwa {namespace grfx {


class View;
class Framebuffer;
class Renderer;


// TODO: class description
class Presenter {
 public:
    Presenter(const shared_ptr<Renderer>& r);
    virtual ~Presenter() {}

    // Set the view that will be renderered.
    void setView(const shared_ptr<View>& view);

    // Render the specified view into the specified framebuffer.
    void render(uint64_t time);

 protected:
    // Subclasses may override how the rendered view is presented.
    // By default, simply tell the framebuffers
    // For example, the view might be rendered into a renderbuffer
    // which must be presented via the native windowing system,
    // or it might be rendered into a texture that will be applied
    // to other geometry (perhaps after computing mipmaps, or
    // applying a blur or other effect).
    virtual void present() = 0;

    // Used to update view bounds when either the view or the framebuffer changes.
    void setBounds(uint16_t width, uint16_t height);

    shared_ptr<View> _view;
    shared_ptr<Framebuffer> _framebuffer;
    shared_ptr<Renderer> _renderer;
    uint16_t _width, _height;
};


}}  // namespace schwa::grfx


#endif  // __schwa__grfx__presenter__
