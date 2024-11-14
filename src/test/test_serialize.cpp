//
// Created by lqf on 23-4-24.
//
#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include <type_traits>
#include "serialize/BinarySerialize.h"
#include "serialize/StringSerializer.h"
#include "serialize/detail/config.h"
#include "serialize/detail/helper.h"

using namespace clsn;

struct A {
  A() = default;

  int x;
  int y;
  int z;
  std::shared_ptr<int> ptr;
  std::shared_ptr<int> ptr1;
  std::vector<int> vec;
  std::unique_ptr<int> n;
  std::unique_ptr<int> n1;
  //    template<class Sr, class Construct>
  //    void DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE(Sr &sr, Construct &c) noexcept {
  //        int x1, y1, z1;
  //        sr(x1, y1, z1);
  //        c(x1, y1, z1);
  //    }

  template <class Sr>
  void SerializeFunc(Sr &sr) noexcept {
    sr(x, y, z, ptr, ptr1, vec, n, n1);
  }

  template <class Sr>
  void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr) noexcept {
    sr(x, y, z, ptr, ptr1, vec, n, n1);
  }

  [[nodiscard]] std::string to_string() const noexcept {
    return "X:" + std::to_string(x) + " Y:" + std::to_string(y) + " Z:" + std::to_string(z) +
           " ptr:" + std::to_string(*ptr);
  }
};

// template<class Sr, class T>
// void SerializeFunc(Sr &sr, const T &) noexcept {
//
// }
//
// template<class Sr, class T>
// void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, const T &) noexcept {
//
// }

// template<class Sr, class Construct>
// void DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE(Sr &sr, Construct &c) noexcept {
//     int x1, y1, z1;
//     sr(x1, y1, z1);
//     c(x1, y1, z1);
// }

namespace temp {

template <class sr, typename T,
          typename v = std::enable_if<
              std::is_same_v<void, decltype(DeSerializeFunc(std::declval<sr &>(), std::declval<T &&>()))>>>
std::true_type has_no_member_deserialize_func_helper(int);

template <class sr, typename T>
std::false_type has_no_member_deserialize_func_helper(...);

template <class Sr, typename T>
struct has_no_member_deserialize_func : public decltype(has_no_member_deserialize_func_helper<std::decay_t<Sr>, T>(0)) {
};

template <class sr, typename T,
          typename v = std::enable_if<std::is_same_v<
              void, decltype(LoadAndConstruct(std::declval<sr &>(),
                                              std::declval<Construct<std::remove_reference_t<T>> &>()))>>>
std::true_type has_no_member_load_and_construct_func_helper(int);

template <class sr, typename T>
std::false_type has_no_member_load_and_construct_func_helper(...);

template <class Sr, typename T>
struct has_no_member_load_and_construct_func
    : public decltype(has_no_member_load_and_construct_func_helper<std::decay_t<Sr>, T>(0)) {};
template <class Sr, typename T>
inline constexpr bool has_no_member_load_and_construct_func_v = has_no_member_load_and_construct_func<Sr, T>::value;

}  // namespace m_temp_

static void test_binary_serialize() {
  {
    std::ofstream f("/home/lqf/project/DeftRPC/src/test/binary.txt", std::ios_base::ate | std::ios_base::binary);
    if (!f.is_open()) {
      std::cout << "open file failed!" << std::endl;
    }

    BinarySerialize bsr(f);
    A a;
    a.SerializeFunc(bsr);

    std::cout << "A" << std::endl;
    std::cout << has_member_serialize_func_v<BinarySerialize, A> << std::endl;
    std::cout << has_no_member_serialize_func_v<BinarySerialize, A> << std::endl;

    std::cout << has_member_deserialize_func_v<BinarySerialize, A &> << std::endl;
    std::cout << has_no_member_deserialize_func<BinarySerialize, A &>::value << std::endl;

    std::cout << has_member_load_and_construct_func<BinarySerialize, A &>::value << std::endl;

    //        using p = std::enable_if_t<
    //                std::is_same_v<
    //                        void,
    //                        decltype(LoadAndConstruct(std::declval<BinarySerialize &>(), std::declval<Construct<A>
    //                        &>()))>>;

    std::cout << has_no_member_load_and_construct_func<BinarySerialize, A &>::value << std::endl;

    std::cout << "vec" << std::endl;
    std::cout << has_member_serialize_func_v<BinarySerialize, std::vector<int>> << std::endl;
    std::cout << has_no_member_serialize_func_v<BinarySerialize, std::vector<int>> << std::endl;

    using p1 = std::enable_if<std::is_same_v<void, decltype(DeSerializeFunc(std::declval<BinarySerialize &>(),
                                                                            std::declval<std::vector<int> &>()))>>;

    std::cout << has_member_deserialize_func_v<BinarySerialize, std::vector<int>> << std::endl;
    std::cout << has_no_member_deserialize_func<BinarySerialize, std::vector<float> &>::value << std::endl;

    std::cout << has_member_load_and_construct_func<BinarySerialize, std::vector<float> &>::value << std::endl;
    std::cout << has_no_member_load_and_construct_func<BinarySerialize, std::vector<float> &>::value << std::endl;
  }
  //    b(a);
}

