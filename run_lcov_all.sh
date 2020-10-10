#!/bin/bash
#
# Generate HTML-formatted gcov coverage analysis for the smarter_stack_lib_test
# program.
# Largely follows
# https://qiaomuf.wordpress.com/2011/05/26/use-gcov-and-lcov-to-know-your-test-coverage/

set -e
set -u

for FILE in $@
do
  >&2 echo "*****************************************************************************"
  >&2 echo "Generating coverage analysis for ${FILE}."
  # applications must be recompiled afresh so that timestamps *inside* gcno files match
  # https://www.gnu.org/software/automake/manual/html_node/Tricks-For-Silencing-Make.html
  make "$FILE" > /dev/null || make
  run_lcov.sh "$FILE"
  >&2 echo "*****************************************************************************"
done
