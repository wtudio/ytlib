#include <iostream>
#include <string>
#include <type_traits>

#include "ytlib/dll_tools/dynamic_lib.hpp"
#include "ytlib/misc/misc_macro.h"

/**
 * @brief thirdparty
 *
 */
namespace thirdparty {

struct complicated_structure {
  friend void do_something(complicated_structure &t) noexcept {
    std::cout << "customized do something" << std::endl;
  }
};

struct simple_structure {
};

}  // namespace thirdparty

/**
 * @brief standard
 *
 */
namespace standard {

namespace detail {

template <typename T>
void do_something(T &t) noexcept {
  std::cout << "standard do something" << std::endl;
}

struct do_something_t {
  template <typename T>
  void operator()(T &t) noexcept {
    do_something(t);
  }
};

}  // namespace detail

inline detail::do_something_t do_something{};
}  // namespace standard

/**
 * @brief TestCPO1
 *
 */
void TestCPO1() {
  thirdparty::simple_structure s;
  standard::do_something(s);

  thirdparty::complicated_structure c;
  standard::do_something(c);

  using namespace standard;
  do_something(s);
  do_something(c);
}

// --------------------------------------------------

namespace standard2 {

// -------------------定义tag_invoke-----------
namespace detail {

void tag_invoke();

struct tag_invoke_t {
  template <typename Tag, typename... Args>
  constexpr auto operator()(Tag tag, Args &&...args) const
      noexcept(noexcept(tag_invoke(static_cast<Tag &&>(tag), static_cast<Args &&>(args)...)))
          -> decltype(tag_invoke(static_cast<Tag &&>(tag), static_cast<Args &&>(args)...)) {
    return tag_invoke(static_cast<Tag &&>(tag), static_cast<Args &&>(args)...);
  }
};
}  // namespace detail

inline constexpr detail::tag_invoke_t tag_invoke{};

template <auto &Tag>
using tag_t = std::decay_t<decltype(Tag)>;

// -----------------声明do_something方法可以自定义，同时定义默认do_something方法-----------
namespace detail {
struct do_something_t {
  template <typename T>
  std::string operator()(T &t, int n) noexcept {
    return tag_invoke(do_something_t{}, t, n);
  }
};

// 注意函数定义不再是do_something，而是tag_invoke，tag就是 detail::do_something_t
template <typename T>
std::string tag_invoke(do_something_t, T &t, int n) noexcept {
  std::cout << "standard do something " << n << std::endl;
  return "standard do something";
}

}  // namespace detail

inline detail::do_something_t do_something{};

}  // namespace standard2

// -----------------自定义do_something方法-----------
namespace thirdparty2 {

struct complicated_structure {
  // tag_t<do_something>就是standard::detail::do_something_t
  friend std::string tag_invoke(standard2::tag_t<standard2::do_something>, complicated_structure &t, int n) noexcept {
    std::cout << "customized do something " << n << std::endl;
    return "customized do something";
  }
};

struct simple_structure {
};

}  // namespace thirdparty2

void TestCPO2() {
  thirdparty2::simple_structure s;
  std::cout << standard2::do_something(s, 1) << std::endl;

  thirdparty2::complicated_structure c;
  std::cout << standard2::do_something(c, 1) << std::endl;

  using namespace standard2;
  std::cout << do_something(s, 2) << std::endl;
  std::cout << do_something(c, 2) << std::endl;
}

std::tuple<int, std::string> foo() {
  int n = 1;
  std::string s = "aaa";

  return std::make_tuple(n, std::move(s));
}

int32_t main(int32_t argc, char **argv) {
  DBG_PRINT("hello world");

  // int n = 0;
  // std::string s;
  // std::tie(n, s) = foo();
  auto [n, s] = foo();
  DBG_PRINT("n: %d, s: %s", n, s.c_str());

  // TestCPO1();

  // TestCPO2();

  // using AddFun = int (*)(int, int);

  // auto &dll_container = ytlib::DynamicLibContainer::Ins();
  // auto testlib = dll_container.LoadLib("D:\\project\\github.com\\wt201501\\ytlib\\build\\test\\testlib\\Debug\\testlibd.dll");
  // auto fun = (AddFun)(testlib->GetSymbol("add"));
  // int c = fun(1, 2);

  // auto testlib2 = dll_container.LoadLib("D:\\project\\github.com\\wt201501\\ytlib\\build\\test\\testlib2\\Debug\\testlib2d.dll");

  return 0;
}
