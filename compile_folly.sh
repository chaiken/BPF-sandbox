#!/bin/bash
set -e
set -u

readonly TLD="$HOME"/gitsrc
readonly FOLLY_DIR="$TLD"/folly
readonly OLDWD="$PWD"

echo "Cloning folly . . ."
cd "$TLD"
git clone https://github.com/facebook/folly.git
cd folly

echo "Installing dependencies . . ."
sudo apt-get install libboost-dev libboost-all-dev
sudo apt-get install libdouble-conversion-dev libdouble-conversion3 liblz4-dev libbz2-dev
sudo apt-get install libgoogle-glog-dev libfmt-dev pandoc
sudo apt-get install libunwind-dev libsodium-dev liburing-dev libaio-dev libiberty-dev libdwarf-dev libsnappy-dev libzstd-dev

echo "Generating Makefile"
cd "$FOLLY_DIR"/build
cmake ..

echo "Building folly"
cd "$FOLLY_DIR"/build/folly
make
echo "Produced ${FOLLY_DIR}/build/libfolly.a: "
echo "$(ls -l ${FOLLY_DIR}/build/libfolly.a)"
echo "Add ${FOLLY_DIR}/build/libfolly.a to Makefile."
echo "Headers are in ${FOLLY_DIR}/folly/; add '-I${FOLLY_DIR} to Makefile."

echo "Building documentation . . ."
cd "$FOLLY_DIR"/folly/docs
make 
echo "Produced file ${FOLLY_DIR}/folly/docs/index.html: "
echo "$(ls -l ${FOLLY_DIR}/folly/docs/index.html)"

cd "$OLDPWD"

