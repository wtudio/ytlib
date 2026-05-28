#!/bin/bash

# cmake
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=./build/install \
    -DYTLIB_BUILD_TESTS=ON \
    -DYTLIB_BUILD_BENCH_TESTS=OFF \
    -DYTLIB_BUILD_WITH_BOOST=OFF \
    -DYTLIB_BUILD_WITH_PROTOBUF=OFF \
    -DYTLIB_BUILD_WITH_LIBUNIFEX=OFF \
    -DYTLIB_BUILD_WITH_STDEXEC=OFF \
    -DYTLIB_BUILD_WITH_TBB=OFF \
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
