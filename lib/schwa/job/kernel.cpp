//
//  kernel.cpp
//  Sketchy
//
//  Created by Josh Gargus on 12/16/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#include "kernel.h"


// namespace schwa::job
namespace schwa {namespace job {


// No-op kernel... Jobs are initialized with this by default.
void EmptyKernel(JobData& data, Spawn& parent) { }


}}  // namespace schwa::job