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
                processJobs();
                break;
            case kStopped:
                return;
        }
    }
}

// TODO: work-in-progress, probably broken
void Worker::Loop() {
    while (true) {
        base::Stack<JobRequest> requests;

        if (_interrupted) {
            lock_guard lock(_mutex);
            _interrupted = false;

            requests.Add(_queue->HandleInterrupt());

            // Receive
            ReceiveMessages();
            if (!_running)
                return;
        }
        auto job = _queue->NextJob();
        ProcessMessages();
        if (job) {
            job->Run();
        } else {
            std::this_thread::yield();
        }
    }
}

void Worker::ReceiveMessages() {

}

}}  // schwa::job01 ===========================================================
