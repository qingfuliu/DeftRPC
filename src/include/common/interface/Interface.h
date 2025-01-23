#ifndef DEFTRPC_INTERFACE_H
#define DEFTRPC_INTERFACE_H

#include <string>
namespace clsn {

class Interface {
 public:
  virtual bool ParseFromString(const std::string &data) = 0;
  virtual bool SerializeToString(std::string &output) const = 0;
};
}  // namespace clsn

#endif
