# See ~/gitsrc/googletest/googletest/build/Makefile
# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = $(HOME)/gitsrc/googletest
# Wrong: do not include $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_HEADERS = $(GTEST_DIR)/googletest/include
GTESTLIBPATH=$(GTEST_DIR)/build/lib
# See ~/gitsrc/googletest/googletest/README.md.
# export GTEST_DIR=/home/alison/gitsrc/googletest/
# g++ -std=c++17 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
# cd make; make all
# 'make' in the README.md above doesn't create libgtest_main.a.  'make all' does.
GTESTLIBS= $(GTESTLIBPATH)/libgtest.a $(GTESTLIBPATH)/libgtest_main.a
# Where to find user code.
USER_DIR = .

BPF_LIB_PATH=/usr/lib/x86_64-linux-gnu
BPF_HEADERS=/usr/include/bcc
BPF_LIBS=$(BPF_LIB_PATH)/libbcc.a $(BPF_LIB_PATH)/libbcc-loader-static.a $(BPF_LIB_PATH)/libbcc_bpf.a

TLD=$(HOME)/gitsrc

FOLLY_DIR=$(TLD)/folly
FOLLY_LIB_PATH=$(TLD)/fbcode-install/folly/lib
FOLLY_HEADERS=$(FOLLY_DIR)
FOLLY_LIBS=$(FOLLY_LIB_PATH)/libfolly.a $(FOLLY_LIB_PATH)/libfolly_test_util.a

GCC_DIR=$(TLD)/gcc
GCC_HEADERS=$(GCC_DIR)

CPP_SRC_DIR=$(TLD)/Cpp-Exercises
# The template_integrate code has only headers, so there's no need to create a
# static library.
INTEGRATE_HEADER=$(CPP_SRC_DIR)/template_integrate.h

# http://www.valgrind.org/docs/manual/quick-start.html#quick-start.prepare
# Compile your program with -g . . . Using -O0 is also a good idea, 
# cc1plus: error: ‘-fsanitize=address’ and ‘-fsanitize=kernel-address’ are incompatible with ‘-fsanitize=thread’
CXXFLAGS= -std=c++17 -pthread -ggdb -Wall -Wextra -g -O0 -fno-inline -fsanitize=address,undefined -I$(GTEST_HEADERS)
CXXFLAG-NOTEST= -std=c++17 -ggdb -Wall -Wextra -g -O0 -fno-inline -fsanitize=address,undefined
CXXFLAGS-NOSANITIZE= -std=c++17 -ggdb -Wall -Wextra -g -O0 -fno-inline -I$(GTEST_HEADERS)
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include
BPF_FLAGS= -I$(BPF_HEADERS)
FOLLY_FLAGS= -I$(FOLLY_HEADERS)
INTEGRATE_FLAGS= -I$(CPP_SRC_DIR)
PREPROCESSOR_FLAGS= -CC -C -E
GCC_FLAGS= -I$(GCC_HEADERS)

# DO NOT put '-lm' or '-lpthread' here.   Libraries must be listed after source
# files due to newly applied 'as-needed' flag:
#https://bbs.archlinux.org/viewtopic.php?id=156639
LDFLAGS= -ggdb -g -fsanitize=address -L$(GTESTLIBPATH) -L$(FOLLYLIBPATH)
LDFLAGS-NOSANITIZE= -ggdb -g -L$(GTESTLIBPATH)
LDFLAGS-NOTEST= -ggdb -g -fsanitize=address
#THREADFLAGS= -D_REENTRANT -I/usr/include/ntpl -L/usr/lib/nptl -lpthread
THREADFLAGS= -D_REENTRANT -lpthread
#https://gcc.gnu.org/ml/gcc-help/2003-08/msg00128.html
DEADCODESTRIP := -Wl,-static -fvtable-gc -fdata-sections -ffunction-sections -Wl,--gc-sections -Wl,-s
# gcc and clang won't automatically link .cc files against the standard library.
#CC=/usr/bin/g++
CC=/usr/bin/clang++
LIBWR=-Llibwr -lwr

async_logger_orig: async_logger_orig.h async_logger_orig.cc async_enqueue_orig.cc
	$(CC) $(CXXFLAGS) $(LDFLAGS) async_logger_orig.cc async_enqueue_orig.cc -o $@

async_logger_lib_test_orig: async_logger_orig.h async_logger_orig.cc async_logger_lib_test_orig.cc
	$(CC) $(CXXFLAGS) $(LDFLAGS) async_logger_orig.cc async_logger_lib_test_orig.cc $(GTESTLIBS) -o $@

async_logger_improved: async_logger_improved.h async_logger_improved.cc async_enqueue_improved.cc $(BPF_HEADERS) $(FOLLY_HEADERS) $(INTEGRATE_HEADERS)
	$(CC) $(CXXFLAGS) $(BPF_FLAGS) $(FOLLY_FLAGS) $(GCC_FLAGS) $(INTEGRATE_FLAGS) $(LDFLAGS) async_logger_improved.cc async_enqueue_improved.cc $(BPF_LIBS) $(FOLLY_LIBS) -o $@

async_logger_improved_preprocessor_output: async_logger_improved.h async_logger_improved.cc async_enqueue_improved.cc $(BPF_HEADERS) $(FOLLY_HEADERS) $(INTEGRATE_HEADERS)
	$(CC) $(CXXFLAGS) $(PREPROCESSOR_FLAGS) $(BPF_FLAGS) $(FOLLY_FLAGS) $(INTEGRATE_FLAGS) $(GCC_FLAGS) async_logger_improved.cc -o async_logger_improved.i

async_logger_lib_test_improved: async_logger_improved.h async_logger_improved.cc async_logger_lib_test_improved.cc arg_classifier.h $(FOLLY_HEADERS) $(INTEGRATE_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(FOLLY_FLAGS)  $(INTEGRATE_FLAGS) $(GCC_FLAGS) async_logger_improved.cc async_logger_lib_test_improved.cc $(GTESTLIBS) $(FOLLY_LIBS) -o $@

arg_classifier_lib_test: arg_classifier.h arg_classifier_lib_test.cc $(GCC_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GCC_FLAGS) arg_classifier_lib_test.cc $(GTESTLIBS) -o $@

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

# https://github.com/gcovr/gcovr/issues/314
# 'A “stamp mismatch” error is shown when the compilation time stamp *within*
# the gcno and gcda files doesn't match.'
# Therefore compilation must take place in association with the same test binary,
# not in association with another dependent test.
coverage_all:
	make clean
	@echo "CXXFLAGS is $(CXXFLAGS)"
	run_lcov_all.sh $(NO_DEPS_LIST)
