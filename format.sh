#!/bin/bash

# clang-format
find ./ytlib -regex '.*\.cpp\|.*\.hpp\|.*\.proto' -and -not -regex '.*/gen/.*\|.*\.pb\.cc\|.*\.pb\.h' | xargs clang-format -i --style=file
find ./test -regex '.*\.cpp\|.*\.hpp\|.*\.proto' -and -not -regex '.*/gen/.*\|.*\.pb\.cc\|.*\.pb\.h' | xargs clang-format -i --style=file

# cmake-format
find ./ -regex '.*\.cmake\|.*CMakeLists\.txt$' -and -not -regex '\./build/.*\|\./document/.*' | xargs cmake-format -c ./.cmake-format.py -i
