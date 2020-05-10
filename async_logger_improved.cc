/*
 * Code as amended by Ovidiu Pardu <accu@ovidiuparvu.com> as part of Code
 * Critique Competition 122, which appears in ACCU's {cvu} magazine in the Mar
 * 2020 issue.  Parvu was awarded the prize for best critique by organizer Roger
 * Howarth.
 *
 */

#include "async_logger_improved.h"

#include "folly/tracing/StaticTracepoint.h"

#include <iostream>

void async_logger::print_oldest_msg() {
  uint64_t operationId = queue_.front().first;
  std::string front = queue_.front().second;
  std::cout << front << std::endl;
  queue_.pop();
  FOLLY_SDT(async_logger_improved, operation_end, operationId, front);
}

// This function runs in a dedicated thread.
void async_logger::run() {
  while (active_) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!queue_.empty()) {
      print_oldest_msg();
    }
  }
  // To ensure that all log messages are printed to the standard output before
  // the async_logger is destructed, an additional loop is added to the run()
  // method which prints all log messages remaining in queue_ after active_ is
  // set to false.
  while (!queue_.empty()) {
    print_oldest_msg();
  }
}

// The thread used to print messages to standard output should be join()'ed in
// the async_logger dtor so that it will be possible to wait for all messages to
// be printed before the async_logger object is destructed.  After removing the
// detach() statement from the async_logger() ctor, its body is empty.
async_logger::~async_logger() {
  active_ = false;
  thread_.join();
  ::std::cerr << "Destroying async_logger." << std::endl;
}

// Queue for processing on the other thread.
// http://www.cplusplus.com/reference/mutex/unique_lock/lock/
// Calling lock on a mutex object that has already been locked by other threads
// causes the current thread to block (wait) until it can own a lock to it.
void async_logger::log(const std::string &str) {
  static std::uint64_t operationIdCounter(0);

  std::unique_lock<std::mutex> lock(mutex_);
  std::uint64_t operationId = operationIdCounter++;
  FOLLY_SDT(async_logger_improved, operation_start, operationId, str);
  queue_.emplace(std::pair<std::uint64_t, std::string>(operationId, str));
}
