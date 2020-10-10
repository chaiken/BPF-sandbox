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

# http://www.valgrind.org/docs/manual/quick-start.html#quick-start.prepare
# Compile your program with -g . . . Using -O0 is also a good idea, 
CXXFLAGS= -std=c++11 -pthread -ggdb -Wall -Wextra -g -O0 -fno-inline -fsanitize=address,undefined -I$(GTEST_HEADERS)
CXXFLAG-NOTEST= -std=c++11 -ggdb -Wall -Wextra -g -O0 -fno-inline -fsanitize=address,undefined
CXXFLAGS-NOSANITIZE= -std=c++11 -ggdb -Wall -Wextra -g -O0 -fno-inline -I$(GTEST_HEADERS)
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

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

num_digits: num_digits.cc num_digits.h
	$(CC) $(CFLAGS) $(LDFLAGS) calc_num_digits.cc -lm

libcalcfuncs: num_digits.o
	ar rvs libcalcfuncs.a num_digits.o

gcd: gcd.cc gcd_lib.cc gcd_lib.h
	$(CC) $(CFLAGS) $(LDFLAGS) gcd.cc gcd_lib.cc -o $@

gcd_lib_test: gcd_lib_test.cc gcd_lib.cc gcd_lib.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) gcd_lib_test.cc gcd_lib.cc -o $@

reverse_char_stack: reverse_char_stack.cc reverse_char_stack_lib.cc \
    reverse_char_stack_lib.h
	$(CC) $(CFLAGS) $(LDFLAGS) reverse_char_stack.cc \
           reverse_char_stack_lib.cc -o $@

reverse_char_stack_lib_test: reverse_char_stack_lib_test.cc \
    reverse_char_stack_lib.cc reverse_char_stack_lib.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) reverse_char_stack_lib_test.cc \
           reverse_char_stack_lib.cc -o $@

dyn_string_lib_test: dyn_string_lib.cc dyn_string.h dyn_string_lib_test.cc $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) dyn_string_lib.cc \
           dyn_string_lib_test.cc -o $@

dyn_string: dyn_string_lib.cc dyn_string.h
	$(CC) $(CFLAGS) $(LDFLAGS) dyn_string_lib.cc dyn_string.cc -o $@

notqsort: notqsort.cc notqsort_lib.cc notqsort_lib.h
	$(CC) $(CFLAGS) $(LDFLAGS) notqsort_lib.cc notqsort.cc -o $@

notqsort_lib_test: notqsort_lib.cc notqsort_lib_test.cc notqsort_lib.h  $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) notqsort_lib_test.cc \
           notqsort_lib.cc -o $@

dbl_vector_lib_test: dbl_vector_lib.cc dbl_vector_lib_test.cc dbl_vector.h  $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) dbl_vector_lib.cc dbl_vector_lib_test.cc -o $@

dbl_vector_lib_test2: dbl_vector_lib.cc dbl_vector_lib_test2.cc dbl_vector.h  $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) dbl_vector_lib.cc dbl_vector_lib_test2.cc -o $@

dbl_vector_time: dbl_vector_lib.cc dbl_vector_time.cc dbl_vector.h
	$(CC) $(CXXFLAGS) $(LDFLAGS) dbl_vector_lib.cc dbl_vector_time.cc -o $@

slist_main: slist_main.cc slist_lib.cc slist.h
	$(CC) $(CXXFLAGS) $(LDFLAGS) slist_main.cc slist_lib.cc -o $@

slist_lib_test: slist_lib.cc slist_lib_test.cc slist.h  $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) slist_lib.cc slist_lib_test.cc -o $@

slist_lib2_test: slist_lib2.cc slist_lib2_test.cc slist2.h  $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) slist_lib2.cc slist_lib2_test.cc -o $@

matrix_lib_test: matrix_lib.cc matrix_lib_test.cc matrix.h dbl_vector.h dbl_vector_lib.cc $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) -lm $(GTESTLIBS) matrix_lib.cc matrix_lib_test.cc dbl_vector_lib.cc -o $@

matrix_lib_test_debug: matrix_lib.cc matrix_lib_test.cc matrix.h dbl_vector.h dbl_vector_lib.cc $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) -DDEBUG $(LDFLAGS) -lm $(GTESTLIBS) matrix_lib.cc matrix_lib_test.cc dbl_vector_lib.cc -o $@

term_lib_test: term_lib.cc term_lib_test.cc term.h term_impl.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) term_lib.cc term_lib_test.cc -o $@

polynomial_lib_test: polynomial_lib.cc polynomial_lib_test.cc polynomial.h polynomial_impl.h term_lib.cc term_vector_lib.cc term.h term_impl.h term_vector.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) polynomial_lib.cc polynomial_lib_test.cc term_lib.cc term_vector_lib.cc -o $@

polynomial_lib_test_debug: polynomial_lib.cc polynomial_lib_test.cc polynomial.h polynomial_impl.h term_lib.cc term_vector_lib.cc term.h term_impl.h term_vector.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) -DDEBUG $(LDFLAGS) $(GTESTLIBS) polynomial_lib.cc polynomial_lib_test.cc term_lib.cc term_vector_lib.cc -o $@

reference_count_string_test: reference_count_string.h reference_count_string.cc reference_count_string_test.cc $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) reference_count_string.cc reference_count_string_test.cc -o $@

