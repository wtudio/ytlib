#!/bin/bash

# cmake
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=./build/install \
    -DYTLIB_BUILD_DOC=ON \
    -DYTLIB_BUILD_TESTS=ON \
    -DYTLIB_BUILD_BENCH_TESTS=ON \
    -DYTLIB_BUILD_WITH_BOOST=ON \
    -DYTLIB_BUILD_WITH_PROTOBUF=ON \
    -DYTLIB_BUILD_WITH_LIBUNIFEX=ON \
    -DYTLIB_BUILD_WITH_STDEXEC=ON \
    -DYTLIB_BUILD_WITH_TBB=ON \
    -DYTLIB_BUILD_CUSTOM_TESTS=OFF \
    $@

if [ $? -ne 0 ]; then
    echo "cmake failed"
    exit 1
fi

# make
cd build
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "make failed"
    exit 1
fi

# test
ctest
