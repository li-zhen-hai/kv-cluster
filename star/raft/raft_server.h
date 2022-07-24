
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

    enum State{
        Leader_State,
        Follower_State,
        Candidate_State
    };

    using MutexType = CoMutex;
    using ptr = std::shared_ptr<Raft_Server>;
    using GetSnapshotFunc = std::function<std::unordered_map<std::string,std::pair<std::string,uint64_t>>()>;
    using ApplySnapshotFunc = std::function<bool(std::unordered_map<std::string,std::pair<std::string,uint64_t>>)>;

    Raft_Server(std::string ip,Channel<LogEntry> chan,GetSnapshotFunc get = nullptr,ApplySnapshotFunc apply = nullptr,int maxlogsize=50);

    ~Raft_Server();

    bool start();

    State GetState();


    RequestVoteReply requestVote(RequestVoteArgs args);
    AppendLogEntryReply appendlog(AppendLogEntryArgs args);
    InstallSnapshotReply installsnapshot(InstallSnapshotArgs args);
    RequestVoteReply PrepareVote(RequestVoteArgs args);


    int getState();

    int getCommitIndex();


    bool sendAppendLogEntry(int server,AppendLogEntryArgs args);

    bool sendInstallSnapshot(int server,InstallSnapshotArgs args);

    void boardcastHeartBeat();

    void boardcastRequestVote();

    void boardcastPrepareVote();

    void handleRequestVote(RequestVoteReply reply);

    void persisent();

    void recover();

    LogEntry start(std::string log);

    void setGetSnapshot(GetSnapshotFunc func) { GetSnapshot = func;}

    void setApplySnapshot(ApplySnapshotFunc func) { ApplySnapshot = func; }

private:

    bool isUptoMe(int index,int term);

    void update();

    void applyLog();

    void persisentlog();

private:

    Timer::ptr heartBeat;
    MutexType m_mutex;
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

    std::vector<int> nextIndex;
    std::vector<int> matchIndex;

    State state;
    int voteCount;

    std::atomic<int> prepareCount;
    bool winPreVote;

    bool recive_heartbeat = false;

    // std::map<std::string,std::string> m_values;


    int maxLogSize;

    // distri_lock::ptr dis_lock;

    CoCondVar m_con;

    Channel<LogEntry> m_chan;

    GetSnapshotFunc GetSnapshot;
    ApplySnapshotFunc ApplySnapshot;
};



}

#endif

