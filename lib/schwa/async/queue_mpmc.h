//
//  queue_mpmc.h
//  async
//
//  Created by Josh Gargus on 12/30/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//

#ifndef __schwa__async__queue_mpmc__
#define __schwa__async__queue_mpmc__


#include <atomic>


// namespace schwa::async
namespace schwa {namespace async {


#define CACHE_LINE_SIZE 64
    
struct Link {
    Link* volatile next;
};


typedef struct { char pad[CACHE_LINE_SIZE - sizeof(Link*)]; } CACHE_LINE_PAD;


// An efficient multiple-producer, multiple-consumer queue.
class QueueMPMC {
 public:
    QueueMPMC();
    
    inline void push(Link* newHead);
    inline Link* pop();
    
 private:

    std::atomic<Link*> _head;
    CACHE_LINE_PAD     _pad0;
    
    Link*              _tail;
    CACHE_LINE_PAD     _pad1;
    
    Link               _stub;
    CACHE_LINE_PAD     _pad2;
};


    
// INLINE FUNCTION DEFINITIONS ///////////////////////////////////
    
// Afterward, the old head points to the new head.
void QueueMPMC::push(Link* newHead) {
    newHead->next = nullptr;
    Link* oldHead = _head.exchange(newHead);
    // NOTE: if we block on exchange, consumers will be blocked too.
    oldHead->next = newHead;
}

    
Link* QueueMPMC::pop() {
    Link* tail = _tail;
    Link* next = tail->next;

    // If the tail is the stub, proceed past it.
    if (tail == &_stub) {
        if (!next)
            return nullptr;
        _tail = next;
        tail = next;
        next = next->next;
    }
    
    // If there is more than one
    return nullptr;
}
    
    
    
