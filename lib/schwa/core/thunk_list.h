//
//  thunk_list.h
//  schwa::core
//
//  Created by Josh Gargus on 9/10/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//


#ifndef __schwa__core__thunk_list__
#define __schwa__core__thunk_list__

#include <functional>
#include <vector>

namespace schwa {
namespace core {

typedef std::function<void ()> Thunk;

class ThunkList {
    typedef std::lock_guard<std::mutex> lock_guard;

 public:
    // Add a thunk to be executed later.
    void add(Thunk t) {
        lock_guard lock(_mutex);
        _thunks.push_back(t);
    }

    // Remove and execute all previously-added thunks.
    void drain() {
        std::vector<Thunk> thunks;
        {
            // Release lock as quickly as possible...
            // new thunks can be added while drained thunks are still executing.
            lock_guard lock(_mutex);
            std::swap(thunks, _thunks);
        }
        for (auto th : thunks) th();
    }

 private:
    std::mutex _mutex;
    std::vector<Thunk> _thunks;
};


// TODO: move to separate file
// TODO: better name?
// Defer execution of thunk until current scope is exited.
class ThunkAfterScope {
 public:
    ThunkAfterScope(Thunk thunk) : _thunk(thunk) { }
    ~ThunkAfterScope() { _thunk(); }

 protected:
    Thunk _thunk;
};


}  // namespace core
}  // namespace schwa



#endif  // #ifndef __schwa__core__thunk_list__

