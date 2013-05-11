//
//  jobqueue.h
//  schwa::job01
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//  *** TODO ***
//      - unit test !!!
//      - use schwassert.h
//


#ifndef __schwa__job01__jobqueue__
#define __schwa__job01__jobqueue__


#include "job01/core/padding.h"
#include "job01/core/queue.h"
#include "job01/core/ring.h"
#include "job01/host/host.h"
#include "job01/impl/job_impl.h"

#include "mem00/align.h"

#include <mutex>
#include <algorithm>

// TODO: put in include file
#ifndef SCHWASSERT
#include <assert.h>    
#define SCHWASSERT(COND, MSG) assert(COND)
// Assert that the mutex is already locked by the current thread (how to implement?)
#define SCHWASSERT_LOCKED(MUTEX, MSG)
#endif


// schwa::job01 ===============================================================
namespace schwa { namespace job01 {

class JobQueue : public core::RingElement<JobQueue, 3*host::CACHE_LINE_SIZE> {


 public:
    JobQueue() { }

    // Obtain the next job from the queue.
    // If there are none available, ask our peers for some.
    impl::JobImpl* nextJob() {
        auto job = _queue.next();
        if (!job)
            requestJobsFromPeers();

        return job;
    }

    // Do any bookkeeping necessary to communicate with the queue's peers.
    void processInterrupt() {
        if (!_interrupted) return;

        // Will hold copies of requests for processing outside of the critical
        // section below.  This is necessary for avoiding deadlock, whether 
        // passing jobs to the requestor or forwarding unfulfilled requests to
        // the next queue in the ring.
        Request reqs[kMaxRequests];
        int reqCount;
 
        {
            lock lk(_mutex);
            _interrupted = false;

            // Assimilate any jobs given to us by our peers.
            _queue.add(&_received);

            // Copy the requests for processing outside the critical section.
            reqCount = _requestCount;
            for (int i = 0; i < reqCount; i++) {
                reqs[i] = _requests[i];
            }
            _requestCount = 0;

            // Check if it's now OK to request more jobs.
            if (_requestFinished) {
                _requestFinished = false;
                _requestSent = false;
            }
        }

        // Fulfill whichever requests we can, and pass the rest along.
        // First, figure out how many jobs we're willing to part with.
        int available = std::max(0, _queue.count() - kNumReservedJobs);
        // As long as jobs are available, keep fulfilling requests.
        for (int i = 0; i < reqCount; ++i) {
            available -= fulfillRequest(reqs, i, reqCount, available);
            if (!available) break;
        }
    }

 protected:
    typedef core::Queue<impl::JobImpl> JobChain;
    typedef std::lock_guard<std::mutex> lock;

    // Number of jobs to ask for if you don't have any.
    static const int kRequestBatchSize = 5;

    // Max number of job-requests that can be queued.
    // This will never be exceeded, because each queue
    // in the ring has at most one request in flight.
    static const int kMaxRequests = host::NUM_CPUS - 1;

    // Number of jobs that this queue will reserve for itself,
    // even if other queues request work to do.
    static const int kNumReservedJobs = 3;

    // Represents a request to transfer jobs to the requestor.
    // These are passed around the ring of peer-queues until either:
    // - they are completely fulfilled
    // - they reach the original requestor without being fulfilled
    struct Request {
        Request() : requestor(nullptr), desired(0) { }
        Request(JobQueue* r, int num) : requestor(r), desired(num) { }

        JobQueue* requestor;    // who is requesting the jobs?
        int desired;            // number of jobs requested
    }; 

    // Properties accessed only from our worker's thread.
    JobChain _queue;
    bool     _interrupted;
    bool     _requestSent;

    // Properties used for communication with other peers.
    CACHE_ALIGNED std::mutex _mutex;
    JobChain                 _received;
    Request                  _requests[kMaxRequests];
    int                      _requestCount;
    bool                     _requestFinished;

    // Ask our peers for something to do.
    void requestJobsFromPeers() {
        // Only one outstanding request at a time.
        if (_requestSent) return;
        _requestFinished = false;
        _requestSent = true;

        // Ask the next queue in the ring... if they don't
        // have enough jobs, they'll pass the request along.
        Request req(this, kRequestBatchSize);
        auto peer = next();
        lock lk(peer->_mutex);
        peer->receiveJobRequest(req);
    }

    // Receive a job-request originating from a peer, or our own unfulfilled
    // request when it comes full-circle.  Either way, set the interrupt flag.
    void receiveJobRequest(Request& req) {
        SCHWASSERT_LOCKED(_mutex, "Caller is responsible for locking mutex");
        SCHWASSERT(req.desired > 0, "request was already fulfilled");

        _interrupted = true;

        if (req.requestor == this) {
            // Received our own unfulfilled request.
            // We are now free to make another one.
            SCHWASSERT(_requestSent && !_requestFinished, "bad request state");
            _requestFinished = true;
        } else {
            // Enqueue request for later processing. 
            SCHWASSERT(_requestCount < kMaxRequests, "too many job requests");
            _requests[_requestCount++] = req;
        }
    }

    // Fulfill the request by passing jobs to the requestor.  If the request
    // can only be partially fulfilled (i.e. not enough jobs are available),
    // decrement the number of desired jobs in the request, and forward it to
    // the next queue in the ring.  Otherwise, pass the fulfilled request to
    // the requestor at the same time as giving it the jobs... that way it 
    // knows it can issue another request for more jobs.
    //
    // USAGE: 
    // - must be called from worker-thread which owns this queue
    // - we must not hold the lock on our mutex, since we will lock the requestor's mutex.
    int fulfillRequest(Request* reqs, int reqInd, int reqCount, int available) {
        Request& request = reqs[reqInd];

        // Try to give as many jobs as were requested.
        // Of course, there might not be enough jobs available.
        int numJobs = std::min(request.desired, available);
        request.desired -= numJobs;

        // Lock the requestor, so it's safe to give it jobs.
        JobQueue* requestor = request.requestor;
        std::unique_lock<std::mutex> requestorLock(requestor->_mutex);

        // Give the jobs to the requestor.
        JobChain jobs = _queue.next(numJobs);
        requestor->_received.add(jobs);
        requestor->_interrupted = true;

        if (request.desired == 0) {
            // The request was completely fulfilled.  Notify the
            // requestor, so that it is free to make another request.
            requestor->_requestFinished = true;
            ++reqInd;  // skip forwarding this request... see below.
        }

        if (numJobs == available && reqInd < reqCount) {
            // There are no more jobs available to fulfill requests, so forward
            // all unfulfilled requests to the next peer (including the current
            // request, if it was only partially fulfilled).
            auto peer = next();

            // It's possible that the next peer is the same as the requestor.
            // If so, avoid unlocking and relocking the same mutex.
            std::unique_lock<std::mutex> peerLock(peer->_mutex, std::defer_lock);
            if (peer != requestor) {
                requestorLock.unlock();
                peerLock.lock();
            }

            // Now the next peer is locked: forward all remaining requests.
            while (reqInd < reqCount) {
                peer->receiveJobRequest(reqs[reqInd++]);
            }
        } else {
            // There are no requests left, and/or no jobs left to fulfill them.
            // Either way, processInterrupt() will notice when we return.
        }

        return numJobs;
    }
};


class JobQueueRing : public core::Ring<JobQueue, host::NUM_CPUS> {



};


}}  // schwa::job01 ===========================================================


#endif  // #ifndef __schwa__job01__jobqueue__

