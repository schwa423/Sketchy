//
//  Worker.cpp
//  Sketchy
//
//  Created by Joshua Gargus on 6/24/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#include "Worker.h"

namespace Sketchy {
namespace Task {

    Worker::Worker(int num_threads) {
        for (int i = 0; i < num_threads; i++) {
            _threads.push_back(new std::thread(&Worker::run, this));
        }
    }


    Worker::~Worker() {
        for (auto th : _threads) { th->join(); }
    }


    void Worker::run() {
        // TODO: do it
    }


} // namespace Task {
} // namespace Sketchy {
