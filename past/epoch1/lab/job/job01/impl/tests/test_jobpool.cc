//
//    test_jobpool.cc
//    schwa::job01::impl
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    TODO: much more comprehensive testing!  Need a test plan...
//          what are the likely breakage points?  Test those.
//
//    TODO: compile breaks if you comment out SizeCode2() below!!!!!
//          Has something to do with not forcing instantiation of
//          a template specialization... need to fully understand
//          this issue, because it will crop up elsewhere!
//
//////////////////////////////////////////////////////////////////////////////

#include "job01/impl/jobpool.h"
#include "job01/impl/typedjobx.h"
#include "job01/jobdescription.h" 

using namespace schwa::job01;
using namespace schwa::job01::impl;

#include <iostream>
using std::cerr;
using std::endl;


class JobStats {
 public:
    JobStats() : allocs(0), deallocs(0), runs(0) { }
    int allocs;
    int deallocs;
    int runs;
};


class JobStatsTracker : public schwa::job01::JobDescription {
 public:
    JobStatsTracker(JobStatsTracker&& other) 
    : JobDescription(std::move(other)), _stats(other._stats) {
        other._stats = nullptr;
    }

    JobStatsTracker(JobStats* stats) {
        _stats = stats;
        _stats->allocs++;
    }

    ~JobStatsTracker() {
        if (_stats) _stats->deallocs++;
    }

    void Run() {
        // TODO: verify that we user can't cause Run()
        //       to occur without a valid stats object.
        SCHWASSERT(_stats != nullptr, 
                   "Run() shouldn't be without a stats object");
        _stats->runs++;
    }

 private:
    JobStats* _stats;
};


int main() {
    JobPool pool;
    JobStats stats;

    assert(0 == stats.allocs);
    assert(0 == stats.runs);
    assert(0 == stats.deallocs);

    JobRef ref = pool.Alloc(JobStatsTracker(&stats));
    JobX* job = static_cast<JobX*>(ref);
    assert(1 == stats.allocs);
    assert(0 == stats.runs);
    assert(0 == stats.deallocs);

    job->Run();
    assert(1 == stats.allocs);
    assert(1 == stats.runs);
    assert(0 == stats.deallocs);

    job->Run();
    assert(1 == stats.allocs);
    assert(2 == stats.runs);
    assert(0 == stats.deallocs);

    pool.Free(ref, job);
    assert(1 == stats.allocs);
    assert(2 == stats.runs);
    assert(1 == stats.deallocs);

    cerr << "job01/impl/test_jobpool...  PASSED!" << endl << endl;
}
