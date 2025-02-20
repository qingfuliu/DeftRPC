#include <gtest/gtest.h>
#include <iostream>
#include "common/type.h"
using namespace clsn;

static void get_index_type_test() {
  class A {
   public:
    A() = default;
  };

  //    for (auto _: state) {
  get_index_type<int, float, double, A>::type<2> a;
  get_index_type<int, float, double, A>::type<1> b;
  int c = 1;
  get_index_type<int &, float, double, A>::type<0> c1 = c;
  get_index_type<int, float, double, A>::type<3> d;
  std::cout << std::string(typeid(a).name()) << std::endl;
  std::cout << std::string(typeid(b).name()) << std::endl;
  std::cout << std::string(typeid(c1).name()) << std::endl;
  std::cout << std::string(typeid(d).name()) << std::endl << std::string(typeid(A).name()) << std::endl;
  //    }
}

// BENCHMARK(get_index_type_test)->Iterations(1)->Threads(1);

TEST(test_common, test_common) { get_index_type_test(); }