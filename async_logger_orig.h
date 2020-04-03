/* Code written by Roger Orr, rogero@howzatt.demon.co.uk and published in ACCU
 * {cvu}, Mar 2020. */

#ifndef ASYNC_LOGGER_ORIG_H
#define ASYNC_LOGGER_ORIG_H

#include <queue>
#include <string>
#include <thread>

class async_logger {
  bool active_;
  std::queue<std::string> queue_;
  std::thread thread_{[this]() { run(); }};

  // Reads queue.
  void run();

public:
  async_logger();
  ~async_logger();
  // Writes queue.
  void log(const std::string &str);
};

#endif
