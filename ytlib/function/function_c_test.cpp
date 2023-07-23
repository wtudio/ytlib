#include <gtest/gtest.h>

#include "function.hpp"

extern "C" typedef struct {
  void (*invoker)(void*);
  void (*relocator)(void* from, void* to);
  void (*destroyer)(void* object);
} test_function_ops_1_t;

extern "C" typedef struct {
  uint32_t (*invoker)(void*, uint32_t);
  void (*relocator)(void* from, void* to);
  void (*destroyer)(void* object);
} test_function_ops_2_t;

extern "C" typedef struct {
  int (*invoker)(void*, bool, double, uint64_t);
  void (*relocator)(void* from, void* to);
  void (*destroyer)(void* object);
} test_function_ops_3_t;

extern "C" typedef struct {
  int (*invoker)(void*);
  void (*relocator)(void* from, void* to);
  void (*destroyer)(void* object);
} test_function_ops_4_t;

namespace ytlib {

TEST(FUNCTION_C_TEST, base) {
  Function<test_function_ops_1_t> f1;

  ASSERT_EQ(sizeof(f1), 4 * sizeof(void*));
  ASSERT_FALSE(f1);

  Function<test_function_ops_1_t> f2(nullptr);
  ASSERT_FALSE(f2);
}

TEST(FUNCTION_C_TEST, Lambda) {
  Function<test_function_ops_2_t> f([](uint32_t in) { return in + 1; });
  ASSERT_TRUE(f);

  ASSERT_EQ(f(42), 43);
}

uint32_t TestPlainFunc2(bool, double, uint64_t) { return 42; }

TEST(FUNCTION_C_TEST, PlainFunc) {
  Function<test_function_ops_3_t> f(TestPlainFunc2);
  ASSERT_EQ(f(false, 1.23, 888), 42);
}

class TestClass2 {
 public:
  static uint32_t foo(bool, double, uint64_t) { return 42; }

  uint32_t bar(bool, double, uint64_t) { return n; }
  uint32_t n = 0;
};

TEST(FUNCTION_C_TEST, MemberMethod) {
  Function<test_function_ops_3_t> f(&TestClass2::foo);
  ASSERT_EQ(f(false, 1.23, 888), 42);

  TestClass2 c{100};
  Function<test_function_ops_3_t> f2(
      std::bind(&TestClass2::bar, &c, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  ASSERT_EQ(f2(false, 1.23, 888), 100);
}

TEST(FUNCTION_C_TEST, LargeFunctorTest) {
  Function<test_function_ops_4_t> f;
  char payload[1000];

  payload[999] = 42;
  f = [payload] { return payload[999]; };
  ASSERT_EQ(42, f());
}

TEST(FUNCTION_C_TEST, FunctorMoveTest) {
  struct OnlyCopyable {
    OnlyCopyable() : v(new std::vector<int>()) {}
    OnlyCopyable(const OnlyCopyable& oc) : v(new std::vector<int>(*oc.v)) {}
    ~OnlyCopyable() { delete v; }
    std::vector<int>* v;
  };
  Function<test_function_ops_4_t> f, f2;
  OnlyCopyable payload;

  payload.v->resize(100, 12);

  // BE SURE THAT THE LAMBDA IS NOT LARGER THAN kOptimizableSize.
  f = [payload] { return payload.v->back(); };
  f2 = std::move(f);
  ASSERT_EQ(12, f2());
}

TEST(FUNCTION_C_TEST, LargeFunctorMoveTest) {
  Function<test_function_ops_4_t> f, f2;
  std::array<std::vector<int>, 100> payload;

  payload.back().resize(10, 12);
  f = [payload] { return payload.back().back(); };
  f2 = std::move(f);
  ASSERT_EQ(12, f2());
}

TEST(FUNCTION_C_TEST, Capture) {
  Function<test_function_ops_1_t> f;
  int x = 0;

  f = [&x]() -> int {
    x = 1;
    return x;
  };
  f();

  ASSERT_EQ(1, x);
}

TEST(FUNCTION_C_TEST, Clear) {
  Function<test_function_ops_1_t> f = [] {};
  ASSERT_TRUE(f);

  f = nullptr;
  ASSERT_FALSE(f);

  std::function<void()> f1 = nullptr;
  Function<test_function_ops_1_t> f2 = f1;
  ASSERT_FALSE(f2);

  f = std::move(f1);
  ASSERT_FALSE(f);

  std::function<void()> f3 = []() {};
  f = std::move(f3);
  ASSERT_TRUE(f);

  Function<test_function_ops_1_t> f4([]() {});
  ASSERT_TRUE(f4);
}

TEST(FUNCTION_C_TEST, NativeHandle) {
  using TestFunctionType = Function<test_function_ops_2_t>;
  TestFunctionType f([](uint32_t in) { return in + 1; });

  ytlib_function_base_t* f_base = f.NativeHandle();
  const auto* ops = static_cast<const test_function_ops_2_t*>(f_base->ops);
  ASSERT_NE(ops, nullptr);

  int ret = ops->invoker(&(f_base->object_buf), 41);
  ASSERT_EQ(ret, 42);
}

}  // namespace ytlib
