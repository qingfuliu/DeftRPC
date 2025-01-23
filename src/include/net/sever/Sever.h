//
// Created by lqf on 23-5-10.
//

#ifndef DEFTRPC_TCPSEVER_H
#define DEFTRPC_TCPSEVER_H

#include <sys/socket.h>
#include <cerrno>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include "net/Addr.h"
#include "net/Socket.h"
#include "net/sever/TcpSever.h"

namespace clsn {

class Sever : public clsn : TcpSever {
 public:
};

}  // namespace clsn

#endif  // DEFTRPC_TCPSEVER_H
