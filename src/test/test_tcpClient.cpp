//
// Created by lqf on 23-5-11.
//
#include "net/client/TcpClient.h"
#include "log/Log.h"
#include "hook/Hook.h"

int main() {
    Enable_Hook();
    Disable_Enable_Hook();

    CLSN::TcpClient client("0.0.0.0:5201");
    CLSN_LOG_DEBUG << "remote:" << client.GetRemote().toString();
    if (!client.Connect()) {
        CLSN_LOG_ERROR << "connect failed,error is "
                       << strerror(errno);
        return -1;
    }
    std::string str = "test str";
    if (str.size() != client.Send(str)) {
        CLSN_LOG_ERROR << "Send failed,error is "
                       << strerror(errno);
        return -1;
    }
    int res = 0;
    int totalLen = str.size();
    do {
        int temp = client.Receive(str.data() + res, totalLen - res);
        if (temp < 0) {
            CLSN_LOG_ERROR << "Receive failed,error is "
                           << strerror(errno);
            return -1;
        }
        res += temp;
    } while (res < totalLen);
    CLSN_LOG_DEBUG << "receive str is " << str;
    sleep(30);
    client.Close();
    return 0;
};