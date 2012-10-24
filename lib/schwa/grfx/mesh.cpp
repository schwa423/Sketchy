//
//  mesh.cpp
//  schwa::grfx
//
//  Created by Josh Gargus on 10/14/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "mesh.h"


// namespace schwa::grfx
namespace schwa {namespace grfx {

Mesh::Mesh(const shared_ptr<Renderer>& renderer, const shared_ptr<Format>& format)
    : Resource(renderer), _format(format), _vao(0), _vertices(0), _indices(0) {

}


void Mesh::draw() {

}


}}  // namespace schwa::grfx
