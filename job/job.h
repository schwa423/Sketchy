//
//  job.h
//  schwa::job
//
//  Created by Josh Gargus on 11/23/12.
//  Copyright (c) 2012 Schwaftwarez. All rights reserved.
//
//

#ifndef __schwa__job__job__
#define __schwa__job__job__


#include "free_list.h"

#include <memory>


// namespace schwa::job
namespace schwa {namespace job {


class Job : private mem::FreeList::Link {

};


class JobRunner {
 public:
    JobRunner(int maxJobs) {
        size_t sz = maxJobs * sizeof(Job) + sizeof(mem::FreeList);
        char* mem = static_cast<char*>(operator new(sz));
        char* jobMem = mem + sizeof(mem::FreeList);
        _freeList.reset(new (mem) mem::FreeList(jobMem, maxJobs, sizeof(Job)));
    }

 protected:
    std::unique_ptr<mem::FreeList> _freeList;
};


}}  // namespace schwa::job


#endif  // #ifndef __schwa__job__job__
