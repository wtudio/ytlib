#pragma once

#include <unifex/task.hpp>

namespace ytlib {

template <typename T>
using Task = typename unifex::task<T>;

}
