#include "async_logger_orig.h"

#include "gtest/gtest.h"

#include <cstdio>

#include <fstream>
#include <iostream>

using namespace std;
// using ParamType = typename std::size_t;

// $HOME/gitsrc/googletest/googletest/docs/advanced.html
// class AsyncLoggerTest : public testing::Test, public
// testing::WithParamInterface<size_t> {
class AsyncLoggerTest : public testing::TestWithParam<string> {
public:
  AsyncLoggerTest() : oldCoutStreamBuf(cout.rdbuf()) {
    // https://stackoverflow.com/questions/4810516/c-redirecting-stdout
    strCout = new ostringstream;
    cout.rdbuf(strCout->rdbuf());
    logger = new async_logger;
    logger->log("main_thread");
    test1 = new thread([this]() { logger->log("testing thread 1"); });
  }
  ~AsyncLoggerTest() {
    // Restore output to stdout.
    cout.rdbuf(oldCoutStreamBuf);
    cout << "Output buffer is: " << endl << strCout->str() << endl;

    delete logger;
    delete test1;
    delete strCout;
  }

  ostringstream *strCout;
  async_logger *logger;
  thread *test1;

private:
  streambuf *oldCoutStreamBuf;
};

TEST_P(AsyncLoggerTest, CheckLogOutput) {
  /* Check for ctor log message. */
  ASSERT_NE(nullptr, strCout);
  EXPECT_NE(string::npos, strCout->str().find("main_thread\n"));

  /* Log the "command-line" parameters. */
  string inputstr = GetParam();
  for (size_t idx = 0; idx < inputstr.length(); ++idx) {
    logger->log("Argument " + std::to_string(idx) + " = " + inputstr.at(idx));
  }

  /* Let thread launched explicitly in ctor catch up. */
  test1->join();
  ASSERT_NE(nullptr, strCout);
  EXPECT_NE(string::npos, strCout->str().find("testing thread 1"));

  /* Check output from async_logger thread. */
  ASSERT_NE(nullptr, strCout);
  for (size_t idx = 0; idx < inputstr.length(); ++idx) {
    EXPECT_NE(string::npos,
              strCout->str().find("Argument " + std::to_string(idx) + " = " +
                                  inputstr.at(idx)));
  }
  logger->log("Main ending");
}

/* Arguments are, in order, an arbitrary choice, the test class name, and the
 * parameter values. */
INSTANTIATE_TEST_SUITE_P(AsyncLogger, AsyncLoggerTest,
                         testing::Values("", "a",
                                         "abcdefghijklmnopqrstuvwxyz"));
