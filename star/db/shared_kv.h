#ifndef STAR_SHARED_KV_H
#define STAR_SHARED_KV_H

#include "star/db/kv_server.h"
#include "star/db/kv_client.h"
#include "star/sync.h"

#include <string>
#include <functional>
#include <vector>
#include <map>
#include <atomic>
// #include <queue>

namespace star{

class shared_kv
{
public:

    /**
     * @brief 用来将key分布到kv_server上
     * @param[in] key
     * @return 得到 0 ~ 2^32 的 unsigned int 值
     */
    using hash_function = std::function<unsigned int(std::string)>;

    using MutexType = CoMutex;

    enum PartState{
        Ready=0,
        Run,
        MOVE,
        Close
    };

    /**
     * @brief raft 集群分片信息
     */
    struct partition {
        unsigned int hash;
        size_t pos;
        bool operator < (const partition& thr) const;
    };

    using PartPoint=std::vector<partition>;

    shared_kv(bool autoload=true,hash_function func=nullptr);

    /**
     * @brief 开启 shared-kv 服务
     * @param ip rpc服务绑定的ip X.X.X.X:port
     */
    void start(std::string ip);

    /**
     * @brief 开启 shared-kv 服务
     * @param addr rpc服务绑定的地址
     */
    void start(star::Address::ptr addr);

    /**
     * @brief 服务开启前加添加raft集群,如果在开启之后添加,添加失败,返回-1
     * @param ips 添加的集群的所有ip
     * @return int 连接的句柄
     */
    int addserver(std::vector<std::string> ips);

    /**
     * @brief 服务开启前删除raft集群
     * @param pos 集群连接的句柄
     * @return true 删除成功
     * @return false 删除失败
     */
    bool delserver(int pos);

    /**
     * @brief 添加 KV对
     * @param key 键
     * @param value 值
     * @param flag 是否重试到成功
     * @return true 添加成功
     * @return false 添加失败
     */
    bool set(std::string key,std::string value,bool flag=false);

    /**
     * @brief TCC 实现的简单事务
     * 
     * @param keys 要更改的 key
     * @param vals 更改后的 al
     * @return true 事务成功
     * @return false 事务失败
     */
    bool atomic_set(std::vector<std::string> keys,std::vector<std::string> vals);

    /**
     * @brief 获取 key 的 value
     * @param key 要获取的key值
     * @return std::string key的value值,如果失败返回空字符串
     */
    std::string get(std::string key);

    std::map<std::string,std::string> GetAllKV();

    std::map<int,std::vector<std::string>> GetCluster();

    void close();

    ~shared_kv();
private:

    unsigned int gethash(std::string key);

    bool reloadkey(int part,int client);

    /**
     * @brief hash 算法 
     */
    unsigned int murMurHash(const void* key,int len);

    /**
     * @brief 获取 hash 值对应的 session
     * @param hash_  key 的 一致性hash值
     * @return kv_client::ptr hash 值对应的 session
     */
    kv_client::ptr GetClient(unsigned int hash_);

    /**
     * @brief Get the Client Pos object
     * @param hash_ 键的哈希值
     * @return size_t 连接在连接池里的位置
     */
    size_t GetClientPos(unsigned int hash_);

private:

    int n;
    bool is_stop;
    bool is_start;
    bool auto_reload;
    rpc::RpcServer::ptr m_server;
    hash_function m_hash;
    std::vector<kv_client::ptr> m_sessions;
    std::vector<std::atomic<uint64_t>*> loads;
    PartPoint points;

    MutexType m_mutex;

    std::atomic<uint64_t> idx;
};


}


#endif 