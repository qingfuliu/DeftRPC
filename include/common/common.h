//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_COMMON_H
#define DEFTRPC_COMMON_H

#include <sys/epoll.h>
#include <memory>
#include <mutex>

namespace CLSN {

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

enum class CoroutineState : short {
  construct = 0,  // before load the register viable
  init,
  executing,
  yield,
  finished,
  terminal  // except
};

extern inline constexpr unsigned DefaultStackSize = 1024 * 1024 * 1280;

enum class Event : uint32_t { Read = EPOLLIN, Write = EPOLLOUT, Error = EPOLLERR };

inline constexpr size_t MAXEPOLLSIZE = 1024 << 1;

class noncopyable {
 public:
  noncopyable() = default;

  virtual ~noncopyable() = default;

  noncopyable(const noncopyable &) = delete;

  noncopyable &operator=(const noncopyable &) = delete;
};

//    template<typename T>
//    class Singleton : protected noncopyable {
//    public:
//
//        static T &GetInstance() noexcept {
//            std::call_once(flag, [] {
//                instance.reset(new T);
//            });
//            return *instance.get();
//        }
//
//        static const T *GetInstancePtr() noexcept {
//            std::call_once(flag, [] {
//                instance.reset(new T);
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

#define SINGLETON_DEFINE(X)        \
  friend class Singleton<X>;       \
                                   \
 private:                          \
  X() = default;                   \
  virtual ~X() override = default; \
  X(const X &) = delete;           \
  X &operator=(const X &) = delete;

template <typename T>
class Singleton {
  SINGLETON_BASE_DEFINE(Singleton)

 public:
  static T &getInstance() {
    std::call_once(flag, [] {
      autoRelease = AutoRelease();
      Singleton::instance = new T{};
    });
    return *instance;
  }

 private:
  class AutoRelease {
   public:
    AutoRelease() noexcept = default;

    ~AutoRelease() {
      if (instance != nullptr) delete instance;
    }
  };

  inline static std::once_flag flag;
  inline static AutoRelease autoRelease;
  inline static T *instance = nullptr;
};

//********************buffer***************************//
using PackageLengthType = uint32_t;
using Crc32Type = uint32_t;
//********************RPC***************************//

enum class RpcType : short { Async = 1, Sync = 2 };

}  // namespace CLSN

#endif  // DEFTRPC_TYPE_COMMON_H