static void test_binary_serialize_vector() {
  {
    {
      std::vector<float> vec(3, 10.2);
      std::ofstream f("/home/lqf/project/DeftRPC/src/test/binary.txt", std::ios_base::ate | std::ios_base::binary);
      if (!f.is_open()) {
        std::cout << "open file failed!" << std::endl;
      }
      BinarySerialize bsr(f);
      bsr(vec);

      f.close();
    }

    {
      std::vector<float> vec;
      std::ifstream f("/home/lqf/project/DeftRPC/src/test/binary.txt", std::ios_base::binary);
      if (!f.is_open()) {
        std::cout << "open file failed!" << std::endl;
      }
      BinaryDeSerialize bsr(f);
      bsr(vec);
      f.close();
      for (const auto &v : vec) {
        std::cout << v << " ";
      }
      std::cout << std::endl;
    }
  }
  //    b(a);
}

static void test_binary_serialize_tuple() {
  {
    {
      std::tuple<float, float, std::string> tp(3, 10.2, "wangfei222");
      std::ofstream f("/home/lqf/project/DeftRPC/src/test/binary.txt", std::ios_base::ate | std::ios_base::binary);
      if (!f.is_open()) {
        std::cout << "open file failed!" << std::endl;
      }
      BinarySerialize bsr(f);
      bsr(tp);

      f.close();
    }

    {
      std::tuple<float, float, std::string> tp;
      std::ifstream f("/home/lqf/project/DeftRPC/src/test/binary.txt", std::ios_base::binary);
      if (!f.is_open()) {
        std::cout << "open file failed!" << std::endl;
      }
      BinaryDeSerialize bsr(f);
      bsr(tp);
      f.close();
    }
  }
  //    b(a);
}

static void test_string_serialize_tuple() {
  {
    std::string res;
    {
      std::tuple<float, float, std::string> tp(3, 10.2, "wangfei222");
      StringSerialize bsr(res);
      bsr(tp);
    }

    {
      std::tuple<float, float, std::string> tp;
      StringDeSerialize bsr{std::string_view(res)};
      bsr(tp);
      int a = 1;
    }
  }
  //    b(a);
}

static void test_binary_serialize_memory() {
  {
    {
      std::shared_ptr<int> ptr = std::make_shared<int>(1);
      std::ofstream f("/home/lqf/project/DeftRPC/src/test/binary.txt", std::ios_base::ate | std::ios_base::binary);
      if (!f.is_open()) {
        std::cout << "open file failed!" << std::endl;
      }
      BinarySerialize bsr(f);
      bsr(ptr);
      f.close();
    }

    {
      std::shared_ptr<int> ptr = std::make_shared<int>(1);
      std::ifstream f("/home/lqf/project/DeftRPC/src/test/binary.txt", std::ios_base::binary);
      if (!f.is_open()) {
        std::cout << "open file failed!" << std::endl;
      }
      BinaryDeSerialize bsr(f);
      bsr(ptr);
      std::cout << "test_binary_serialize_memory " << *ptr << std::endl;
      f.close();
    }
  }
  //    b(a);
}

static void test_binary_serialize_memory_complex() {
  {
    {
      //            A a{1,2,3};
      std::shared_ptr<A> ptr = std::make_shared<A>();
      ptr->x = 1;
      ptr->y = 2;
      ptr->z = 3;
      ptr->ptr.reset(new int(1));
      ptr->ptr1 = ptr->ptr;
      ptr->vec = {1, 2, 4, 5};
      ptr->n1 = std::make_unique<int>(1);
      std::ofstream f("/home/lqf/project/DeftRPC/src/test/binary.txt", std::ios_base::ate | std::ios_base::binary);
      if (!f.is_open()) {
        std::cout << "open file failed!" << std::endl;
      }
      BinarySerialize bsr(f);
      bsr(ptr);
      std::cout << "test_binary_serialize_memory_complex: " << ptr->to_string()
                << " use count: " << ptr->ptr.use_count() << " " << ptr->ptr1.use_count() << std::endl;
      f.close();
    }

    {
      std::shared_ptr<A> ptr = std::make_shared<A>();
      std::ifstream f("/home/lqf/project/DeftRPC/src/test/binary.txt", std::ios_base::binary);
      if (!f.is_open()) {
        std::cout << "open file failed!" << std::endl;
      }
      BinaryDeSerialize bsr(f);
      bsr(ptr);
      std::cout << "test_binary_serialize_memory_complex: " << ptr->to_string()
                << " use count: " << ptr->ptr.use_count() << " " << ptr->ptr1.use_count() << std::endl;
      f.close();
    }
  }
}

static void test_binary_string() {
  {
    {
      std::string str = "wangfei222";
      std::ofstream f("/home/lqf/project/DeftRPC/src/test/binary.txt", std::ios_base::ate | std::ios_base::binary);
      if (!f.is_open()) {
        std::cout << "open file failed!" << std::endl;
      }
      BinarySerialize bsr(f);
      bsr(str);
      f.close();
    }

    {
      std::string str = "wangfei222";
      std::ifstream f("/home/lqf/project/DeftRPC/src/test/binary.txt", std::ios_base::binary);
      if (!f.is_open()) {
        std::cout << "open file failed!" << std::endl;
      }
      BinaryDeSerialize bsr(f);
      bsr(str);
      std::cout << "test_binary_string " << str << std::endl;
      f.close();
    }
  }
  //    b(a);
}

TEST(test_serialize, test_serialize) {}
