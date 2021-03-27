/* Code written by Roger Orr, rogero@howzatt.demon.co.uk and published in ACCU
 * {cvu}, Mar 2020.
 *
 * $ ./async_logger_orig a b c d e
 * main_thread
 * testing thread 1
 * Argument 0 = ./async_logger_orig
 * Argument 1 = a
 * Argument 2 = b
 * Argument 3 = c
 *
 * $  ./async_logger_orig a bu c dae asfua a g aud fg main_thread
 * testing thread 1
 * Argument 0 = ./async_logger_orig
 * Argument 1 = a
 * Argument 2 = bu
 * Argument 3 = c
 * Argument 4 = dae
 * Argument 5 = asfua
 * Argument 6 = a
 * Argument 7 = g
 * Argument 8 = aud
 * ==598470==ERROR: AddressSanitizer: attempting double-free on 0x603000001990
 in thread T1: #0 0x7f14c74d9f97 in operator delete(void*)
 (/usr/lib/x86_64-linux-gnu/libasan.so.5+0x109f97)
    *
 */

#include "async_logger_improved.h"

using namespace alogger;

int main(int argc, char **argv) {
  async_logger logger;
  logger.log("main_thread\n");
  std::thread test1([&logger]() { logger.log("testing thread 1"); });
  // Skip over the application's name.
  for (int idx = 1; idx < argc; ++idx) {
    std::string not_a_number =
        "Argument " + std::to_string(idx) + " = " + argv[idx];
    std::pair<bool, double> conversion = make_numeric(argv[idx]);
    if (conversion.first) {
      logger.log(conversion.second);
    } else {
      logger.log(not_a_number);
    }
  }
  logger.log("\nMain ending\n");
  test1.join();
}
