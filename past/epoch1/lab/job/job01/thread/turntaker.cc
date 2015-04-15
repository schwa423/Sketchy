//
//    turntaker.cc
//    schwa::job01::thread
//
//    Copyright (c) 2013 Schwaftwarez
//    Licence: Apache v2.0
//
///////////////////////////////////////////////////////////////////////////////

#include "job01/thread/turntaker.h"

#include "job01/core/schwassert.h"

// schwa::job01::thread =======================================================
namespace schwa { namespace job01 { namespace thread {

TurnTaker::TurnTaker(Mutex& shared_mutex) : next_(nullptr),
                                            mutex_(shared_mutex),
                                            is_waiting_(false) {}

bool TurnTaker::Notify() {
  UniqueLock lock(mutex_);

  // Cannot notify turn-taker unless it is waiting.
  if (!is_waiting_)
    return false;

  is_waiting_ = false;
  condition_.notify_one();
  return true;
}

bool TurnTaker::Wait() {
  UniqueLock lock(mutex_);

  // Cannot wait if already waiting.
  if (is_waiting_)
    return false;

  FinishTurn(lock);
  return true;
}

bool TurnTaker::TakeTurn(std::function<void()> func) {
  UniqueLock lock(mutex_);

  // Cannot act if it is not our turn.
  if (is_waiting_)
    return false;

  // Do action and wait for next turn.
  func();
  FinishTurn(lock);
  return true;
}

bool TurnTaker::PassTurn() {
  UniqueLock lock(mutex_);

  // Cannot pass if it is not our turn.
  if (is_waiting_)
    return false;

  // Wait for next turn.
  FinishTurn(lock);
  return true;
}

bool TurnTaker::NotifyNextTurnTaker(UniqueLock& lock) {
  SCHWASSERT(lock.owns_lock(), "Lock must be held.");

  // Can only notify next turn-taker if it exists, and is currently waiting.
  if (!next_ || !next_->is_waiting_)
    return false;

  next_->is_waiting_ = false;
  next_->condition_.notify_one();
  return true;
}

// Notify the next turn-taker, then wait for my next turn.
void TurnTaker::FinishTurn(UniqueLock& lock) {
  if (next_)
    NotifyNextTurnTaker(lock);

  // Wait for our next turn.
  is_waiting_ = true;
  condition_.wait(lock, [&]{
    // Handle spurious wake-ups by returning false if it's not yet our turn.
    if (is_waiting_)
      return false;
    is_waiting_ = false;
    return true;
  });
}

}}}  // schwa::job01::thread ==================================================
