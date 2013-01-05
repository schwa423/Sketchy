//
//  layer.h
//  schwa::app::sketchy
//
//  Created by Josh Gargus on 1/5/13.
//  Copyright (c) 2013 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__app__sketchy__layer__
#define __schwa__app__sketchy__layer__


// namespace schwa::app::sketchy
namespace schwa {namespace app {namespace sketchy {
    

class Layer {
    
 protected:
    std::vector<shared_ptr<Stroke>> _strokes;
    shared_ptr<StrokeShader> _shader;
    
    Eigen::Affine2f _transform;
};
    
    
}}}  // namespace schwa::app::sketchy


#endif  // #ifndef __schwa__app__sketchy__page__


