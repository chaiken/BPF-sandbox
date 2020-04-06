#include "async_logger_improved.h"

#include "gtest/gtest.h"

#include <condition_variable>
#include <iostream>

using namespace std;

// See $HOME/gitsrc/googletest/googletest/docs/advanced.html
class AsyncLoggerTest : public testing::TestWithParam<string> {
protected:
  AsyncLoggerTest() : oldCoutStreamBuf(cout.rdbuf()) {
    // https://stackoverflow.com/questions/4810516/c-redirecting-stdout
    strCout = new ostringstream;
    cout.rdbuf(strCout->rdbuf());
    logger = new async_logger;
    logger->log("main_thread");
    test1 = new thread([this]() { logger->log("testing thread 1"); });
  }
  ~AsyncLoggerTest() {
    /* Adding a new lock for synchronization seems rather contrary to the spirit
     * of unit tests, but I don't see another way to force the logger thread to
     * terminate before the test exits. Trying to join() the logger thread from
     * the unit test triggers an exception as indicated by joinable() being
     * false. */
    std::unique_lock<std::mutex> lck(test_mutex);
    ready = true;

    // Restore output to stdout.
    cout.rdbuf(oldCoutStreamBuf);
    cout << "Output buffer is: " << endl << strCout->str() << endl;

    delete logger;
    cout << "Notifying test TearDown()." << endl;
    cv.notify_all();
    delete test1;
    delete strCout;
  }

  /* Putting the actual tests here in TearDown() means that the logger thread is
   * reliably joined before they run, presumably when the test exits.  Otherwise
   * the test failed about 1/200 times.  I also tried creating a test class
   * derived from async_logger whose destructor didn't call join() on the logger
   * thread, but that did not work since the base class destructor always runs
   * before that of the derived class. */
  void TearDown(const string &inputstr) {
    // Wait for test class destructor to destroy the async_logger object.
    std::unique_lock<std::mutex> lck(test_mutex);
    while (!ready) {
      cv.wait(lck);
    }
    /* Check output from async_logger thread. */
    for (size_t idx = 0; idx < inputstr.length(); ++idx) {
      EXPECT_NE(string::npos,
                strCout->str().find("Argument " + std::to_string(idx) + " = " +
                                    inputstr.at(idx)));
    }
    EXPECT_NE(string::npos, strCout->str().find("testing thread 1"));
    /* Check for ctor log message. */
    EXPECT_NE(string::npos, strCout->str().find("main_thread\n"));
    EXPECT_NE(string::npos, strCout->str().find("\nMain ending\n"));
  }

  async_logger *logger;
  thread *test1;

private:
  streambuf *oldCoutStreamBuf;
  ostringstream *strCout;
  // Based on
  // http://www.cplusplus.com/reference/condition_variable/condition_variable/
  std::mutex test_mutex;
  std::condition_variable cv;
  bool ready = false;
};

TEST_P(AsyncLoggerTest, CheckLogOutput) {
  /* Log the "command-line" parameters. */
  string inputstr = GetParam();
  for (size_t idx = 0; idx < inputstr.length(); ++idx) {
    logger->log("Argument " + std::to_string(idx) + " = " + inputstr.at(idx));
  }

  test1->join();
  logger->log("\nMain ending\n");
}

/* Arguments are, in order, an arbitrary choice, the test class name, and the
 * parameter values. */
INSTANTIATE_TEST_SUITE_P(AsyncLogger, AsyncLoggerTest,
                         testing::Values("", "a",
                                         "abcdefghijklmnopqrstuvwxyz"));
