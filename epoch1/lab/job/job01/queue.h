#ifndef __schwa__job01__queue__
#define __schwa__job01__queue__


#include "job01/link.h"

#include <type_traits>
#include <assert.h>


// schwa::job01 ===============================================================
namespace schwa { namespace job01 {


template <class MyJobT>
class SimpleQueue : protected Linker<MyJobT> {
public:
    typedef MyJobT JobT;

    // Number of enqueued jobs.
    int count() { return _count; }

    // Add job to the queue.
    inline void add(JobT* job);

    // Pull the next job from the queue, or nullptr.
    inline JobT* next();

    // Attempt to pull a JobChain of the desired length from the queue.
    // If the queue has too few elements, return the biggest chain possible.
    inline SimpleQueue<JobT> next(int length);

protected:
    SimpleQueue() : _head(nullptr), _tail(nullptr), _count(0) { }

    SimpleQueue(JobT* head, JobT* tail, int count)
    : _head(head), _tail(tail), _count(count) { }

    SimpleQueue(SimpleQueue<JobT>&& rval)
    : _head(rval._head), _tail(rval._tail), _count(rval._count) {
        rval._head = rval._tail = nullptr;
        rval._count = 0;
    }

private:
    // Quick sanity check... ensure that _head and _tail values
    // are consistent with _count.
    void sanityCheck();

    JobT* _head;
    JobT* _tail;
    int   _count;
};


template <class OtherQueueT>
class OtherQueue : public SimpleQueue<typename OtherQueueT::JobT> {
public:

    OtherQueue() : SimpleQueue<typename OtherQueueT::JobT>() { }
//    OtherQueue() : SimpleQueue<JobT>() { }


};



// TODO: work-in-progress
template <class MyJobT>
class BossQueue : public SimpleQueue<MyJobT> {


public:
    BossQueue() : SimpleQueue<MyJobT>() { }

//    typedef MyJobT JobTT;

};



// TODO: work-in-progress
template <class QueueT>
class LocalQueue {
    // TODO: clean-up
//    static_assert(is_same<LocalQueue, QueueT::LocalQueueT>::value);
    static_assert(
        std::is_same<LocalQueue, class QueueT::LocalQueueT>::value,
        "LocalQueue class doesn't match QueueT:LocalQueueT");

public:
    LocalQueue(QueueT& master);


private:
    QueueT& _master;
};




// Inline function definitions.
#include "job01/queue_inline.h"


}}  // schwa::job01 ===========================================================


#endif  // #ifndef __schwa__job01__queue__

