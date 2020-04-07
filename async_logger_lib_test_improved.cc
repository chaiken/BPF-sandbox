#include "async_logger_improved.h"

#include "gtest/gtest.h"

#include <condition_variable>
#include <iostream>

using namespace std;

// See $HOME/gitsrc/googletest/googletest/docs/advanced.html
// https://stackoverflow.com/questions/4810516/c-redirecting-stdout
class AsyncLoggerTest : public testing::TestWithParam<string> {
protected:
  AsyncLoggerTest()
      : logger(new async_logger), inputstr(new string),
        oldCoutStreamBuf(cout.rdbuf()), strCout(new ostringstream) {
    cout.rdbuf(strCout->rdbuf());
    logger->log("main_thread");
    test1 = new thread([this]() { logger->log("testing thread 1"); });
  }
  ~AsyncLoggerTest() {
    delete logger;

    /* Check output from async_logger thread. */
    for (size_t idx = 0; idx < inputstr->length(); ++idx) {
      EXPECT_NE(string::npos,
                strCout->str().find("Argument " + std::to_string(idx) + " = " +
                                    inputstr->at(idx)));
    }

    delete test1;
    EXPECT_NE(string::npos, strCout->str().find("testing thread 1"));

    /* Check for ctor log message. */
    EXPECT_NE(string::npos, strCout->str().find("main_thread\n"));
    EXPECT_NE(string::npos, strCout->str().find("\nMain ending\n"));

    // Restore output to stdout.
    cout.rdbuf(oldCoutStreamBuf);
    cerr << "Output buffer is: " << endl << strCout->str() << endl;
    delete strCout;
    delete inputstr;
  }

  async_logger *logger;
  thread *test1;
  string *inputstr;

private:
  streambuf *oldCoutStreamBuf;
  ostringstream *strCout;
};

TEST_P(AsyncLoggerTest, CheckLogOutput) {
  /* Log the "command-line" parameters. */
  *inputstr = GetParam();
  for (size_t idx = 0; idx < inputstr->length(); ++idx) {
    logger->log("Argument " + std::to_string(idx) + " = " + inputstr->at(idx));
  }

  test1->join();
  logger->log("\nMain ending\n");
}

/* Arguments are, in order, an arbitrary choice, the test class name, and the
 * parameter values. */
INSTANTIATE_TEST_SUITE_P(AsyncLogger, AsyncLoggerTest,
                         testing::Values("", "a",
                                         "abcdefghijklmnopqrstuvwxyz"));
