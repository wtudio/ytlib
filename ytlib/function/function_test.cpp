#include <gtest/gtest.h>

#include "function.hpp"

namespace ytlib {

TEST(FUNCTION_TEST, base) {
  Function<void()> f1;

  ASSERT_EQ(sizeof(f1), 4 * sizeof(void*));
  ASSERT_FALSE(f1);

  Function<void()> f2(nullptr);
  ASSERT_FALSE(f2);
}

TEST(FUNCTION_TEST, Lambda) {
  Function f([](uint32_t in) { return in + 1; });
  ASSERT_TRUE(f);

  ASSERT_EQ(f(42), 43);
}

uint32_t TestPlainFunc(bool, double, uint64_t) { return 42; }

TEST(FUNCTION_TEST, PlainFunc) {
  Function f(TestPlainFunc);
  ASSERT_EQ(f(false, 1.23, 888), 42);
}

class TestClass {
 public:
  static uint32_t foo(bool, double, uint64_t) { return 42; }

  uint32_t bar(bool, double, uint64_t) { return n; }
  uint32_t n = 0;
};

TEST(FUNCTION_TEST, MemberMethod) {
  Function<uint32_t(bool, double, uint64_t)> f(&TestClass::foo);
  ASSERT_EQ(f(false, 1.23, 888), 42);

  TestClass c{100};
  Function<uint32_t(TestClass*, bool, double, uint64_t)> f2(&TestClass::bar);
  ASSERT_EQ(f2(&c, false, 1.23, 888), 100);
}

TEST(FUNCTION_TEST, LargeFunctorTest) {
  Function<int()> f;
  char payload[1000];

  payload[999] = 42;
  f = [payload] { return payload[999]; };
  ASSERT_EQ(42, f());
}

TEST(FUNCTION_TEST, FunctorMoveTest) {
  struct OnlyCopyable {
    OnlyCopyable() : v(new std::vector<int>()) {}
    OnlyCopyable(const OnlyCopyable& oc) : v(new std::vector<int>(*oc.v)) {}
    ~OnlyCopyable() { delete v; }
    std::vector<int>* v;
  };
  Function<int()> f, f2;
  OnlyCopyable payload;

  payload.v->resize(100, 12);

  // BE SURE THAT THE LAMBDA IS NOT LARGER THAN kOptimizableSize.
  f = [payload] { return payload.v->back(); };
  f2 = std::move(f);
  ASSERT_EQ(12, f2());
}

TEST(FUNCTION_TEST, LargeFunctorMoveTest) {
  Function<int()> f, f2;
  std::array<std::vector<int>, 100> payload;

  payload.back().resize(10, 12);
  f = [payload] { return payload.back().back(); };
  f2 = std::move(f);
  ASSERT_EQ(12, f2());
}

TEST(FUNCTION_TEST, Capture) {
  Function<void()> f;
  int x = 0;

  f = [&x]() -> int {
    x = 1;
    return x;
  };
  f();

  ASSERT_EQ(1, x);
}

TEST(FUNCTION_TEST, Clear) {
  Function<void()> f = [] {};
  ASSERT_TRUE(f);

  f = nullptr;
  ASSERT_FALSE(f);

  std::function<void()> f1 = nullptr;
  Function<void()> f2 = f1;
  ASSERT_FALSE(f2);

  f = std::move(f1);
  ASSERT_FALSE(f);

  std::function<void()> f3 = []() {};
  f = std::move(f3);
  ASSERT_TRUE(f);

  Function<void()> f4([]() {});
  ASSERT_TRUE(f4);
}

TEST(FUNCTION_TEST, NativeHandle) {
  using TestFunctionType = Function<int(int)>;
  TestFunctionType f([](uint32_t in) { return in + 1; });

  ytlib_function_base_t* f_base = f.NativeHandle();
  const auto* ops = static_cast<const TestFunctionType::OpsType*>(f_base->ops);
  ASSERT_NE(ops, nullptr);

  int ret = ops->invoker(&(f_base->object_buf), 41);
  ASSERT_EQ(ret, 42);
}

}  // namespace ytlib
