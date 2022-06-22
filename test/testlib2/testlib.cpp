#include "testlib.h"

int add(int a, int b) {
  return a + b;
}

int sub(int a, int b) {
  return a - b;
}

class TestC2 {
 public:
  TestC2() {
    printf("TestC2\n");
  }
};
static TestC2 test_c2;
