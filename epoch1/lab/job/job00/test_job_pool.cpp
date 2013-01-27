#include "job00/job_pool.h"
#include "job00/job_impl_create.h"

#include <assert.h>


class JobDescTester {
public:
    explicit JobDescTester() : run_count_(nullptr), destroy_count_(nullptr) { }

    JobDescTester(int& run_count, int& destroy_count) :
      run_count_(&run_count), destroy_count_(&destroy_count) { }

    JobDescTester(JobDescTester&& moved) :
      run_count_(moved.run_count_), destroy_count_(moved.destroy_count_) {
      // Trash old object.
      moved.run_count_ = moved.destroy_count_ = nullptr;
    }

    ~JobDescTester() {
      if (destroy_count_)
        ++*destroy_count_;
    }

    void operator()(){ ++*run_count_; }

private:
    int* run_count_;
    int* destroy_count_;
};

class MediumJobTester : public JobDescTester {
public:
    MediumJobTester() : JobDescTester() { }
    char padding[64];
};

class LargeJobTester : public JobDescTester {
public:
    LargeJobTester() : JobDescTester() { }
    char padding[128];
};

class JumboJobTester : public JobDescTester {
public:
    JumboJobTester() : JobDescTester() { }
    char padding[256];
};


int main(void) {

  int desc1_runs = 0;
  int desc1_destroys = 0;

  JobImpl* job = JobImpl::Create(JobDescTester(desc1_runs, desc1_destroys));

  job->run();
  assert(job->isAllocated());
  assert(desc1_runs == 1);
  assert(desc1_destroys == 0);

  job->run();
  assert(job->isAllocated());
  assert(desc1_runs == 2);
  assert(desc1_destroys == 0);

  job->destroy();
  assert(!job->isAllocated());
  assert(desc1_runs == 2);
  assert(desc1_destroys == 1);

  // Test allocation of jobs of different sizes.
  JobImpl* medium[1024];
  JobImpl* large[1024];
  JobImpl* jumbo[1024];
  for (int i=0; i < 1023; i++) {
    medium[i] = JobImpl::Create(MediumJobTester());
    large[i] = JobImpl::Create(LargeJobTester());
    jumbo[i] = JobImpl::Create(JumboJobTester());

    assert(medium[i] != nullptr);
    assert(large[i]  != nullptr);
    assert(jumbo[i]  != nullptr);
  }
  medium[1023] = JobImpl::Create(MediumJobTester());
  large[1023] = JobImpl::Create(LargeJobTester());
  jumbo[1023] = JobImpl::Create(JumboJobTester());

  // All sub-pools should be empty now.
  assert(!JobImpl::Create(MediumJobTester()));
  assert(!JobImpl::Create(LargeJobTester()));
  assert(!JobImpl::Create(JumboJobTester()));

  // Put back all medium jobs.  Still should not be able to allocate
  // large and jumbo jobs.
  for (int i=0; i < 1024; i++) {
    medium[i]->destroy();
  }
  assert(!JobImpl::Create(LargeJobTester()));
  assert(!JobImpl::Create(JumboJobTester()));

  // Put back the rest of the jobs.
  for (int i=0; i < 1024; i++) {
    large[i]->destroy();
    jumbo[i]->destroy();
  }

  // Allocate max number of jobs, test we can't allocate more,
  // and finally release them all.
  for (int i=0; i < 1024; i++) {
    medium[i] = JobImpl::Create(MediumJobTester());
    large[i] = JobImpl::Create(LargeJobTester());
    jumbo[i] = JobImpl::Create(JumboJobTester());

    assert(medium[i] != nullptr);
    assert(large[i]  != nullptr);
    assert(jumbo[i]  != nullptr);
  }
  assert(!JobImpl::Create(MediumJobTester()));
  assert(!JobImpl::Create(LargeJobTester()));
  assert(!JobImpl::Create(JumboJobTester()));
  for (int i=0; i < 1024; i++) {
    medium[i]->destroy();
    large[i]->destroy();
    jumbo[i]->destroy();
  }

  cerr
    << "sizeof(JobImpl): " << sizeof(JobImpl) << endl
    << "sizeof(JobDescTester): " << sizeof(JobDescTester) << endl;

  return 0;
}
