#!/bin/bash

# cmake
cmake -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=./build/install \
    -DYTLIB_BUILD_TESTS=OFF \
    -DYTLIB_BUILD_BENCH_TESTS=OFF \
    -DYTLIB_BUILD_WITH_BOOST=OFF \
    -DYTLIB_BUILD_WITH_PROTOBUF=OFF \
    -DYTLIB_BUILD_WITH_LIBUNIFEX=OFF \
    -DYTLIB_BUILD_WITH_STDEXEC=OFF \
    -DYTLIB_BUILD_WITH_TBB=OFF \
    -DYTLIB_BUILD_CUSTOM_TESTS=ON \
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

# install
if [ -d install ]; then
    rm -rf install
fi

make install
