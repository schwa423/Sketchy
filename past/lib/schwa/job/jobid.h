//
//  jobid.h
//  Sketchy
//
//  Created by Josh Gargus on 12/17/12.
//  Copyright (c) 2012 Schwaftwarez.
//  Licensed under the Apache License, Version 2.0.
//
//
//  App-devs never touch a Job directly, only indirectly via a JobId.
//  This indirection provides the option of eg: relocating jobs in a
//  JobPool in order to combat memory fragmentation (we might allocate
//  it from an arena that only contains relocatable objects).
//
//

#ifndef __schwa__job__jobid__
#define __schwa__job__jobid__


#include <stdint.h>
#include "schwassert.h"


// namespace schwa::job
namespace schwa {namespace job {


typedef int32_t JobOffset;
class JobPool;

class JobId {
public:
    // Default constructor creats an ID which refers to no job.
    inline JobId();

    // Move and copy constructors, for constructing IDs from existing IDs.
    inline JobId(JobId&& id);
    inline JobId(const JobId& id);

    // Test whether two IDs are equivalent.
    inline bool operator==(const JobId& id) const;

    // Assignment opeartor.
    inline void operator=(const JobId& id);

    // Answer true unless JobOffset is BAD_OFFSET.
    bool isValid() const { return _offset != BAD_OFFSET; }

private:
    // Only JobPool can create new IDs.
    inline JobId(JobOffset offset, uint32_t generation);

    // These two fields uniquely identify a Job within a JobPool.
    JobOffset _offset;
    uint32_t  _generation;

    // Used for asserting that JobId is only used to refer to
    // jobs in the pool that it was created by.
    SCHWASSERT_CODE(JobPool* _pool);

    // Used to designate a JobId that does not refer to a job.
    static const JobOffset BAD_OFFSET = -13;

    friend class JobPool;
};


// Inline method definitions /////////////////////////////////////////////////////

// Default constructor
JobId::JobId() : _offset(BAD_OFFSET), _generation(0) {
    SCHWASSERT_CODE( _pool = nullptr; )
}


// Move constructor
JobId::JobId(JobId&& id) : _offset(id._offset), _generation(id._generation) {
    id._offset = BAD_OFFSET;
    id._generation = 0;
    SCHWASSERT_CODE(
        _pool = id._pool;
        id._pool = nullptr;
    );
}


// Copy constructor
JobId::JobId(const JobId& id) : _offset(id._offset), _generation(id._generation) {
    SCHWASSERT_CODE( _pool = id._pool; )
}


// Equality test.
bool JobId::operator==(const JobId& id) const {

#if defined(GTEST)
    // Disable this assertion in unit-tests, since we want to test that
    // our move-constructor works properly.
    // TODO: is there a less hacky way to do this?
    SCHWASSERT( _pool == id._pool, "JobId pools don't match");
    SCHWASSERT(_offset != BAD_OFFSET && id._offset != BAD_OFFSET,
               "JobId with bad offset");
#endif

    return _offset == id._offset
        && _generation == id._generation;
}


// Assignment operator
void JobId::operator=(const JobId& id) {
    _offset = id._offset;
    _generation = id._generation;
    SCHWASSERT_CODE( _pool = id._pool; )
}


// Private constructor.
JobId::JobId(JobOffset o, uint32_t g) : _offset(o), _generation(g) {
    SCHWASSERT_CODE( _pool = nullptr; )
}


}}  // namespace schwa::job


#endif  // #ifndef __schwa__job__jobid__