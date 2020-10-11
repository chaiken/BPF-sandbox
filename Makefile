# See ~/gitsrc/googletest/googletest/make/Makefile
# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = $(HOME)/gitsrc/googletest/googletest
# Wrong: do not include $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_HEADERS = $(GTEST_DIR)/include
GTESTLIBPATH=$(GTEST_DIR)/make
# See ~/gitsrc/googletest/googletest/README.md.
# export GTEST_DIR=/home/alison/gitsrc/googletest/
# g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
# cd make; make all
# 'make' in the README.md above doesn't create libgtest_main.a.  'make all' does.
GTESTLIBS= $(GTESTLIBPATH)/libgtest.a $(GTESTLIBPATH)/libgtest_main.a
# Where to find user code.
USER_DIR = .

USDT_DIR=$(HOME)/gitsrc/bcc
USDT_LIB_PATH=$(USDT_DIR)/build/src/cc
USDT_HEADERS=$(USDT_DIR)/tests/python/include
USDT_LIBS=$(USDT_LIB_PATH)/libbcc.a $(USDT_LIB_PATH)/libbcc_bpf.a

# http://www.valgrind.org/docs/manual/quick-start.html#quick-start.prepare
# Compile your program with -g . . . Using -O0 is also a good idea, 
# cc1plus: error: ‘-fsanitize=address’ and ‘-fsanitize=kernel-address’ are incompatible with ‘-fsanitize=thread’
CXXFLAGS= -std=c++11 -pthread -ggdb -Wall -Wextra -g -O0 -fno-inline -fsanitize=address,undefined -I$(GTEST_HEADERS)
CXXFLAG-NOTEST= -std=c++11 -ggdb -Wall -Wextra -g -O0 -fno-inline -fsanitize=address,undefined
CXXFLAGS-NOSANITIZE= -std=c++11 -ggdb -Wall -Wextra -g -O0 -fno-inline -I$(GTEST_HEADERS)
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include
USDT_FLAGS= -I$(USDT_HEADERS)

LDFLAGS= -ggdb -g -fsanitize=address -L$(GTESTLIBPATH) -lpthread
LDFLAGS-NOSANITIZE= -ggdb -g -L$(GTESTLIBPATH) -lpthread
LDFLAGS-NOTEST= -ggdb -g -fsanitize=address -lpthread
#LDFLAGS= -ggdb -g -L$(GTESTLIBPATH) -lpthread
#THREADFLAGS= -D_REENTRANT -I/usr/include/ntpl -L/usr/lib/nptl -lpthread
THREADFLAGS= -D_REENTRANT -lpthread
#https://gcc.gnu.org/ml/gcc-help/2003-08/msg00128.html
DEADCODESTRIP := -Wl,-static -fvtable-gc -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s
# gcc and clang won't automatically link .cc files against the standard library.
CC=/usr/bin/g++
#CC=/usr/bin/clang
LIBWR=-Llibwr -lwr

async_logger_orig: async_logger_orig.h async_logger_orig.cc async_enqueue_orig.cc
	$(CC) $(CXXFLAGS) $(LDFLAGS) async_logger_orig.cc async_enqueue_orig.cc -o $@

async_logger_lib_test_orig: async_logger_orig.h async_logger_orig.cc async_logger_lib_test_orig.cc
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS)  async_logger_orig.cc async_logger_lib_test_orig.cc -o $@

async_logger_improved: async_logger_improved.h async_logger_improved.cc async_enqueue_improved.cc $(USDT_HEADERS)
	$(CC) $(CXXFLAGS)  $(USDT_FLAGS) $(LDFLAGS) $(USDT_LIBS) async_logger_improved.cc async_enqueue_improved.cc -o $@

async_logger_lib_test_improved: async_logger_improved.h async_logger_improved.cc async_logger_lib_test_improved.cc
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS)  async_logger_improved.cc async_logger_lib_test_improved.cc -o $@

BINARY_LIST = async_logger_orig async_logger_lib_test_orig async_logger_improved async_logger_lib_test_improved

TESTS_LIST = async_logger_lib_test_orig async_logger_lib_test_improved

clean:
	rm -rf *.o *~ $(BINARY_LIST) *.gcda *.gcov *.gcno *.info *_output *css *html a.out

all:
	make clean
	make $(BINARY_LIST)

.SILENT: *.o

# “–coverage” is a synonym for-fprofile-arcs, -ftest-coverage(compiling) and
# -lgcov(linking).
COVERAGE_EXTRA_FLAGS = --coverage -Werror

$(TESTS_LIST): CXXFLAGS += $(COVERAGE_EXTRA_FLAGS)

# https://github.com/gcovr/gcovr/issues/314
# 'A “stamp mismatch” error is shown when the compilation time stamp *within*
# the gcno and gcda files doesn't match.'
# Therefore compilation must take place in association with the same test binary,
# not in association with another dependent test.
coverage_all:
	make clean
	@echo "CXXFLAGS is $(CXXFLAGS)"
	run_lcov_all.sh $(NO_DEPS_LIST)

TEST_EXTRA_FLAGS = -Werror -O2
TESTFLAGS = -std=c++11 -pthread -ggdb -Wall -Wextra -g $(TEST_EXTRA_FLAGS) -fno-inline -fsanitize=address,undefined -I$(GTEST_HEADERS)

# Doesn't work.
%.o: %.cc
	override CXXFLAGS = $(TESTFLAGS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $< -o $@

test_all:
	make clean
	@echo "CXXFLAGS is $(CXXFLAGS)"
	run_all_tests.sh $(TESTS_LIST)
