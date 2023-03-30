#ifndef STAR_RAFT_CLIENT_H
#define STAR_RAFT_CLIENT_H

#include "star/raft/raft_server.h"
#include "star/rpc/rpc_client.h"
#include "star/config.h"
#include "star/net/address.h"
#include "star/io_manager.h"
#include "star/log.h"
#include "star/raft/common.h"

namespace star{

class Raft_Client {
public:
    Raft_Client();

    // bool append(const std::string& key,const std::string& val);

    bool set(const std::string& key,const std::string& val);

    std::string get(const std::string& key);

    void close();

    ~Raft_Client();
private:
    std::vector<star::Address::ptr> addrs;
    std::vector<star::rpc::RpcClient::ptr> m_servers;
    bool isclose;
};

}

#endif