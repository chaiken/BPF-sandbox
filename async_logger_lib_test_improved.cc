#include "async_logger_improved.h"

#include "gtest/gtest.h"

#include <sys/types.h>
#include <unistd.h>

#include <condition_variable>
#include <iostream>

using namespace std;

// See $HOME/gitsrc/googletest/googletest/docs/advanced.html
// https://stackoverflow.com/questions/4810516/c-redirecting-stdout
class AsyncLoggerCharTest : public testing::TestWithParam<string> {
protected:
  AsyncLoggerCharTest()
      : logger(new async_logger), inputstr(new string),
        oldCoutStreamBuf(cout.rdbuf()), strCout(new ostringstream) {
    cout.rdbuf(strCout->rdbuf());
    logger->log("main_thread");
    test1 = new thread([this]() { logger->log("testing thread 1"); });
  }
  ~AsyncLoggerCharTest() {
    delete logger;

    /* Check output from async_logger thread. */
    for (size_t idx = 0; idx < inputstr->length(); ++idx) {
      EXPECT_NE(string::npos, strCout->str().find("Argument " + to_string(idx) +
                                                  " = " + inputstr->at(idx)));
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

TEST_P(AsyncLoggerCharTest, CheckLogOutput) {
  /* Log the "command-line" parameters. */
  *inputstr = GetParam();
  for (size_t idx = 0; idx < inputstr->length(); ++idx) {
    logger->log("Argument " + to_string(idx) + " = " + inputstr->at(idx));
  }

  test1->join();
  logger->log("\nMain ending\n");
}

/* Arguments are, in order, an arbitrary choice, the test class name, and the
 * parameter values. */
INSTANTIATE_TEST_SUITE_P(HereAreSomeChars, AsyncLoggerCharTest,
                         testing::Values("", "a",
                                         "abcdefghijklmnopqrstuvwxyz"));

// Might there be some clever way to make a single test class templated so that
// the string and vector<double> test classes are instances of it?
class AsyncLoggerDoubleTest : public testing::TestWithParam<vector<double>> {
protected:
  AsyncLoggerDoubleTest()
      : logger(new async_logger), inputvec(new vector<double>),
        oldCoutStreamBuf(cout.rdbuf()), strCout(new ostringstream) {
    cout.rdbuf(strCout->rdbuf());
    // Print out PID for use with USDT tracing tools following
    // bcc/examples/usdt_sample/usdt_sample_app1/main.cpp.
    cerr << "pid: " << getpid() << endl;
    logger->log("main_thread");
    test1 = new thread([this]() { logger->log("testing thread 1"); });
  }
  ~AsyncLoggerDoubleTest() {
    delete logger;

    /* Check output from async_logger thread.
       Numerical integration functions have their own tests in the Cpp-Exercises
       repository at from which they originate.   See
       https://github.com/chaiken/Cpp-exercises/blob/master/template_integrate_lib_test.cc
    */
    for (size_t idx = 0; idx < inputvec->size(); ++idx) {
      EXPECT_NE(string::npos,
                strCout->str().find("Interval: {0," + to_string(inputvec->at(idx)) + "}, Result: "));
    }

    delete test1;
    EXPECT_NE(string::npos, strCout->str().find("testing thread 1"));

    /* Check for ctor log message. */
    EXPECT_NE(string::npos, strCout->str().find("\nMain ending\n"));

    // Restore output to stdout.
    cout.rdbuf(oldCoutStreamBuf);
    cerr << "Output buffer is: " << endl << strCout->str() << endl;
    delete strCout;
    delete inputvec;
  }

  async_logger *logger;
  thread *test1;
  vector<double> *inputvec;

private:
  streambuf *oldCoutStreamBuf;
  ostringstream *strCout;
};

TEST_P(AsyncLoggerDoubleTest, CheckLogOutput) {
  *inputvec = GetParam();
  for (size_t idx = 0; idx < inputvec->size(); ++idx) {
    logger->log(inputvec->at(idx));
  }
  test1->join();
  logger->log("\nMain ending\n");
}

const array<double, 4u> a({14.7, -6.11, 142.4e-3, 3.14159});
const array<double, 4u> b({123.0, 0.123, -11.236, 41.8});

INSTANTIATE_TEST_SUITE_P(HereAreSomeNumbers, AsyncLoggerDoubleTest,
                         testing::Values(vector<double>(a.begin(), a.end()),
                                         vector<double>(b.begin(), b.end())));
