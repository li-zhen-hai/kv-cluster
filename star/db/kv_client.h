#ifndef STAR_KV_CLIENT_H
#define STAR_KV_CLIENT_H

#include "star/raft/raft_server.h"
#include "star/rpc/rpc_client.h"
#include "star/config.h"
#include "star/net/address.h"
#include "star/io_manager.h"
#include "star/log.h"
#include "star/raft/common.h"

#include <vector>
#include <string>
#include <atomic>

namespace star {

class kv_client {
public:

    using ptr = std::shared_ptr<kv_client>;

    kv_client();

    // bool append(const std::string& key,const std::string& val);

    bool set(const std::string& key,const std::string& val,uint64_t version);

    void set_until_success(const std::string& key,const std::string& val,uint64_t version);

    std::vector<std::string> TCC_Try(std::vector<std::string> keys,std::vector<std::string> vals,uint64_t version);

    bool TCC_Commit(std::vector<std::string> keys,uint64_t version);
    
    bool TCC_Cancel(std::vector<std::string> keys,std::vector<std::string> vals,uint64_t version);

    bool start();

    bool start(std::vector<star::Address::ptr> s_addrs);

    bool start(std::vector<std::string> s_ips);

    std::string get(const std::string& key);

    std::unordered_map<std::string,std::pair<std::string,uint64_t>> GetSnapshot();

    bool ApplySnapshot(std::unordered_map<std::string,std::pair<std::string,uint64_t>> shot);

    std::map<std::string,std::string> GetAllKV();

    std::pair<uint64_t,uint64_t> GetOps();

    int GetServerSize();

    // std::vector<std::string> GetAllKey();

    // Data erase(std::string key);

    void close();

    bool clean();

    ~kv_client();
private:

    bool still_alive();
    
private:
    std::vector<star::Address::ptr> addrs;
    std::vector<star::rpc::RpcClient::ptr> m_servers;
    bool isclose;
    std::atomic<uint64_t> calls;
};
}
#endif