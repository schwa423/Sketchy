//
//  queue_mpmc.cpp
//  schwa::async
//
//  Created by Josh Gargus on 12/30/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#include "queue_mpmc.h"


// namespace schwa::async
namespace schwa {namespace async {


QueueMPMC::QueueMPMC() {
    _head = &_stub;
    _tail = &_stub;
    _stub.next = nullptr;
}


}}  // namespace schwa::async