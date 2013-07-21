//
//    turntaker.h
//    schwa::job01::thread
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
//    TurnTaker is a class that is useful for orchestrating sequences of events
//    between multiple threads.  Each thread is given a TurnTaker.
//
//    See test_turntaker.cc for example usage, and some nuanced discussion.
//
//    TODO: move into a utils or testing package?
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __schwa__job01__thread__turntaker__
#define __schwa__job01__thread__turntaker__

#include <functional>

#include "job01/thread/synchronization.h"

// schwa::job01::thread =======================================================
namespace schwa { namespace job01 { namespace thread {

class TurnTaker {
 public:
  // Initialize TurnTaker with a mutex that is shared with other TurnTakers;
  // it is used to coordinate who is allowed to take a turn.
  TurnTaker(Mutex& shared_mutex);

  // Notify TurnTaker that it is now her turn.
  // If TurnTaker is not waiting, return false without notifying.
  // Note: locks the shared mutex.
  bool Notify();

  // Blocking wait for TurnTaker's next turn.
  // If TurnTaker is already waiting, return false without waiting.
  // Note: locks the shared mutex.
  bool Wait();

  // Do the specified action, then wait for the next turn.
  // If TurnTaker is already waiting, return false without performing action.
  // Note: locks the shared mutex.
  bool TakeTurn(std::function<void()> func);

  // Do nothing this turn; instead wait for the next turn.
  // If TurnTaker is already waiting, return false without notifying anyone.
  // Note: locks the shared mutex.
  bool PassTurn();

  // Set the turn-taker whose turn will be next, after mine.
  void SetNextTurnTaker(TurnTaker* next) { next_ = next; }

 protected:
  // Notify the next turn-taker that it's now her turn.
  // Return false if there is no next turn-taker,
  // or if that turn-taker is not currently waiting for their turn.
  // TODO: document difference between FinishTurn() and NotifyNextTurnTaker().
  bool NotifyNextTurnTaker(UniqueLock& lock);

  // Notify the next turn-taker, then wait for my next turn.
  // TODO: document difference between FinishTurn() and NotifyNextTurnTaker().
  void FinishTurn(UniqueLock& lock);

  // Subclasses must be able to access the mutex in order to lock it.
  Mutex& mutex_;

 private:
  TurnTaker* next_;
  Condition  condition_;
  bool       is_waiting_;
};

}}}  // schwa::job01::thread ==================================================

#endif  // __schwa__job01__thread__turntaker__
