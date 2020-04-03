/*
 * Code written by Roger Orr, rogero@howzatt.demon.co.uk and published in ACCU
 * {cvu}, Mar 2020.  The unit tests sometimes pass, sometimes hit a
 * use-after-free when the test dtor deletes the queue_ object while logger
 * thread is still running, and sometimes simply fail since logger scarcely runs
 * at all.
 *
 */

#include "async_logger_orig.h"

#include <iostream>

// This function runs in a dedicated thread.
void async_logger::run() {
  while (active_) {
    // Will trigger a user-after-free if the async_logger object is destroyed
    // before the thread running this function finishes its work.
    if (!queue_.empty()) {
      std::cout << queue_.front() << std::endl;
      queue_.pop();
    }
  }
}

async_logger::async_logger() {
  active_ = true;
  thread_.detach();
}

async_logger::~async_logger() {
  active_ = false;
  ::std::cout << "Destroying async_logger." << std::endl;
}

// Queue for processing on the other thread.
void async_logger::log(const std::string &str) { queue_.emplace(str); }
