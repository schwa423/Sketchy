//
//    test_blocks.cc
//    schwa::job01::mem::test
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Exercise raw block-allocation using a simplified job-queue as an example.
//
///////////////////////////////////////////////////////////////////////////////


#include "job01/core/link.h"
#include "job01/core/queue.h"
#include "job01/mem/align.h"
#include "job01/mem/blocks.h"
#include "job01/dbg/dbg.h"
using namespace schwa::job01;
using namespace schwa::job01::mem;


#include <iostream>
#include <vector>
using std::cerr;
using std::endl;


// Forward declaration of TestJob, so we can define the reference-type.
class TestJob;
typedef BlockRef<TestJob> TestJobRef;


// Job definition.
class__cache_align  TestJob : public core::Link<TestJob, TestJobRef> {
 public:        
 	TestJob() { }

 	uint8_t  foo;
    uint16_t bar;
    uint32_t baz;
};


// BlockArray containing jobs.
const int kJobArraySize = 1024;
typedef BlockArray<TestJob, kJobArraySize> TestJobArray;


class TestJobQueue : public core::Queue<TestJob> {
 public:
 	// Allocate a new TestJobArray, and add the jobs to the queue.
 	void allocateJobs() {
 		// Allocate a new TestJobArray, and remember that we allocated it.
 		TestJobArray& jobs = *(Create<TestJobArray>());
 		_arrays.push_back(jobs.id);

 		// Add the jobs.
 		for (int i = 0; i < kJobArraySize; i++) {
 			jobs[i]->baz = i;
 			add(TestJobRef(jobs[i]));
 		}
 	}

 private:
 	std::vector<impl::BlockArrayRef> _arrays;
};


// Friend of BlockArray etc., to facilitate testing.
namespace schwa { namespace job01 { namespace mem {
class BlockTests {
public:

	static TestJob* GetBlockFromArray(TestJobArray& array, int index) {
		return array._blocks + index;
	}

};
}}}


// Test that we can obtain TestJobRefs to the array's contents, and that
// out-of-bounds accesses do not cause problems.
void testBlockAccess() {
	TestJobArray& jobs1 = *(Create<TestJobArray>());
 	TestJobArray& jobs2 = *(Create<TestJobArray>());

	for (int i = 0; i < kJobArraySize; ++i) {
	 	// Test that BlockRef computes the correct pointers.
	 	assert(jobs1[i] != nullptr);
	    assert(jobs2[i] != nullptr);
	 	assert(static_cast<TestJob*>(jobs1[i]) != nullptr);
	 	assert(static_cast<TestJob*>(jobs2[i]) != nullptr);
	 	assert(static_cast<TestJob*>(jobs1[i]) == BlockTests::GetBlockFromArray(jobs1, i));
	 	assert(static_cast<TestJob*>(jobs2[i]) == BlockTests::GetBlockFromArray(jobs2, i));
		assert(static_cast<TestJob*>(jobs1[i]) != static_cast<TestJob*>(jobs2[i]));
	}

	// Test out-of-bounds accesses.
	assert(static_cast<TestJob*>(jobs1[-1]) == nullptr);
	assert(static_cast<TestJob*>(jobs2[-1]) == nullptr);
	assert(static_cast<TestJob*>(jobs1[kJobArraySize]) == nullptr);
	assert(static_cast<TestJob*>(jobs2[kJobArraySize]) == nullptr);
}


// Show that jobs allocated from an array can be enqueued and dequeued.
void testSimpleJobQueue() {
	TestJobQueue q1, q2;

	assert(q1.count() == 0);
	assert(q1.count() == 0);

	q1.allocateJobs();

	TestJobRef prev;
	for (int i = 0; i < 512; ++i) {
		TestJobRef job = q1.next();

		// Since the job was removed from the queue, it should be unlinked.
		assert(job->nextLink() == nullptr);

		// TestJobs were added in order.
		assert(i == job->baz);

		// Add the job to the other queue.
		q2.add(job);

		// Now that the job was added to the queue, 
		// the previous job should be linked to it.
		if (i > 0) {
			assert(prev->nextLink() == job);
		}
		// However, the most-recently-added job isn't linked to anything.
		assert(job->nextLink() == nullptr);
		prev = job;
	}

	// Both queues should have 512 jobs now.
	assert(q1.count() == 512);
	assert(q2.count() == 512);

	// Allocate some more jobs and check again.
	q1.allocateJobs();
	q1.allocateJobs();
	assert(q1.count() == 512 + 2 * kJobArraySize);
	assert(q2.count() == 512);
	q2.allocateJobs();
	q2.allocateJobs();
	assert(q1.count() == 512 + 2 * kJobArraySize);
	assert(q2.count() == 512 + 2 * kJobArraySize);	
}


void testPerformance(TestJobRef* refs, TestJob** ptrs) {
	// Obtain a bunch of job-refs so that we can see
	// how fast they can be dereferenced.
	for(int i = 0; i < 100; ++i) {
		TestJobArray& jobs = *(Create<TestJobArray>());
		for(int j = 0; j < 1000; ++j) {
			refs[i * 1000 + j] = jobs[j];
		}
	}

	{	// Print out how long it takes to obtain a gazillion raw pointers
		// from their corresponding block-refs.
		dbg::ScopeTimer t([](time::msecs elapsed) {
			cerr << "finished 100 million real iterations in " 
			     << elapsed << endl;
		});

		for (int i = 0; i < 1000; ++i) {
			for (int j = 0; j < 100000; ++j) {
				ptrs[j] = static_cast<TestJob*>(refs[j]);
			}
		}
	}

	{   // Print out how long it takes to obtain a gazillion raw pointers
		// from their corresponding block-refs.
		dbg::ScopeTimer t([](time::msecs elapsed) {
			cerr << "finished 100 million bogus iterations in " 
			     << elapsed << endl;
		});

		for (int i = 0; i < 1000; ++i) {
			for (int j = 0; j < 100000; ++j) {
				ptrs[j] = ptrs[100000-j];
			}
		}
	}
}


int main() {
	testBlockAccess();
	testSimpleJobQueue();

    if (false) {
    	// If we don't pass in these arrays, the optimized build will
    	// eliminate the benchmark loop as dead code (at least on XCode).
    	TestJobRef refs[100000];
    	TestJob*   ptrs[100000];

    	// TODO: incorporate this into a perf-test framework.
        testPerformance(refs, ptrs);
    }

    cerr << "job01/mem/test_blocks...  PASSED!" << endl << endl;
}

