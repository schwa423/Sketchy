#ifndef _QI_BASE_Runner_h_
#define _QI_BASE_Runner_h_

#include "qi/qi.h"

namespace qi {

class IRunner {
 public:
  // Schedule |job| to run asynchronously.  Return false the job could not be
  // scheduled (i.e. because Shutdown() was already called.
  virtual bool Run(std::function<void()> job) = 0;

  // Disallow scheduling of additional jobs (after this is called, Run() must
  // return false).  Block waiting for all currently-scheduled jobs to complete.
  virtual void Shutdown() = 0;

  virtual ~IRunner() {}
};

}  // namespace qi

#endif  // _QI_BASE_Runner_h_
