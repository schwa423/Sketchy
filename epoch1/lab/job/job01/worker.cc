//
//    worker.cc
//    schwa::job01
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////

#include "job01/worker.h"
#include "job01/core/stack.h"

#include <iostream>

typedef std::lock_guard<std::mutex> lock_guard;

// schwa::job01 ===============================================================
namespace schwa { namespace job01 {

Worker::Worker() : _queue(nullptr), _interrupted(false), _running(false) {}

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
                ProcessJobs();
                break;
            case kStopped:
                return;
        }
    }
}

void Worker::ProcessJobs() {
    std::cerr << "Worker::ProcessJobs() not yet implemented" << std::endl;
}

// TODO: work-in-progress, probably broken
void Worker::Loop() {
    while (true) {
        core::Stack<JobRequest> requests;

        if (_interrupted) {
            lock_guard lock(_mutex);
            _interrupted = false;

// TODO: implement
//            requests.add(_queue->HandleInterrupt());

            // Receive
// TODO: implement
//            ReceiveMessages();
            if (!_running)
                return;
        }
        auto job = _queue->NextJob();
// TODO: implement
//        ProcessMessages();
        if (job) {
            job->Run();
        } else {
            std::this_thread::yield();
        }
    }
}


}}  // schwa::job01 ===========================================================
