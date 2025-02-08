//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_COMMON_H
#define DEFTRPC_COMMON_H

#include <sys/epoll.h>
#include <memory>
#include <mutex>

namespace clsn {

#if defined(__i386__)
enum {
  kEIP = 0,
  kEBP = 6,
  kESP = 7,
};
#elif defined(__x86_64__)
enum {
  kRDI = 7,
  kRSI = 8,
  kRETAddr = 9,
  kRSP = 13,
};
#endif

using CoroutineFunc = void (*)(void *);

using CoroutineArg = void *;

enum class kCoroutineState : std::int16_t {
  construct = 0,  // before load the register viable
  init,
  executing,
  yield,
  finished,
  terminal  // except
};

extern inline constexpr std::uint32_t DEFAULT_STACK_SIZE = 128 * 1024;

class Noncopyable {
 public:
  Noncopyable() = default;

  virtual ~Noncopyable() = default;

  Noncopyable(const Noncopyable &) = delete;

  Noncopyable &operator=(const Noncopyable &) = delete;
};

//    template<typename T>
//    class Singleton : protected Noncopyable {
//    public:
//
//        static T &GetInstance() noexcept {
//            std::call_once(flag, [] {
//                instance.Reset(new T);
//            });
//            return *instance.get();
//        }
//
//        static const T *GetInstancePtr() noexcept {
//            std::call_once(flag, [] {
//                instance.Reset(new T);
//            });
//            return instance.get();
//        }
//
//    protected:
//        Singleton() = default;
//
//        ~Singleton() override = default;
//
//    private:
//        inline static std::unique_ptr<T> instance{nullptr};
//        inline static std::once_flag flag;
//    };

#define SINGLETON_BASE_DEFINE(X) \
 protected:                      \
  X() = default;                 \
  virtual ~X() = default;        \
  X(const X &) = delete;         \
  X &operator=(const X &) = delete;

#define SINGLETON_DEFINE(X)  \
  friend class Singleton<X>; \
                             \
 private:                    \
  X() = default;             \
  ~X() override = default;   \
  X(const X &) = delete;     \
  X &operator=(const X &) = delete;

template <typename T>
class Singleton {
  SINGLETON_BASE_DEFINE(Singleton)

 public:
  static T &GetInstance() {
    std::call_once(flag, [] {
      auto_release = AutoRelease();
      Singleton::instance = new T{};
    });
    return *instance;
  }

 private:
  class AutoRelease {
   public:
    AutoRelease() noexcept = default;

    ~AutoRelease() { delete instance; }
  };

  inline static std::once_flag flag;
  inline static AutoRelease auto_release;
  inline static T *instance = nullptr;
};

// ********************m_buffer_*************************** //
using PackageLengthType = std::uint32_t;
using Crc32Type = std::uint32_t;
// ********************RPC*************************** //

enum class kRpcType : std::int16_t { Async = 1, Sync = 2 };

}  // namespace clsn

#endif  // DEFTRPC_TYPE_COMMON_H
