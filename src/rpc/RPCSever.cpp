//
// Created by lqf on 23-6-23.
//
#include "log/Log.h"
#include "rpc/RPCSever.h"

namespace CLSN {
    void RPCSever::Start(int timeout) noexcept {
#ifdef DEBUG
        auto it = router->GetIterator();
        CLSN_LOG_DEBUG
                    << "============================================rpc functions============================================";
        while (it->IsValid()) {
            CLSN_LOG_DEBUG << HashTable::GetKeyByIterator(it.get());
            it->Next();
        }
        CLSN_LOG_DEBUG
                    << "============================================rpc functions============================================";


#else
#endif

        TcpSever::Start(timeout);
    }

} // CLSN