#ifndef __schwa__job__job_pool__
#define __schwa__job__job_pool__

#include "job00/job_impl.h"
#include "job00/generic_pool.h"

#include <cstddef>
#include <type_traits>

#include <assert.h>

#include <iostream>
using std::cerr;
using std::endl;

const int CACHE_LINE_SIZE = 64;


class JobPool {
public: // private:
    JobPool() { }

    // Tags to identify subpools
    enum {
      SMALL_POOL = 0, MEDIUM_POOL, LARGE_POOL, JUMBO_POOL
    } PoolTag;

    enum {
      SMALL =  CACHE_LINE_SIZE << 0,
      MEDIUM = CACHE_LINE_SIZE << 1,
      LARGE =  CACHE_LINE_SIZE << 2,
      JUMBO =  CACHE_LINE_SIZE << 3
    } PoolSize;

private:
    JobPool(const JobPool& pool);  // no copy constructor allowed

    template <size_t JOB_SIZE, uint8_t POOL_TAG>
    class SubPool : public GenericPool<JobImpl, JOB_SIZE, 1024> {
    public:
        SubPool() {
            SubPool::for_each([](JobImpl* job){
            job->pool_ = POOL_TAG;
          });
        }
    };

    typedef SubPool<SMALL,  0> SmallJobPool;
    typedef SubPool<MEDIUM, 1> MediumJobPool;
    typedef SubPool<LARGE,  2> LargeJobPool;
    typedef SubPool<JUMBO,  3> JumboJobPool;

    static JobPool global_pool_;
    SmallJobPool  small_;
    MediumJobPool medium_;
    LargeJobPool  large_;
    JumboJobPool  jumbo_;

  public:


#define JOB_SIZE (sizeof(JobDescT) + sizeof(JobImpl))
#define ENABLE_ALLOC_IF_SIZE_BETWEEN(LO, HI) typename std::enable_if< (JOB_SIZE > LO) && (JOB_SIZE <= HI) >::type* dummy2 = nullptr
#define DEFINE_ALLOC(POOL_NAME, LO, HI)  \
    template <class JobDescT>            \
    static JobImpl* Alloc(const JobDescT& dummy, ENABLE_ALLOC_IF_SIZE_BETWEEN(LO, HI)) { \
        return global_pool_.POOL_NAME##_.alloc(); \
    }

    DEFINE_ALLOC(small,  0,      SMALL);
    DEFINE_ALLOC(medium, SMALL,  MEDIUM);
    DEFINE_ALLOC(large,  MEDIUM, LARGE);
    DEFINE_ALLOC(jumbo,  LARGE,  JUMBO);

#undef JOB_SIZE
#undef ENABLE_ALLOC_IF_SIZE_BETWEEN
#undef DEFINE_ALLOC


    static void Dealloc(JobImpl* job) {
        switch (job->pool_) {
            case SMALL_POOL:
                global_pool_.small_.dealloc(job);
                break;
            case MEDIUM_POOL:
                global_pool_.medium_.dealloc(job);
                break;
            case LARGE_POOL:
                global_pool_.large_.dealloc(job);
                break;
            case JUMBO_POOL:
                global_pool_.jumbo_.dealloc(job);
                break;
            default:
                bool foundDeallocPool = false;
                assert(foundDeallocPool);
        }
    }
};


#endif  // #ifndef __schwa__job__job_pool__