//
//    worker.cc
//    schwa::job01
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////


#include "job01/worker.h"



typedef std::lock_guard<std::mutex> lock_guard;


// schwa::job01 ===============================================================
namespace schwa { namespace job01 {



Worker::Worker() : _queue(nullptr), _interrupted(false), _is_stopped(true) {



}




void Worker::start() {
    lock_guard lock(_mutex);
    if (_state != kStopped) {
        // Already started.
        return;
    }

    _thread = std::thread(&Worker::run, this);
}


void Worker::stop() {
    lock_guard lock(_mutex);
    if (_state == kStopped) {
        // Already stopped.
        return;
    }

    _thread.join();
}



void Worker::run() {
    while (true) {
        switch (_state) {
            case kProcessing:
                processJobs();
                break;
            case kStopped:
                return;
        }
    }
}


void Worker::processJobs() {
    while(!_interrupted) {
        auto job = _queue->NextJob();
        if (job) {
            job->Run();
        } else {
            std::this_thread::yield();
        }
    }
}


}}  // schwa::job01 ===========================================================
