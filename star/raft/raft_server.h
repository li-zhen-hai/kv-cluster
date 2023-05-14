
#ifndef STAR_RAFT_SERVER_H
#define STAR_RAFT_SERVER_H

#include <vector>
#include <string>
#include <unordered_map>

#include "star/rpc/protocol.h"
#include "star/rpc/rpc_client.h"
#include "star/rpc/rpc_server.h"
#include "star/timer.h"
#include "star/sync.h"
#include "star/config.h"
#include "star/rpc/rpc.h"
#include "star/hook.h"
#include "star/net/tcp_server.h"
// #include "star/sync/distri_lock.h"
#include "common.h"
#include "star/sync/co_condvar.h"
#include "star/sync/channel.h"
//#include "proto/test.pb.h"

#include <vector>
#include <cstdlib>
#include <algorithm>
#include <atomic>
#include <unordered_map>
#include <string>

namespace star{


class Raft_Server : public TcpServer
{
public:


    /**
     * @brief raft的状态
     */
    enum State{
        Leader_State,
        Follower_State,
        Candidate_State
    };

    using MutexType = CoMutex;
    using ptr = std::shared_ptr<Raft_Server>;

    /**
     * @brief 获取快照的函数
     */
    using GetSnapshotFunc = std::function<std::map<std::string,std::string>()>;

    /**
     * @brief 应用快照的函数
     */
    using ApplySnapshotFunc = std::function<bool(std::map<std::string,std::string>)>;


    using CreateSnapshotFunc = std::function<bool()>;

    using SnapshotPersisentFunc = std::function<bool()>;

    /**
     * @brief Construct a new Raft_Server object
     * 
     * @param[in] ip raft 监听的 ip 地址
     * @param[in] chan 和 kv-server 通信的消息队列
     * @param[in] get 获取快照的回调
     * @param[in] apply 应用快照的回调
     * @param[in] create 创建快照的回调
     */
    Raft_Server(std::string ip,Channel<LogEntry> chan,GetSnapshotFunc get = nullptr,ApplySnapshotFunc apply = nullptr,CreateSnapshotFunc create = nullptr,SnapshotPersisentFunc persisent = nullptr,bool async_log = false);

    ~Raft_Server();

    /**
     * @brief raft 启动
     * 
     * @return true 启动成功
     * @return false 启动失败
     */
    bool start();

    /**
     * @brief 获取当前状态机状态
     * 
     * @return State 状态机的状态 Leader_State, Follower_State, Candidate_State
     */
    State GetState();

    /**
     * @brief 获取投票
     * 
     * @param args 请求者的状态
     * @return RequestVoteReply 投票的返回
     */
    RequestVoteReply requestVote(RequestVoteArgs args);

    /**
     * @brief 添加日志，也即心跳
     * 
     * @param args 添加日志的信息
     * @return AppendLogEntryReply 添加日志的返回
     */
    AppendLogEntryReply appendlog(AppendLogEntryArgs args);

    /**
     * @brief 安装快照
     * 
     * @param args 快照
     * @return InstallSnapshotReply 安装快照的返回值
     */
    InstallSnapshotReply installsnapshot(InstallSnapshotArgs args);

    /**
     * @brief 预选举，不实际自增term，只是发送 请求投票
     * 
     * @param args 
     * @return RequestVoteReply 
     */
    RequestVoteReply PrepareVote(RequestVoteArgs args);

    /**
     * @brief 返回状态码
     * 
     * @return int 状态码
     */
    int getState();

    /**
     * @brief 获取已提交日志的 index
     * 
     * @return int commitIndex
     */
    int getCommitIndex();

    /**
     * @brief 向指定 server 发送 appendLog
     * 
     * @param server server的id
     * @param args
     * @return true 
     * @return false 
     */
    bool sendAppendLogEntry(int server,AppendLogEntryArgs args);

    /**
     * @brief 向指定 server 发送 InstallSnapshot
     * 
     * @param server server 的 id
     * @param args 
     * @return true 
     * @return false 
     */
    bool sendInstallSnapshot(int server,InstallSnapshotArgs args);

    /**
     * @brief 发起心跳或者向 follower 添加日志
     * 
     */
    void boardcastHeartBeat();

    /**
     * @brief 发起投票
     * 
     */
    void boardcastRequestVote();

    /**
     * @brief 发起预选举
     * 
     */
    void boardcastPrepareVote();

    /**
     * @brief 对投票请求进行处理
     * 
     * @param reply 
     */
    void handleRequestVote(RequestVoteReply reply);

    /**
     * @brief 达到指定最大日志长度就持久化日志
     * 
     */
    void persisent();

    /**
     * @brief 在 raft 重启之后进行日志恢复
     * 
     */
    void recover();

    /**
     * @brief 尝试提交日志
     * 
     * @param log 
     * @return LogEntry 
     */
    LogEntry start(DB_log i);

    /**
     * @brief 设置获取日志回调
     * 
     * @param func 
     */
    void setGetSnapshot(GetSnapshotFunc func) { GetSnapshot = func;}

    /**
     * @brief 设置应用日志回调
     * 
     * @param func 
     */
    void setApplySnapshot(ApplySnapshotFunc func) { ApplySnapshot = func; }


    bool createSnapshot();

    int GetId() { return id; }

private:

    /**
     * @brief 判断日志是否比自己新
     * 
     * @param index 日志的 index
     * @param term 日志的任期
     * @return true 
     * @return false 
     */
    bool isUptoMe(int index,int term);

    void update();

    void applyLog();

    void persisentlog();

    int GetLiveNode();

    bool reopen();

    void async_persisent_log();

private:

    Timer::ptr heartBeat;
    /**
     * @brief 保护类内变量
     * 
     */
    MutexType m_mutex;

    /**
     * @brief 确保只有一个线程在操作持久化文件
     * 
     */
    MutexType p_mutex;

    int id = 0;
    std::string me;
    std::vector<std::string> m_addrs;
    std::map<std::string,int> m_servers;
    std::vector<star::rpc::RpcClient::ptr> servers;

    rpc::RpcServer::ptr m_server;

    int currentTerm;
    int votedFor;

    std::vector<LogEntry> log;

    int commitIndex;
    int lastApplied;

    /**
     * @brief 记录持久化log的index
     * 
     */
    int persisentLog;

    std::vector<int> nextIndex;
    std::vector<int> matchIndex;

    State state;
    int voteCount;

    std::atomic<int> prepareCount;
    bool winPreVote;

    bool recive_heartbeat = false;

    int maxLogSize;

    // CoCondVar m_con;

    Channel<LogEntry> m_chan;
    
    // 异步持久化日志
    Channel<LogEntry> async_persisent_log_chan;
    // 是否开启异步持久化日志
    bool is_async_log;

    GetSnapshotFunc GetSnapshot;
    ApplySnapshotFunc ApplySnapshot;
    CreateSnapshotFunc CreateSnapshot;
    SnapshotPersisentFunc snapshotPersisent;

    /**
     * @brief 实现 leader_lease
     * 
     */
    Timer::ptr lease;
    bool lease_time;

    bool persisent_is_run;

    std::atomic<int> prepare_vote_try_count;

    //int fd;
};
}

#endif

