# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure and build (release, all optional deps enabled)
./build.sh

# Configure and build with tests, then run them
./test.sh

# Build with specific options (minimal)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DYTLIB_BUILD_TESTS=ON
cmake --build build -j$(nproc)

# Run all tests
cd build && ctest

# Run a single test (test binaries are named <target>_test)
./build/ytlib_misc_test
./build/ytlib_container_test --gtest_filter="*SomeTest*"

# Format code
./format.sh
```

## Architecture

This is a header-only C++20 library organized into independent modules under `ytlib/`. Each module is a directory containing `.hpp` headers, an optional `*_test.cpp`, and a `CMakeLists.txt`.

**Namespace system:** All targets use a hierarchical namespace (`ytlib::misc`, `ytlib::container`, etc.) managed by `cmake/NamespaceTool.cmake`. The CMake target name uses underscores (`ytlib_misc`) while the alias uses `::`.

**Module categories:**

- Core (no external deps): `cache`, `container`, `dll_tools`, `file`, `function`, `logic`, `math`, `misc`, `string`, `thread`, `timer`
- Boost-dependent: `boost_tools_asio`, `boost_tools_fiber`, `boost_tools_util`
- Protobuf-dependent: `pb_tools`
- Execution (requires libunifex + Boost): `execution` — async primitives (schedulers, async_mutex, task, when_all)
- RPC (requires libunifex + Boost + Protobuf): `ytrpc` — protoc plugins generating RPC code for asio and unifex backends

**Dependencies** are fetched via `cmake/Get*.cmake` modules using FetchContent.

## Conventions

- C++20 standard, Google-based clang-format style with no column limit
- Headers use `#pragma once`, Doxygen `@file`/`@brief`/`@date` comments
- All code lives in the `ytlib` namespace
- Tests use Google Test; test files are named `*_test.cpp`, benchmarks `*_benchmark.cpp`
- CMakeLists.txt per module follows a template pattern (see `cmake/CMakeListsForHppLib.template`)
- Protobuf generated files (`*.pb.cc`, `*.pb.h`) are excluded from formatting