    /*
    
    mpscq_node_t* mpscq_pop(mpscq_t* self)
    {
        mpscq_node_t* tail = self->tail;
        mpscq_node_t* next = tail->next;
        if (tail == &self->stub)
        {
            if (0 == next)
                return 0;
            self->tail = next;
            tail = next;
            next = next->next;
        }
        if (next)
        {
            self->tail = next;
            return tail;
        }
        mpscq_node_t* head = self->head;
        if (tail != head)
            return 0;
        mpscq_push(self, &self->stub);
        next = tail->next;
        if (next)
        {
            self->tail = next;
            return tail;
        }
        return 0;
    } 

     */
    
    
    
    
    
    
    
    
    



/*

#include <windows.h>
#include <intrin.h>

class mpmc_queue
{
public:
    struct node_t
    {
        node_t* volatile        next_;
    };
    mpmc_queue()
    {
        head_.ptr_ = 0;
        head_.cnt_ = 0;
        tail_ = &head_.ptr_;
    }

    ~mpmc_queue()
    {
        ASSERT(head_.ptr_ == 0);
        ASSERT(tail_ == &head_.ptr_);
    }
    void enqueue(node_t* node)
    {
        ASSERT(node);
        node->next_ = 0;
        node_t** prev = (node_t**)
        _InterlockedExchange((long*)&tail_, (long)node);
        ASSERT(prev);
        // <--- the window of inconsistency is HERE (***)
        prev[0] = node;
    }

    node_t* dequeue()
    {
        unsigned retry_count = 0;
    retry:
        __try
        {
            head_t h;
            h.ptr_= head_.ptr_;
            h.cnt_ = head_.cnt_;
            for (;;)
            {
                node_t* n = h.ptr_;
                if (n == 0)
                    return 0;
                
                if (n->next_)
                {
                    head_t xchg = {n->next_, h.cnt_ + 1};
                    __int64 prev_raw =
                    _InterlockedCompareExchange64
                    (&head_.whole_, xchg.whole_, h.whole_);
                    head_t prev = *(head_t*)&prev_raw;
                    if (*(__int64*)&prev == *(__int64*)&h)
                        return n;
                    h.ptr_ = prev.ptr_;
                    h.cnt_ = prev.cnt_;
                }
                else
                {
                    node_t* t = (node_t*)tail_;
                    
                    if (n != t)
                    {
                        // spinning here may only be caused
                        // by producer preempted in (***)
                        SwitchToThread();
                        h.ptr_= head_.ptr_;
                        h.cnt_ = head_.cnt_;
                        continue;
                    }
                    
                    head_t xchg = {0, h.cnt_ + 1};
                    head_t prev;
                    prev.whole_ = _InterlockedCompareExchange64
                    (&head_.whole_, xchg.whole_, h.whole_);
                    if (prev.whole_ == h.whole_)
                    {
                        node_t* prev_tail = (node_t*)
                        _InterlockedCompareExchange
                        ((long*)&tail_, (long)&head_.ptr_, (long)n);
                        if (prev_tail == n)
                            return n;
                        
                        // spinning here may only be caused
                        // by producer preempted in (***)
                        
                        while (n->next_ == 0)
                            SwitchToThread();
                        
                        head_.ptr_ = n->next_;
                        return n;
                    }
                    
                    h.ptr_ = prev.ptr_;
                    h.cnt_ = prev.cnt_;
                }
            }
        }
        
        __except ((GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
                   && ++retry_count < 64*1024) ?
                  EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
            goto retry;
        }
    }
private:
    union head_t
    {
        struct
        {
            node_t*             ptr_;
            unsigned            cnt_;
        };
        __int64                 whole_;
    };
    head_t volatile             head_;
    char                        pad_ [64];
    node_t* volatile* volatile  tail_;
    mpmc_queue(mpmc_queue const&);
    mpmc_queue& operator = (mpmc_queue const&);
};


    
    
    
    
    
    
    
// TEST /////////////////////////////////////////////////////////////

size_t const thread_count = 8;
size_t const batch_size = 32;
size_t const iter_count = 400000;
bool volatile g_start = 0;

struct my_node : mpmc_queue::node_t
{
    int data;
    char pad [64];
};

unsigned __stdcall thread_func(void* ctx)
{
    mpmc_queue& queue = *(mpmc_queue*)ctx;
    srand((unsigned)time(0) + GetCurrentThreadId());
    size_t pause = rand() % 1000;
    my_node* node_cache [batch_size];
    for (size_t i = 0; i != batch_size; i += 1)
    {
        node_cache[i] = new my_node;
        node_cache[i]->data = i;
    }
    
    while (g_start == 0)
        SwitchToThread();

    for (size_t i = 0; i != pause; i += 1)
        _mm_pause();
    
    for (int iter = 0; iter != iter_count; ++iter)
    {
        for (size_t i = 0; i != batch_size; i += 1)
        {
            queue.enqueue(node_cache[i]);
        }
        
        for (size_t i = 0; i != batch_size; i += 1)
        {
            for (;;)
            {
                my_node* node = (my_node*)queue.dequeue();
                if (node)
                {
                    node_cache[i] = node;
                    break;
                }
                SwitchToThread();
            }
        }
    }
    return 0;
}

int main()
{
    mpmc_queue queue;
    HANDLE threads [thread_count];
    
    for (int i = 0; i != thread_count; ++i)
    {
        threads[i] = (HANDLE)_beginthreadex
        (0, 0, thread_func, &queue, 0, 0);
    }
    Sleep(1);
    
    unsigned __int64 start = __rdtsc();
    g_start = 1;

    WaitForMultipleObjects(thread_count, threads, 1, INFINITE);
    unsigned __int64 end = __rdtsc();
    unsigned __int64 time = end - start;
    std::cout << "cycles/op=" << time /
    batch_size * iter_count * 2 * thread_count)
    << std::endl;
}


    */
    
    
    
    

}}  // namespace schwa::async

    
#endif  // #ifndef __schwa__async__queue_mpmc__
