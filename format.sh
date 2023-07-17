#!/bin/bash

# clang-format
find ./ytlib -regex '.*\.cpp\|.*\.hpp\|.*\.proto' -and -not -regex '.*\.pb\.cc\|.*\.pb\.h' | xargs clang-format -i --style=file
find ./test -regex '.*\.cpp\|.*\.hpp\|.*\.proto' -and -not -regex '.*\.pb\.cc\|.*\.pb\.h' | xargs clang-format -i --style=file

# cmake-format
find ./ -regex '.*\.cmake\|.*CMakeLists\.txt$' -and -not -regex '\./build/.*\|\./document/.*' | xargs cmake-format -c ./.cmake-format.py -i

# autopep8, apt install python3-autopep8
find ./ -regex '.*\.py' -and -not -regex '\./build/.*\|\./document/.*' | xargs autopep8 -i --global-config ./.pycodestyle
