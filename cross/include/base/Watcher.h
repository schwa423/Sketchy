//
//  Watcher.h
//  Sketchy
//
//  Created by Joshua Gargus on 7/22/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//

#ifndef Sketchy_Watcher_h
#define Sketchy_Watcher_h

#include <set>

namespace Sketchy {
namespace Task {

template<class Owner, class Watched, class WatcherType>
class Watcher : public WatcherType {
 public:
    typedef std::shared_ptr<Watched> WatchedPtr;

    explicit Watcher(Owner& owner) : _owner(&owner) { }

    void watch(const WatchedPtr& watchee) {
        _watched.insert(watchee);
        watchee->addWatcher(this);
    }

    void ignore(const WatchedPtr& watchee) {
        int count = _watched.erase(watchee);
        if (count) watchee->removeWatcher(this);
    }

    void stopWatching() {
        for (auto watchee: _watched) { watchee->removeWatcher(this); }
        _watched.clear();
    }

    int size() { return _watched.size(); }
    bool empty() { return _watched.empty(); }

 protected:
    Owner* _owner;
    std::set<WatchedPtr> _watched;
};

} // namespace Task {
} // namespace Sketchy {

#endif // #ifndef Sketchy_Watcher_h
