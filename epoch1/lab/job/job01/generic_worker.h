#ifndef __schwa__job01__generic_worker__
#define __schwa__job01__generic_worker__


#include <atomic>
#include <bitset>
#include <thread>
#include <vector>

#include <iostream>
using std::cerr;
using std::endl;

#include <assert.h>

// schwa::job01::impl =========================================================
namespace schwa { namespace job01 { namespace impl {


/// BaseWorker provides basic functionality that does not require knowledge of the
/// concrete queue and job-pool types (functionality which requires this knowledge
/// lives in the GenericWorker subclass).  Although there is a focus on performance,
/// virtual functions are used when performance is not critical.
class BaseWorker {
public:
    BaseWorker();
    virtual ~BaseWorker();

    /// Start the Worker, unless it has already been started.
    void start();

    /// Stop the Worker, unless it has already stopped.
    void stop();


protected:
    // Check if we have been interrupted (i.e. are any flags set?)
    bool interrupted() { return _flags.load().any(); }
    void processInterrupt();
    virtual void printOn(std::ostream& stream) const;

    // This must be implemented by GenericWorker, since it knows the actual
    // type of JobPoolT and QueueT.
    virtual void workerThreadFunc() = 0;

    enum FlagNames {
        STOP_WORKING = 0
    };

    std::mutex  _mutex;
    std::thread _thread;
    bool        _running;

    typedef std::bitset<32> Flags;
    std::atomic<Flags> _flags;

    typedef std::lock_guard<std::mutex> lock;

    friend std::ostream& operator<<(std::ostream& stream,
                                    const BaseWorker& worker);
    friend std::ostream& operator<<(std::ostream& stream,
                                    const BaseWorker::Flags& worker);
};

// For pretty printing.
std::ostream& operator<<(std::ostream& stream,
                         const BaseWorker& worker);
std::ostream& operator<<(std::ostream& stream,
                         const BaseWorker::Flags& worker);


/// Uses template specialization
template <class JobPoolT, class QueueT>
class GenericWorker : public BaseWorker {

public:
    GenericWorker(JobPoolT& pool, QueueT& queue) : _pool(pool), _queue(queue) { }
    virtual ~GenericWorker() { }

private:
    void work() {
        /*
        auto job = _queue.next();
        if (!job) {
            std::this_thread::yield();
            return;
        }

        Job


        auto job =
        */
        cerr << "working (unfinished)!  " << *this << endl;
    }

    virtual void workerThreadFunc() {
        while(true) {
            // Happily work until we're interrupted.
            while(!interrupted()) work();

            // We were just interrupted... do the necessary bookkeeping.
            processInterrupt();

            // While processing the interrupt,
            // we encountered the STOP_WORKING flag.
            if (!_running) return;
        }
    }

    typename JobPoolT::LocalPoolT  _pool;
    typename QueueT::LocalQueueT   _queue;
};


}}}  // schwa::job01::impl ====================================================


#endif  // #ifndef __schwa__job01__generic_worker__