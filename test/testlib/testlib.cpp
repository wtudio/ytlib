#include "testlib.h"

int add(int a, int b) {
  return a + b;
}

int sub(int a, int b) {
  return a - b;
}

class TestC1 {
 public:
  TestC1() {
    printf("TestC1\n");
  }
};
static TestC1 test_c1;
