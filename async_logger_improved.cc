/*
 * Code as amended by Ovidiu Pardu <accu@ovidiuparvu.com> as part of Code
 * Critique Competition 122, which appears in ACCU's {cvu} magazine in the Mar
 * 2020 issue.  Parvu was awarded the prize for best critique by organizer Roger
 * Howarth.
 *
 */

#include "async_logger_improved.h"

#include "arg_classifier.h"

// Path is relative to the -I flags in the Makefile.
#include "folly/tracing/StaticTracepoint.h"
#include "template_integrate.h"

#include <climits>

#include <iostream>
#include <string>

namespace {
constexpr int32_t MAX_DIGITS = 100;
} // namespace

/* clang-format off

Following Gregg's Chapter 2.10 USDT, p. 59:
$ readelf -n async_logger_improved
Displaying notes found in: .note.gnu.build-id
  Owner                Data size        Description
  GNU                  0x00000014       NT_GNU_BUILD_ID (unique build ID bitstring)
    Build ID: 47dfaaec2098e4cfc6d31294be879ddb70e6c692
Displaying notes found in: .note.ABI-tag
  Owner                Data size        Description
  GNU                  0x00000010       NT_GNU_ABI_TAG (ABI version tag)
    OS: Linux, ABI: 3.2.0
Displaying notes found in: .note.stapsdt
  Owner                Data size        Description
  stapsdt              0x00000058       NT_STAPSDT (SystemTap probe descriptors)
    Provider: async_logger_improved
    Name: operation_end
    Location: 0x000000000001e8c9, Base: 0x0000000000000000, Semaphore: 0x0000000000000000
    Arguments: -8@-152(%rbp) -32@-64(%r14)
  stapsdt              0x00000051       NT_STAPSDT (SystemTap probe descriptors)
    Provider: async_logger_improved
    Name: operation_start
    Location: 0x000000000001f117, Base: 0x0000000000000000, Semaphore: 0x0000000000000000
    Arguments: -8@%rdx -32@(%rax)

clang-format on
*/

std::pair<bool, double> make_numeric(const std::string &arg) {
  std::pair<bool, double> retval;
  // Since 0 can legitimately be returned on both success and failure, the
  // calling program  should  set  errno  to 0 before the call, and then
  // determine if an error occurred by checking whether errno has a nonzero
  // value after the call. See  the  example on the strtol(3) manual page.
  errno = 0;
  char *endptr;
  retval.first = false;
  retval.second = strtod(arg.c_str(), &endptr);
  // if endptr is the whole string, "No digits were found."
  if ((0 != errno) || (endptr == arg.c_str())) {
    retval.second = INT_MAX;
    return retval;
  }
  retval.first = true;
  return retval;
}

void async_logger::print_oldest_msg() {
  uint64_t operationId = queue_.front().first;
  std::string front = queue_.front().second;
  std::cout << front << std::endl;
  queue_.pop();
  arg_classify::maybe_insert_folly_sdt_probe(__FILE__, "operation_end",
                                             operationId, front.c_str());
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
  FOLLY_SDT(async_logger_improved, operation_start, operationId, str.c_str());
  queue_.emplace(std::pair<std::uint64_t, std::string>(operationId, str));
}

void async_logger::log(const double val) {
  static std::uint64_t operationIdCounter(0);

  if (0.0 == val) {
    std::cerr << "Cannot integrate interval over (0,0) interval" << std::endl;
    return;
  }

  std::unique_lock<std::mutex> lock(mutex_);
  std::uint64_t operationId = operationIdCounter++;
  char input[MAX_DIGITS];
  FOLLY_SDT(async_logger_improved, operation_start, operationId,
            sprintf(input, "%e", val));

  std::vector<double> interval(NUMPTS);
  interval.push_back(0.0);
  // Integrate y=x^3 over the interval {0,1} in val steps.
  integration::CubeIt<double> generator_fn(*(interval.begin()), val / NUMPTS);
  std::generate(interval.begin(), interval.end(), generator_fn);
  double result = integration::do_integrate<double>(interval);
  char result_str[MAX_DIGITS];
  sprintf(result_str, "%e", result);
  std::string summary =
      "Interval: {0," + std::to_string(val) + "}, Result: " + result_str;

  // str is going to be the integral of y=x^3 over the interval (0.0, val).
  queue_.emplace(std::pair<std::uint64_t, std::string>(operationId, summary));
}
