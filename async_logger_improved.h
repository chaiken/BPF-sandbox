/*
 * Code as amended by Ovidiu Pardu <accu@ovidiuparvu.com> as part of Code
 * Critique Competition 122, which appears in ACCU's {cvu} magazine in the Mar
 * 2020 issue.  Parvu was awarded the prize for best critique by organizer Roger
 * Howarth.
 *
 */

#ifndef ASYNC_LOGGER_IMPROVED_H
#define ASYNC_LOGGER_IMPROVED_H

// Path is relative to the -I flags in the Makefile.
#include "folly/tracing/StaticTracepoint.h"

#include <link.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace alogger {

constexpr uint32_t NUMPTS = 1e8;

class async_logger {
  /* Initialize active_ true so that the run() method does not terminate before
   * the async_logger ctor runs.   Make the variable atomic in order to prevent
   * data races when queue_ and active_ are updated in unpredictable order. */
  std::atomic_bool active_{true};
  std::queue<std::pair<uint64_t, std::string>> queue_;
  // Synchronize access to the queue_ data member.
  std::mutex mutex_;
  uint64_t pie_offset_ = 0u;

  std::thread thread_{[this]() { run(); }};
  // The log messages printing code can be extracted into a separate member
  // function.
  void print_oldest_msg();

  // Reads queue.
  void run();

public:
  async_logger() {
    // Copied from
    // https://github.com/facebook/folly/blob/master/folly/experimental/symbolizer/detail/Debug.h
    struct r_debug *rendezvous_structp = &_r_debug;
    if (nullptr == rendezvous_structp) {
      std::cerr << "Could not read dynamic linker information." << std::endl;
    } else {
      struct link_map *loaded_objects = rendezvous_structp->r_map;
      if (nullptr == loaded_objects) {
        std::cerr << "Dynamic linker object map unavailable" << std::endl;
      } else {
        /* Difference between the address in the ELF
          file and the addresses in memory.  */
        pie_offset_ = loaded_objects->l_addr;
        std::cerr << "PIE offset is " << std::hex << "0x"
                  << loaded_objects->l_addr << std::endl;
      }
    }
  }
  // The thread used to print messages to standard output should not be
  // detach();ed in the async_logger ctor. After removing the
  // detach() statement from the async_logger() ctor, its body is empty.
  ~async_logger();
  // Writes queue.
  void log(const std::string &str);
  void log(const double value);
};

std::pair<bool, double> make_numeric(const std::string &arg);

} // namespace alogger

#endif
