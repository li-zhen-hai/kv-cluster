#ifndef STAR_KV_SERVER_H
#define STAR_KV_SERVER_H

#include "star/log.h"
#include "star/raft/raft_server.h"
#include "star/sync.h"
#include "star/rpc/rpc_server.h"
#include "star/raft/common.h"
#include "star/rpc/serializer.h"

#include <map>
#include <string>
#include <atomic>
#include <tuple>
#include <vector>
#include <leveldb/db.h>

namespace star{

class kv_server
{
public:

    enum Key_State{
        UNLOCK,
        LOCK
    };

    using MutexType = CoMutex;
    using ptr = std::shared_ptr<Raft_Server>;
    using Value = std::tuple<std::string,std::shared_ptr<MutexType>,uint64_t>;
    using LogPtr = std::shared_ptr<LogEntry>;
    // using KVSnapshot = std::unordered_map<std::string,std::pair<std::string,uint64_t>>;
    using KVSnapshot = std::map<std::string,std::string>;

    kv_server(std::string m_ip,std::string r_ip,size_t capacity=65535,int maxlogsize=50,bool async_log = false);

    void start();

    bool set(std::string key,std::string value,uint64_t version);

    std::string get(std::string value);

    // Data erase(std::string key);

    // std::vector<std::string> GetAllKey();

    std::vector<std::string> TCC_Try(std::vector<std::string> keys,std::vector<std::string> vals,uint64_t version);
    bool TCC_Commit(std::vector<std::string> keys,uint64_t version);
    bool TCC_Cancel(std::vector<std::string> keys,std::vector<std::string> vals,uint64_t version);

    bool clean();

    bool createSnapshot();

    bool snapshotPersisent();

    void recover_from_snapshot();

    ~kv_server();

    std::map<std::string,std::string> GetAllKV();

    //uint64_t GetMaxVersion();

private:

    bool applylog(std::string mode,std::string key,std::string value,uint64_t version);

    // void applylog(LogEntry entry);

    KVSnapshot GetSnapshot();

    bool ApplySnapshot(KVSnapshot shot,bool f = false);

    bool Try(std::string akey,std::string aval,uint64_t version);
    bool Commit(std::string akey,uint64_t version);
    bool Cancel(std::string akey,std::string aval,uint64_t version);

    bool appendlog(std::string key,std::string val,std::string mode,uint64_t version);

private:

    std::unordered_map<std::string,Value> m_values;
    std::unordered_map<std::string,Key_State> m_states;
    std::unordered_map<std::string,Value> mutli_version;

    leveldb::DB* db;
    const leveldb::Snapshot* snapshot;

    Channel<LogEntry> m_chan;
    Raft_Server::ptr r_server;
    std::unordered_map<uint64_t,std::shared_ptr<Channel<LogEntry>>> channels;

    rpc::RpcServer::ptr m_server;

    bool is_stop;

    MutexType m_mutex;

    //int m_seqid;
    std::atomic<uint64_t> m_seqid;

    //  
    bool is_create_snapshot_run = false;
    //bool is_snapshot_persisent_run = false;

    MutexType create_snapshot_mutex;

};

std::vector<std::string> split(std::string str,std::string flag);

} // namespace star

#endif