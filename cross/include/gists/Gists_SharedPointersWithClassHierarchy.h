//
//  Gists_SharedPointersWithClassHierarchy.h
//  Sketchy
//
//  Created by Josh Gargus on 8/3/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//

#ifndef Sketchy_Gists_SharedPointersWithClassHierarchy_h
#define Sketchy_Gists_SharedPointersWithClassHierarchy_h

#include <vector>
#include <memory>
#include <iostream>
using std::cerr;
using std::endl;

namespace Sketchy {
namespace Gists {

class SharedPointersWithClassHierarchy {
 public:

    // The whole point!
    static void Gist();

    class Spectacle;
    
    class Observer {
     public:
        
        typedef std::shared_ptr<Observer> ptr;
        typedef std::weak_ptr<Observer> weak_ptr;
        
        virtual void fooHappened(const Spectacle& spectacle) = 0;
        virtual void barHappened(const Spectacle& spectacle) = 0;
    };
    
    class Spectacle {
     public:

        // NOTE: arg must be const-ref, otherwise passing a Dude::ptr wouldn't work...
        //       an Observer::ptr would be created from the Dude::ptr at the call-site,
        //       and it would be an rvalue, and therefore can't be bound to a non-const
        //       lvalue.  See:
        //       http://www.gamedev.net/topic/538371-casting-boostshared_ptrderived-to-boostshared_ptrbase/
        void addObserver(const Observer::ptr& observer) {
            _observers.push_back(observer);
        }
        
        void doFoo() {
            notifyObservers([=] (Observer::ptr& observer) { observer->fooHappened(*this); });
        }
        
        void doBar() {
            notifyObservers([=] (Observer::ptr& observer) { observer->barHappened(*this); });
        }
        
        
     protected:
        void notifyObservers(std::function<void (Observer::ptr&)> closure) {
            for (auto o : _observers) {
                Observer::ptr ptr = o.lock();
                if (ptr.get()) closure(ptr);
            }
        }

        std::vector<Observer::weak_ptr> _observers;
    };
    
    class Dude : public Observer {
     public:        
        typedef std::shared_ptr<Dude> ptr;

        explicit Dude(const char* nm) : name(nm) { };
        const char* name;
        
        
        virtual void fooHappened(const Spectacle& spectacle) {
            cerr << name << " observed that foo happened" << endl;
        }
        
        virtual void barHappened(const Spectacle& spectacle) {
            cerr << name << " observed that bar happened" << endl;
        }
        
    };

};  // class SharedPointersWithClassHierarchy
        
    
void SharedPointersWithClassHierarchy::Gist() {
    Spectacle spectacle;
    
    auto Bill = std::make_shared<Dude>("Bill");
    auto Jim = std::make_shared<Dude>("Jim");
    
    Observer::ptr dudePtr = Jim;
    
    spectacle.addObserver(Bill);
    spectacle.addObserver(dudePtr);

    spectacle.doFoo();
    spectacle.doBar();
    spectacle.doFoo();
    spectacle.doFoo();
}
    

}  // namespace Gists
}  // namespace Sketchy

#endif  // Sketchy_Gists_SharedPointersWithClassHierarchy_h
