//
// test_pool.cpp
// schwa::job01::core::test
//
//  Copyright (c) 2013 Schwaftwarez
//  Licence: Apache v2.0
//
//


#include "job01/core/link.h"
#include "job01/core/pool.h"

//using namespace schwa;
//using namespace schwa;
using namespace schwa::job01::core;

#include <iostream>


struct alignas(64) TestJob : public Link<TestJob, PoolObjRef>, public PoolObj {
	int foo;
	int bar;
};


class TestBoss : public Pools {
 public:
	static TestJob* GetJob(const PoolObjRef& ref) {
		return static_cast<TestJob*>(Pools::GetObject(ref));
	}
};


class TestPool : public Pool {
 public:
 	static TestPool* create() { return Pool::create<TestPool>(); }

 	TestPool(PoolID id) : Pool(id, jobs, alignof(TestJob)) { }

 	TestJob jobs[1024];
};


int main() {
	TestPool* pool = TestPool::create();
	PoolID pool_ID = pool->id();

	PoolObjRef null_ref;
	SCHWASSERT(nullptr == TestBoss::GetJob(null_ref), 
		       "null_ref should return a null job-ptr");


    std::cerr << "job01/core/test_pool...  PASSED!" << std::endl << std::endl;
}