rational_lib_test: rational_lib.cc rational_lib_test.cc rational.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) rational_lib.cc rational_lib_test.cc -o $@

complex_lib_test: complex_lib.cc complex_lib_test.cc complex.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) complex_lib.cc complex_lib_test.cc -o $@

complex_vector_lib_test: complex_vector_lib.cc complex_vector_lib_test.cc complex_vector.h complex.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) complex_vector_lib.cc complex_vector_lib_test.cc complex_lib.cc -o $@


reference_count_string_timer: reference_count_string_timer.cc reference_count_string.cc reference_count_string.h
	$(CC) $(CXXFLAGS) $(LDFLAGS) reference_count_string_timer.cc reference_count_string.cc reference_count_string.h -o $@

# make DEBUG=DEBUG reference_count_string_timer for verbosity
reference_count_string_timer_debug: reference_count_string_timer.cc reference_count_string.cc reference_count_string.h
	$(CC) $(CXXFLAGS) $(LDFLAGS) -DDEBUG="DEBUG" reference_count_string_timer.cc reference_count_string.cc reference_count_string.h -o $@

# Does not compile.
#generic_stack_lib_test: generic_stack_lib.cc generic_stack.h generic_stack_lib_test.cc $(GTEST_HEADERS)
#      $(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) generic_stack_lib.cc generic_stack_lib_test.cc -o $@

smarter_stack_lib_test: smarter_stack_lib.cc smarter_stack_lib_test.cc smarter_stack.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) smarter_stack_lib.cc smarter_stack_lib_test.cc -o $@

smarter_queue_lib_test: smarter_queue_lib.cc smarter_queue_lib_test.cc smarter_queue.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) smarter_queue_lib.cc smarter_queue_lib_test.cc -o $@

smarter_list_lib_test: smarter_list_lib.cc smarter_list_lib_test.cc smarter_list.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) smarter_list_lib.cc smarter_list_lib_test.cc -o $@

new_clock_lib_test: new_clock_lib.cc new_clock_lib_test.cc new_clock.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) new_clock_lib.cc new_clock_lib_test.cc -o $@

templated_stack_lib_test: templated_stack_lib.cc templated_stack_lib_test.cc templated_stack.h templated_stack_impl.h complex.h complex_lib.cc $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) templated_stack_lib.cc templated_stack_lib_test.cc complex_lib.cc -o $@

const_templated_stack_lib_test: templated_stack_lib.cc const_templated_stack_lib_test.cc templated_stack.h templated_stack_impl.h complex.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) templated_stack_lib.cc const_templated_stack_lib_test.cc -DCONSTSIZE=20 -o $@

macro-vs-template: macro-vs-template.cc macro-vs-template.h complex_lib.cc complex.h
	$(CC) $(CXXFLAGS-NOTEST) $(LDFLAGS-NOTEST) macro-vs-template.cc complex_lib.cc -o $@

template_cycle_lib_test: template_cycle_lib_test.cc template_cycle.h template_cycle_impl.h polynomial_lib.cc polynomial.h polynomial_impl.h term_lib.cc term.h term_impl.h term_vector_lib.cc term_vector.h $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) template_cycle_lib_test.cc polynomial_lib.cc term_lib.cc term_vector_lib.cc -o $@

template_rotate_lib_test: template_rotate_lib_test.cc template_rotate.h template_rotate_impl.h complex.h complex_lib.cc $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) template_rotate_lib_test.cc complex_lib.cc -o $@

template_vector_lib_test: template_vector.h template_vector_impl.h template_vector_lib_test.cc complex.h complex_lib.cc polynomial.h polynomial_impl.h polynomial_lib.cc term.h term_impl.h term_vector.h term_lib.cc term_vector_lib.cc $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) template_vector_lib_test.cc complex_lib.cc polynomial_lib.cc term_lib.cc term_vector_lib.cc -o $@

template_vector_lib_test_debug: template_vector.h template_vector_impl.h template_vector_lib_test.cc complex.h complex_lib.cc $(GTEST_HEADERS)
	$(CC) $(CXXFLAGS) -DDEBUG="DEBUG" $(LDFLAGS) $(GTESTLIBS) template_vector_lib_test.cc complex_lib.cc -o $@

template_list_lib_test: template_list.h template_list_impl.h template_list_lib.cc template_list_lib_test.cc complex.h complex_lib.cc
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) complex_lib.cc template_list_lib.cc template_list_lib_test.cc -o $@

template_largest_lib_test: template_largest.h template_largest_impl.h template_largest_lib_test.cc
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) template_largest_lib_test.cc -o $@

template_integrate_lib_test: template_integrate.h template_integrate_impl.h template_integrate_lib_test.cc
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) template_integrate_lib_test.cc -o $@

reverse_list_lib_test: reverse_list.h reverse_list_impl.h reverse_list_lib_test.cc
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) reverse_list_lib_test.cc -o $@

student_inheritance_lib_test: student_inheritance.h student_inheritance_lib.cc student_inheritance_lib_test.cc
	$(CC) $(CXXFLAGS) $(LDFLAGS) $(GTESTLIBS) student_inheritance_lib.cc student_inheritance_lib_test.cc -o $@

inheritance_casting_main: student_inheritance.h student_inheritance_lib.cc inheritance_casting_main.cc
	$(CC) $(CXXFLAGS) $(LDFLAGS) student_inheritance_lib.cc inheritance_casting_main.cc -o $@
