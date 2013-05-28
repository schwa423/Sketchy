//
//    typedjobx.h
//    schwa::job01::impl
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    TODO: Rename to aligned_jobx.h ? 
//    TODO: refine (right now is ooogly)
//    TODO: document
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__typed_job_x__
#define __schwa__job01__typed_job_x__


#include "job01/impl/job_impl.h"
#include "job01/impl/jobpool.h"


// schwa::job01::impl =========================================================
namespace schwa { namespace job01 { namespace impl {

// TODO: revisit public vs. private.
template <typename JobDescT>
class TypedJobX : public JobX {
 public:

    virtual ~TypedJobX() { }

    virtual void Run() {
        _description.Run();
    }

 private:
    friend class JobPool;    
    TypedJobX(JobDescT&& desc) : JobX(), _description(std::move(desc)) { }

    JobDescT _description;
};

template <typename JobDescT>
constexpr unsigned SizeCode(SCHWA_ONLY_IF_SIZE_BETWEEN(TypedJobX<JobDescT>, 0, 64)) {
    return 0;
}
template <typename JobDescT>
constexpr unsigned SizeCode(SCHWA_ONLY_IF_SIZE_BETWEEN(TypedJobX<JobDescT>, 64, 128)) {
    return 1;
}
template <typename JobDescT>
constexpr unsigned SizeCode(SCHWA_ONLY_IF_SIZE_BETWEEN(TypedJobX<JobDescT>, 128, 256)) {
    return 2;
}
template <typename JobDescT>
constexpr unsigned SizeCode(SCHWA_ONLY_IF_SIZE_BETWEEN(TypedJobX<JobDescT>, 256, 512)) {
    return 3;
}
template <typename JobDescT>
constexpr unsigned SizeCode2() {
    return SizeCode<JobDescT>();
}



}}}  // schwa::job01::impl ====================================================


#endif  // #ifndef __schwa__job01__typed_job_x__