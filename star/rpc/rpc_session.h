
#ifndef STAR_RPC_SESSION_H
#define STAR_RPC_SESSION_H

#include "star/net/socket_stream.h"
#include "protocol.h"

namespace star::rpc {
/**
 * @brief rpc session 封装了协议的收发
 */
class RpcSession : public SocketStream {
public:
    using ptr = std::shared_ptr<RpcSession>;
    using MutexType = CoMutex;

    /**
     * @brief 构造函数
     * @param[in] sock Socket类型
     * @param[in] owner 是否托管Socket
     */
    RpcSession(Socket::ptr socket, bool owner = true);
    /**
     * @brief 接收协议
     * @return 如果返回空协议则接收失败
     */
    Protocol::ptr recvProtocol();
    /**
     * @brief 发送协议
     * @param[in] proto 要发送的协议
     * @return 发送大小
     */
    ssize_t sendProtocol(Protocol::ptr proto);

private:
    MutexType m_mutex;
};

inline bool operator == (RpcSession& s1,RpcSession& s2) {
    return s1.getSocket() == s2.getSocket();
}

}
#endif //STAR_RPC_SESSION_H
