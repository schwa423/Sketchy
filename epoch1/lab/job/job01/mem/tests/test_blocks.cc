//
// test_blocks.cc
// schwa::job01::mem::test
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//


#include "job01/core/link.h"
#include "job01/core/queue.h"
#include "job01/mem/blocks.h"

using namespace schwa::job01;
using namespace schwa::job01::mem;


#include <iostream>
#include <vector>
using std::cerr;
using std::endl;


class Job;


// Users can wrap BlockRef for convenience.
class JobRef {
 public:
 	JobRef() { }
 	explicit JobRef(const BlockRef& ref) : _ref(ref) { }

 	JobRef& operator=(const BlockRef& ref) {
 		_ref = ref;
 	}

 	// Can't define until after Job is declared.
 	Job* GetJob() const;

 	// Make it handy to access the job.
 	Job* operator->() {
 		return GetJob();
 	}

 	// Comparison with null.
    bool operator== (nullptr_t p) const {
        return _ref == nullptr;
    }
    bool operator!= (nullptr_t p) const {
        return _ref != nullptr;
    }

    // Comparison with other refs.
    bool operator== (const JobRef& ref) const {
        return _ref == ref._ref;
    }  
    bool operator!= (const JobRef& ref) const {
        return _ref != ref._ref;
    }  

 private:
 	BlockRef _ref;
};



class alignas(64) Job : public core::Link<Job, JobRef>,
                        public Block {
 public:                  	
    int index;
};


// Now that Job has been declared, we can define this function.
inline Job* JobRef::GetJob() const {
	return static_cast<Job*>(_ref.GetBlock());
}


const int kJobArraySize = 1024;
class JobArray : public TypedBlockArray<Job, kJobArraySize> {
	// So that the protected constructor can be called.
	// TODO: this is ugly... how can we get around this?
	friend class BlockArrayManager;

 public:
 	Job* GetJob(int index) {
 		return &_blocks[index];
 	}

 protected:
 	JobArray(const BlockArrayRef& id) : TypedBlockArray(id) { }
};


class JobQueue : public core::Queue<Job> {
 public:
 	// Allocate a new JobArray, and add the jobs to the queue.
 	void allocateJobs() {
 		// Allocate a new JobArray, and remember that we allocated it.
 		JobArray& jobs = *(BlockArray::create<JobArray>());
 		_arrays.push_back(jobs.id);

 		// Add the jobs.
 		for (int i = 0; i < kJobArraySize; i++) {
 			jobs.GetJob(i)->index = i;
 			add(JobRef(jobs[i]));
 		}
 	}

 private:
 	std::vector<BlockArrayRef> _arrays;
};



void testBlockAccess() {
	JobArray& jobs1 = *(BlockArray::create<JobArray>());
 	JobArray& jobs2 = *(BlockArray::create<JobArray>());

	for (int i = 0; i < kJobArraySize; ++i) {
	 	// Test that BlockRef computes the correct pointers.
		assert(jobs1[i].GetBlock() != nullptr);
		assert(jobs2[i].GetBlock() != nullptr);		
		assert(jobs1[i].GetBlock() == jobs1.GetJob(i));
		assert(jobs2[i].GetBlock() == jobs2.GetJob(i));		
		assert(jobs1[i].GetBlock() != jobs2[i].GetBlock());
	}

	// Test out-of-bounds accesses.
	assert(jobs1[-1].GetBlock() == nullptr);
	assert(jobs2[-1].GetBlock() == nullptr);
	assert(jobs1[kJobArraySize].GetBlock() == nullptr);
	assert(jobs2[kJobArraySize].GetBlock() == nullptr);
}


void testSimpleJobQueue() {
	JobQueue q1, q2;

	assert(q1.count() == 0);
	assert(q1.count() == 0);

	q1.allocateJobs();

	JobRef prev;
	for (int i = 0; i < 512; ++i) {
		JobRef job = q1.next();

		// Since the job was removed from the queue, it should be unlinked.
		assert(job->nextLink() == nullptr);

		// Jobs were added in order.
		assert(i == job->index);

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


int main() {

	testBlockAccess();
	testSimpleJobQueue();

    cerr << "job01/mem/test_blocks...  PASSED!" << endl << endl;
}

