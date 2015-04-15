//
//    jobdescription.h
//    schwa::job01
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    Placeholder.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef __schwa__job01__jobdescription__
#define __schwa__job01__jobdescription__


// schwa::job01 ===============================================================
namespace schwa { namespace job01 {


class JobDescription {
 public:
  JobDescription(JobDescription&& desc) { }


  // TODO: this should take a Job& argument eventually.
  virtual void Run() { }

 protected:
  explicit JobDescription() {

  }
};


}}  // schwa::job01 ===========================================================

#endif  // #ifndef __schwa__job01__jobdescription__