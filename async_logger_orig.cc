/* Code written by Roger Orr, rogero@howzatt.demon.co.uk and published in ACCU
 * {cvu}, Mar 2020. */

#include "async_logger_orig.h"

#include <iostream>

// This function runs in a dedicated thread.
void async_logger::run() {
  while (active_) {
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

async_logger::~async_logger() { active_ = false; }

// Queue for processing on the other thread.
void async_logger::log(const std::string &str) { queue_.emplace(str); }
