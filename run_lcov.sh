#!/bin/bash
#
# Generate HTML-formatted gcov coverage analysis for the smarter_stack_lib_test
# program.
# Largely follows
# https://qiaomuf.wordpress.com/2011/05/26/use-gcov-and-lcov-to-know-your-test-coverage/

set -e
set -u
set -o pipefail

if (( $# != 1)); then
  echo "Usage: run_lcov.sh <test name>, where 'test name' is that of the test"
  echo "binary, not that the the source file."
  exit 1
fi

# Note: .gcno files are created by compilation with --coverage switch.
#       .gcda files are created by executing the binary directly or with
# 	 "lcov -c -o foo.info".

readonly test_name="$1"
echo "test_name is ${test_name}"
readonly target="$(grep "${test_name}:" Makefile)"
if [[ -z "$target" ]]; then
  echo "No compilation target matching ${test_name}".
  exit 2
fi
echo "Creating coverage data for ${test_name}."

# Actually run the test.  Not actually necessary, but test failure will as a
# side benefit terminate the script.
./"$test_name" > /tmp/"$test_name".out

# "man lcov" Capture coverage data; creates info file
# Use the --directory option to capture counts for a user  space  program.
# Running the binary directly creates .gcda files, but only running this command
# creates .info files.  Running the command with existing .gcda files does not
# work.
lcov --base-directory . --directory . --capture -o "$test_name".info

# Applications which depend on libraries that have their own tests:
#
# Depend on complex_lib.cc: complex_vector_lib_test, templated_stack_lib_test,
# template_rotate_lib_test, template_vector_lib_test, template_list_lib_test
#
# Depend on polynomial_lib.cc: template_cycle_lib_test, template_vector_lib_test

# The blog posting above mentioned the same tracefile as input and output,
# which did not work for me.
# There does not appear to be any way to remove the test binary itself
# from the coverage analysis, which makes the resulting coverage percentages
# too high.
lcov --remove "$test_name".info \
     "/usr/*" \
     "/home/alison/gitsrc/googletest/*" \
     -o "$test_name"_processed.info

genhtml -o . -t "${test_name} coverage" \
    --num-spaces 2 "$test_name"_processed.info

# Removes all gcda files -- not what is needed.
# lcov --base-directory . --directory . --zerocounters
#
# Remove existing .gcda files that may interfere with creation of new ones
# when multiple tests include the same library.  If bar_lib depends on foo_lib
# therefore, run coverage analysis on bar_lib_test first, remove foo_lib.gcda,
# and then run foo_lib_test coverage analysis.
remove_conflicting_artifacts() {
  if (( 1 != $# )); then
    >&2 echo "remove_conflicting_arguments: Provide the name of a single test"
    >&2 echo "as an argument."
    exit 5
  fi
  local tname="$1"
  # Note tname and _test below, not "tname" or "$tname".
  local libname="${tname%%_test}"
  if [[ -z "$libname" ]]; then
    >&2 echo "Erroneous test name ${tname}?"
    exit 6
  fi
  if [[ -f "$libname".gcno ]]; then
    >&2 echo "*** Removing ${libname}.gcno ***"
    /bin/rm "$libname".gcno
  fi
  if [[ -f "$libname".gcda ]]; then
    >&2 echo "*** Removing ${libname}.gcda ***"
    /bin/rm "$libname".gcda
  fi
}
