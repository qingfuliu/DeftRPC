//
// Created by lqf on 23-5-9.
//

#ifndef DEFTRPC_CHAINOFRESPONSIBILITY_H
#define DEFTRPC_CHAINOFRESPONSIBILITY_H

#include <functional>
#include <memory>

namespace CLSN {

class Processor {
 public:
  constexpr Processor() noexcept = default;

  explicit Processor(Processor *n) noexcept : next(n) {}

  virtual ~Processor() = default;

  virtual void process() = 0;

 private:
  //        std::function<>
  std::unique_ptr<Processor> next;
};

//    template<class RreType>
//    class

}  // namespace CLSN
#endif  // DEFTRPC_CHAINOFRESPONSIBILITY_H
