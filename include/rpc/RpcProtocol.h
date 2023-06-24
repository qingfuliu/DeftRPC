//
// Created by lqf on 23-6-24.
//

#ifndef DEFTRPC_RPCPROTOCOL_H
#define DEFTRPC_RPCPROTOCOL_H

#include "serialize/StringSerializer.h"
#include <string>

namespace CLSN {
    struct RPCRequest {
        std::string funcName;
        std::string args;
        short async;

        template<class Sr>
        void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr) {
            sr(funcName, args, async);
        }

        template<class Sr>
        void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr) {
            sr(funcName, args, async);
        }
    };
}

#endif //DEFTRPC_RPCPROTOCOL_H
