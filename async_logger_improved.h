/*
 * Code as amended by Ovidiu Pardu <accu@ovidiuparvu.com> as part of Code
 * Critique Competition 122, which appears in ACCU's {cvu} magazine in the Mar
 * 2020 issue.  Parvu was awarded the prize for best critique by organizer Roger
 * Howarth.
 *
 */

#ifndef ASYNC_LOGGER_ORIG_H
#define ASYNC_LOGGER_ORIG_H

#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class async_logger {
  /* Initialize active_ true so that the run() method does not terminate before
   * the async_logger ctor runs.   Make the variable atomic in order to prevent
   * data races when queue_ and active_ are updated in unpredictable order. */
  std::atomic_bool active_{true};
  std::queue<std::pair<uint64_t, std::string>> queue_;
  // Synchronize access to the queue_ data member.
  std::mutex mutex_;

  std::thread thread_{[this]() { run(); }};
  // The log messages printing code can be extracted into a separate member
  // function.
  void print_oldest_msg();

  // Reads queue.
  void run();

public:
  // The thread used to print messages to standard output should not be
  // detach();ed in the async_logger ctor. After removing the
  // detach() statement from the async_logger() ctor, its body is empty.
  ~async_logger();
  // Writes queue.
  void log(const std::string &str);
};

#endif
