//
//  page.h
//  schwa::app::sketchy
//
//  Created by Josh Gargus on 1/5/13.
//  Copyright (c) 2013 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__app__sketchy__page__
#define __schwa__app__sketchy__page__


// namespace schwa::app::sketchy
namespace schwa {namespace app {namespace sketchy {


class Page {
    Page();
    
    void render(uint64_t time, const Renderer_ptr& renderer, const Eigen::Affine2f& transform);
    
    
    
 protected:
    


    

    std::vector<shared_ptr<Layer>> _layers;
    
        Eigen::Affine2f _transform;
    
    std::vector<shared_ptr<Stroke>> _strokes;
    shared_ptr<StrokeShader> _shader;
    
    
    // TODO: de-hack!
    shared_ptr<Layer> _drawLayer;
    shared_ptr<Layer> _spiralLayer;
};

    
}}}  // namespace schwa::app::sketchy


#endif  // #ifndef __schwa__app__sketchy__page__


