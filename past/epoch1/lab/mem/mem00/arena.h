//
//  arena.h
//  schwa::mem00
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//


#ifndef __schwa__mem00__arena__
#define __schwa__mem00__arena__



// schwa::mem00 ===============================================================
namespace schwa { namespace mem00 {


template<class AllocatorT,      // allocation strategy
         class LockerT,         // thread synchronization policy
         class BoundsCheckerT,  // bounds-checking policy
         class TrackerT>        // instrumentation for memory tracking
class Arena {

    template <class MemRangeT>
    explicit Arena(const MemRangeT& range) :
        _allocator(range.minAddress(), maxAddress()) { }

    void* alloc(size_t size, size_t alignment, size_t offset = 0) {
        _locker.lock();

        const size_t originalSize = size;

        const size_t guardFront = BoundsCheckerT::size_front;
        const size_t guardBack = BoundsCheckerT::size_back;
        const size_t paddedSize = size + guardFront + guardBack;
        const size_t adjustedOffset = offset + guardFront;

        void* mem = _allocator.alloc(paddedSize, alignment, adjustedOffset);
        if (mem) {
            char* bytes = static_cast<char*>(mem);
            _boundsChecker.guard_front(mem);
            _boundsChecker.guard_back(mem + guardFront + originalSize);

            _tracker.track_alloc(mem, paddedSize, alignment)
        }


        // TODO: this is still work-in-progress.
        char* mem = static_cast<char*>(_allocator.alloc(paddedSize, ))





        _locker.unlock();
        return mem;
    }




private:
    AllocatorT     _allocator;
    LockerT        _locker;
    BoundsCheckerT _boundsChecker;
    TrackerT       _tracker;
};


}}  // schwa::mem00 ===========================================================


#endif  // #ifndef __schwa__mem00__arena__



/*
0
10

11
    void* Allocate(size_t size, size_t alignment, const SourceInfo& sourceInfo)
12
    {
13        m_threadGuard.Enter();
14        const size_t originalSize = size;
        const size_t newSize = size +
            BoundsCheckingPolicy::SIZE_FRONT +
            BoundsCheckingPolicy::SIZE_BACK;
        char* plainMemory =
            static_cast<char*>(m_allocator.Allocate(
                    newSize,
                    alignment,
                    BoundsCheckingPolicy::SIZE_FRONT));

        m_boundsChecker.GuardFront(plainMemory);
21
        m_memoryTagger.TagAllocation(plainMemory + BoundsCheckingPolicy::SIZE_FRONT, originalSize);
22
        m_boundsChecker.GuardBack(plainMemory + BoundsCheckingPolicy::SIZE_FRONT + originalSize);
23

24
        m_memoryTracker.OnAllocation(plainMemory, newSize, alignment, sourceInfo);
25

26
        m_threadGuard.Leave();
27

28
        return (plainMemory + BoundsCheckingPolicy::SIZE_FRONT);
29
    }
30

31
    void Free(void* ptr)
32
    {
33
        m_threadGuard.Enter();
34

35
        char* originalMemory = static_cast<char*>(ptr) - BoundsCheckingPolicy::SIZE_FRONT;
36
        const size_t allocationSize = m_allocator.GetAllocationSize(originalMemory);
37

38
        m_boundsChecker.CheckFront(originalMemory);
39
        m_boundsChecker.CheckBack(originalMemory + BoundsCheckingPolicy::SIZE_FRONT + allocationSize);
40

41
        m_memoryTracker.OnDeallocation(originalMemory);
42

43
        m_memoryTagger.TagDeallocation(originalMemory, allocationSize);
44

45
        m_allocator.Free(originalMemory);
46

47
        m_threadGuard.Leave();
48
    }
49

50
private:
51
    AllocationPolicy m_allocator;
52
    ThreadPolicy m_threadGuard;
53
    BoundsCheckingPolicy m_boundsChecker;
54
    MemoryTrackingPolicy m_memoryTracker;
55
    MemoryTaggingPolicy m_memoryTagger;
56
};




01
template <class AllocationPolicy,
class ThreadPolicy,
class BoundsCheckingPolicy,
class MemoryTrackingPolicy,
class MemoryTaggingPolicy>
02
class MemoryArena
*/
