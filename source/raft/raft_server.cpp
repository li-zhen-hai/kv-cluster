#include "star/raft/raft_server.h"
#include "star/config.h"
#include "star/rpc/rpc_session.h"
#include "star/rpc/serializer.h"

#include <unistd.h>
#include <fcntl.h>
#include <functional>
#include <sys/stat.h>
#include <fcntl.h>

namespace star{

static ConfigVar<std::vector<std::string>>::ptr g_raft_servers =
        Config::Lookup<std::vector<std::string>>("raft_server",{},"server");
 

static ConfigVar<uint64_t>::ptr g_heartbeat_time = Config::Lookup<uint64_t>("g_heartbeat_time",5000,"raft_heartBeat_time");

static ConfigVar<uint64_t>::ptr g_rand_time = Config::Lookup<uint64_t>("g_rand_time",10000,"raft_heartBeat_time");

bool Raft_Server::isUptoMe(int index,int term) {
        return term > log.back().term || (term == log.back().term && index >= log.back().index);
}

Raft_Server::Raft_Server(std::string ip,Channel<LogEntry> chan,GetSnapshotFunc get,ApplySnapshotFunc apply,int maxlogsize)
        :me(ip)
        ,currentTerm(0)
        ,votedFor(-1)
        ,commitIndex(0)
        ,lastApplied(0)
        ,persisentLog(0)
        ,state(State::Follower_State)
        ,voteCount(0)
        ,prepareCount(0)
        ,maxLogSize(maxlogsize)
        ,m_chan(chan)
        ,GetSnapshot(get)
        ,ApplySnapshot(apply)
        ,lease_time(false){

        recover();
        if(log.size() == 0)
                log.push_back(LogEntry{0,0,""});
        for(size_t i=0;i<g_raft_servers->getValue().size();++i){
                if(g_raft_servers->getValue()[i]==me)
                        id = i;
                m_addrs.push_back(g_raft_servers->getValue()[i]);
                m_servers[g_raft_servers->getValue()[i]] = i;
                star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
                //if(id != (int)i)
                        client->connect(Address::LookupAny(m_addrs[i]));
                servers.push_back(client);
        }
}

Raft_Server::~Raft_Server(){
}

bool Raft_Server::start(){
        m_server.reset(new star::rpc::RpcServer());
        star::Address::ptr address = star::Address::LookupAny(me);
        
        std::function<AppendLogEntryReply(AppendLogEntryArgs)> fun1 = std::bind(&Raft_Server::appendlog,this,std::placeholders::_1);
        std::function<RequestVoteReply(RequestVoteArgs)> fun2 = std::bind(&Raft_Server::requestVote,this,std::placeholders::_1);
        std::function<InstallSnapshotReply(InstallSnapshotArgs)> fun3 = std::bind(&Raft_Server::installsnapshot,this,std::placeholders::_1);

        std::function<RequestVoteReply(RequestVoteArgs)> fun7 = std::bind(&Raft_Server::PrepareVote,this,std::placeholders::_1);

        if(GetSnapshot == nullptr){
                STAR_LOG_FATAL(STAR_LOG_ROOT()) << "GetSnapshot is nullptr,please call setGetSnapshot!";
                exit(1);
        }

        if(ApplySnapshot == nullptr){
                STAR_LOG_FATAL(STAR_LOG_ROOT()) << "ApplySnapshot is nullptr,please call setApplySnapshot!";
                exit(1);
        }

        m_server->registerMethod("requestVote",fun2);
        m_server->registerMethod("appendlog",fun1);
        m_server->registerMethod("installsnapshot",fun3);
        m_server->registerMethod("PrepareVote",fun7);
        m_server->setName("Raft-Server");

        while(!m_server->bind(address)){
                sleep(1);
        }
        
        IOManager::GetThis()->addTimer(5000,[this](){
                MutexType::Lock lock(p_mutex);
                // STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "Time persisent run ,size is " << log.size() << ",[0]value " << log[0].value;
                if(commitIndex-persisentLog > 0)
                        persisentlog();
        },true);

        usleep(g_heartbeat_time->getValue()+rand()%g_rand_time->getValue());
        update();

        return m_server->start();
}
void Raft_Server::update(){
         if(state == State::Leader_State) {
                 //STAR_LOG_INFO(STAR_LOG_ROOT()) <<"leader state update";
                 if(!heartBeat){
                         heartBeat = IOManager::GetThis()->addTimer(g_heartbeat_time->getValue(),[this](){
                                 //go [this] {
                                         //this->update();
                                        if(GetLiveNode() > (int)servers.size()/2)
                                                this->boardcastHeartBeat();
                                        else
                                                state = State::Follower_State;
                                 //};
                         },true);
                         return ;
                 }
                 heartBeat->reset(g_heartbeat_time->getValue(),[this](){
                         //go [this] {
                                 //this->update();
                                 this->boardcastHeartBeat();
                         //};
                 },true);
	}else{
                 srand((unsigned)time(NULL));
                 //STAR_LOG_INFO(STAR_LOG_ROOT()) <<"other state update";
                 if(!heartBeat){
                         heartBeat = IOManager::GetThis()->addTimer(g_heartbeat_time->getValue()+rand()%g_rand_time->getValue(),[this](){
                                 //go [this] {
                                         //this->update();
                                         //this->state = State::Candidate_State;
                                         //this->overtime = true;
                                         this->boardcastRequestVote();
                                 //};
                         },true);
                         return ;
                 }
                 heartBeat->reset(g_heartbeat_time->getValue()+rand()%g_rand_time->getValue(),[this](){
                         //go [this] {
                                 //this->update();
                                 //this->state = State::Candidate_State;
                                 //this->overtime = true;
                                 this->boardcastRequestVote();
                         //};
                 },true);
                if(!lease){
                        lease = IOManager::GetThis()->addTimer(g_heartbeat_time->getValue(),[this](){
                                lease_time = false;
                        });
                }else{
                        lease->refresh();
                }
                lease_time = true;
         }
        return ;
}


bool Raft_Server::sendAppendLogEntry(int server,AppendLogEntryArgs args){

        if(!servers[server]->isConnected()){
                star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
                new_client->setTimeout(g_heartbeat_time->getValue());
                if(new_client->connect(star::Address::LookupAny(m_addrs[server]))){
                        MutexType::Lock mutex(m_mutex);
                        servers[server]=new_client;
                }else{
                        return false;
                }
        }

        //STAR_LOG_DEBUG(STAR_LOG_ROOT()) << me << " send appendlog to "<<server;

        rpc::Result<AppendLogEntryReply> res = servers[server]->call<AppendLogEntryReply>("appendlog",args);

        MutexType::Lock lock(m_mutex);

        if(res.getCode() != rpc::RpcState::RPC_SUCCESS || state != State::Leader_State || args.term != currentTerm)
                return false;
        AppendLogEntryReply reply;
        reply = res.getVal();
        //STAR_LOG_DEBUG(STAR_LOG_ROOT()) << server <<" reply "<<reply.nextTryLog <<","<< reply.term<<","<<args.entries.size();
        if(reply.term > currentTerm) {
                currentTerm = reply.term;
                state = State::Follower_State;
                votedFor = -1;
                update();
                return true;
        }

        if(reply.success) {
                if(args.entries.size() > 0) {
                        nextIndex[server] = args.entries[args.entries.size()-1].index+1;
                        matchIndex[server] = nextIndex[server]-1;
                }
        }else{
                nextIndex[server] = std::min(reply.nextTryLog,log.back().index);
        }

        int baseIndex = log[0].index;

        for(int n = log.back().index;n>commitIndex && log[n-baseIndex].term == currentTerm;n--) {
                int count = 1;
                for (size_t i=0;i<matchIndex.size();++i)
                        if((int)i != id && matchIndex[i] >= n)
                                count++;

                if(count > (int)m_addrs.size()/2){
                        commitIndex = n;
                        go [this] {
                                this->applyLog();
                        };
                        break;
                }
        }
        //STAR_LOG_DEBUG(STAR_LOG_ROOT()) << server << " reply end!";
        return true;
}

void Raft_Server::boardcastHeartBeat(){
        //STAR_LOG_INFO(STAR_LOG_ROOT()) << id <<" begin heartBeat";
        MutexType::Lock lock(m_mutex);

        int baseIndex = log[0].index;

        for(size_t i=0;i<m_addrs.size();++i){
                if((int)i!=id && state == Leader_State) {
                        if(nextIndex[i] > baseIndex) {
                                //STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "nextindex["<<i<<"] is " <<nextIndex[i]; 
                                AppendLogEntryArgs args;
                                args.term = currentTerm;
                                args.leaderId = id;
                                args.prevLogIndex = nextIndex[i]-1;
                                if(args.prevLogIndex >= baseIndex)
                                        args.prevLogTerm = log[args.prevLogIndex-baseIndex].term;
                                if(nextIndex[i] <= log.back().index) {
                                        args.entries = std::vector<LogEntry>(&log[nextIndex[i]-baseIndex],&log[log.size()]);
                                }
                                //STAR_LOG_INFO(STAR_LOG_ROOT()) << "entry size is "<<args.entries.size();
                                args.leaderCommit = commitIndex;

                                go [this,args,i] {
                                        this->sendAppendLogEntry(i,args);
                                };
                        }else{
                                InstallSnapshotArgs args;
                                args.term = currentTerm;
                                args.leaderId = id;
                                args.lastIncludeIndex = log[0].index;
                                args.lastIncludeTerm = log[0].term;
                                if(GetSnapshot != nullptr)
                                        args.data = GetSnapshot();
                                else {
                                        STAR_LOG_FATAL(STAR_LOG_ROOT()) << "Can not create Snapshot,GetSnapshot is nullptr!";
                                        exit(1);
                                }

                                go [this,i,args] {
                                        this->sendInstallSnapshot(i,args);
                                };
                        }
                }
        }
        go [this] {
                this->persisent();
        };
        update();
}

void Raft_Server::boardcastRequestVote(){

        if(state != State::Leader_State) {
                        prepareCount={0};
                        boardcastPrepareVote();
                        //sleep(g_heartbeat_time->getValue());
                        usleep(g_heartbeat_time->getValue());
                        //STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "PreVote " << winPreVote;
                        if(!winPreVote){
                                update();
                                return ;
                        }
                //STAR_LOG_INFO(STAR_LOG_ROOT()) << "boardcastRequestVote!!!!";
                {
                        MutexType::Lock lock(m_mutex);
                        state = State::Candidate_State;
                        currentTerm = currentTerm + 1;
                        //STAR_LOG_INFO(STAR_LOG_ROOT()) << "currentTerm is "<< currentTerm;
                        votedFor = id;
                        voteCount = 1;
                }

                RequestVoteArgs args;
                args.term = currentTerm;
                args.candidateId = id;
                args.lastLogIndex = log.back().index;
                args.lastLogTerm = log.back().term;

                for(size_t i=0;i<m_addrs.size();++i){
                        if((int)i == id)
                                continue;
                        go [this,i,args] {
                                if(this->state != State::Candidate_State)
                                        return ;
                                if(!servers[i]->isConnected()){
                                        star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
                                        new_client->setTimeout(g_heartbeat_time->getValue());
                                        if(new_client->connect(star::Address::LookupAny(m_addrs[i]))){
                                                MutexType::Lock mutex(m_mutex);
                                                servers[i]=new_client;
                                        }else{
                                                return;
                                        }
                                }
                                rpc::Result<RequestVoteReply> res = servers[i]->call<RequestVoteReply>("requestVote",args);
                                MutexType::Lock lock(this->m_mutex);
                                if(res.getCode() == rpc::RpcState::RPC_SUCCESS){
                                        RequestVoteReply reply = res.getVal();
                                        this->handleRequestVote(reply);
                                }
                        };
                }
                winPreVote = false;
        }
}

AppendLogEntryReply Raft_Server::appendlog(AppendLogEntryArgs args) {

        MutexType::Lock lock(m_mutex);
        // STAR_LOG_INFO(STAR_LOG_ROOT()) << "term is " << args.term
        //                                << ",prevlogIndex is " << args.prevLogIndex
        //                                << ",prevlogterm is " << args.prevLogTerm
        //                                << ",leaderId is " << args.leaderId;

        AppendLogEntryReply reply;
        reply.success = false;

        if(args.term < currentTerm) {
                reply.term = currentTerm;
                reply.nextTryLog = log.back().index+1;
                return reply;
        }

        update();
        recive_heartbeat = true;

        if(args.term > currentTerm) {
                state = State::Follower_State;
                currentTerm = args.term;
                votedFor = args.leaderId;
                update();
        }

        reply.term = currentTerm;
        if(args.prevLogIndex > log.back().index) {
                reply.nextTryLog = log.back().index+1;
                return reply;
        }

        int baseIndex = log[0].index;
        if (args.prevLogIndex >= baseIndex && args.prevLogTerm != log[args.prevLogIndex-baseIndex].term) {
                int term = log[args.prevLogIndex - baseIndex].term;
                for(size_t i=args.prevLogIndex-1;i>=(size_t)baseIndex;--i){
                        if(log[i-baseIndex].term != term) {
                                reply.nextTryLog = i+1;
                                return reply;
                        }
                }
        }else if(args.prevLogIndex >= baseIndex-1) {
                int tmp = args.prevLogIndex-baseIndex+1;
                log = std::vector<LogEntry>(&log[0],&log[tmp]);
                for(size_t i=0;i<args.entries.size();++i)
                        log.push_back(args.entries[i]);
                
                reply.success = true;
                reply.nextTryLog = args.prevLogIndex + args.entries.size();

                if(commitIndex < args.leaderCommit) {
                        commitIndex = std::min(args.leaderCommit,log.back().index);
                        go [this] {
                                this->applyLog();
                        };
                }
        }
        
        go [this] {
                this->persisent();
        };
        update();
        return reply;
}

RequestVoteReply Raft_Server::requestVote(RequestVoteArgs args){
        // STAR_LOG_INFO(STAR_LOG_ROOT()) << "receive requestvote";
        // STAR_LOG_INFO(STAR_LOG_ROOT()) << "term is " << args.term <<" ,currentterm is " << currentTerm
        //                                << ",candidated is " << args.candidateId << " me is " << id
        //                                << ",lastLogIndex is " << args.lastLogIndex << " mylastlog index is " << log.back().index 
        //                                << ",LastLogTerm is " << args.lastLogTerm << "mylastlog term is " << log.back().term;
        //m_mutex.lock();
        MutexType::Lock lock(m_mutex);
        RequestVoteReply reply;
        if(args.term < currentTerm || lease_time) {
                reply.term = currentTerm;
                reply.voteGranted = false;
                return reply;
        }

        if(args.term > currentTerm) {
                currentTerm = args.term;
                state = State::Follower_State;
                votedFor = -1;
                //update();
        }

        reply.term = currentTerm;
        reply.voteGranted = false;

        if ((votedFor == -1 || votedFor == args.candidateId) && isUptoMe(args.lastLogIndex,args.lastLogTerm)){
                //state = State::Follower_State;
                votedFor = args.candidateId;
                reply.voteGranted = true;
                recive_heartbeat = true;
                update();
        }
        return reply;
}

RequestVoteReply Raft_Server::PrepareVote(RequestVoteArgs args){
        // STAR_LOG_INFO(STAR_LOG_ROOT()) << "receive PrepareVote"
        //                                << "term is " << args.term <<" ,currentterm is " << currentTerm
        //                                << ",candidated is " << args.candidateId << " me is " << id
        //                                << ",lastLogIndex is " << args.lastLogIndex << " mylastlog index is " << log.back().index 
        //                                << ",LastLogTerm is " << args.lastLogTerm << "mylastlog term is " << log.back().term;
        
        RequestVoteReply reply;
        reply.term = currentTerm;
        reply.voteGranted = false;

        if(args.term > currentTerm || isUptoMe(args.lastLogIndex,args.lastLogTerm)) {
                reply.term = currentTerm;
                reply.voteGranted = true;
                return reply;
        }
        return reply;
}

void Raft_Server::boardcastPrepareVote(){
        if(state == State::Follower_State) {
                //STAR_LOG_INFO(STAR_LOG_ROOT()) << "boardcastPrepareVote!!!!";
                //STAR_LOG_INFO(STAR_LOG_ROOT()) << "currentTerm is "<< currentTerm;
                {
                        MutexType::Lock lock(m_mutex);
                        prepareCount={1};
                }

                RequestVoteArgs args;
                args.term = currentTerm+1;
                args.candidateId = id;
                args.lastLogIndex = log.back().index;
                args.lastLogTerm = log.back().term;

                for(size_t i=0;i<m_addrs.size();++i){
                        if((int)i == id)
                                continue;
                        go [this,i,args] {
                                if(this->state != State::Follower_State)
                                        return ;
                                if(!servers[i]->isConnected()){
                                        star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
                                        new_client->setTimeout(g_heartbeat_time->getValue());
                                        if(new_client->connect(star::Address::LookupAny(m_addrs[i]))){
                                                MutexType::Lock mutex(m_mutex);
                                                servers[i]=new_client;
                                        }else{
                                                return;
                                        }
                                }
                                rpc::Result<RequestVoteReply> res = servers[i]->call<RequestVoteReply>("PrepareVote",args);
                                MutexType::Lock lock(this->m_mutex);
                                if(res.getCode() == rpc::RpcState::RPC_SUCCESS){
                                        RequestVoteReply reply = res.getVal();
                                        if(reply.voteGranted)
                                                this->prepareCount++;
                                        if(this->prepareCount > ((int)this->m_addrs.size()/2)){
                                                this->winPreVote=true;
                                        }
                                }
                        };
                }
        }
        return ;
}

void Raft_Server::handleRequestVote(RequestVoteReply reply){
        // MutexType::Lock lock(m_mutex);
        //STAR_LOG_INFO(STAR_LOG_ROOT()) <<"reply term is " << reply.term
        //                               <<", voteGranted " << reply.voteGranted;
        if(reply.term < currentTerm)
                return ;
        if(reply.term > currentTerm){
                state = State::Follower_State;
                currentTerm = reply.term;
                votedFor = -1;
                update();
                return ;
        }

        if(state == State::Candidate_State && reply.voteGranted) {
                voteCount += 1;
                if(voteCount > (int)m_addrs.size()/2) {
                        state = State::Leader_State;
                        nextIndex = std::vector<int>(m_addrs.size(),log.back().index+1);
                        matchIndex = std::vector<int>(m_addrs.size());
                        boardcastHeartBeat();
                        update();
                }
        }

}
void Raft_Server::applyLog() {
        int baseIndex,lastapplied,commitindex;
        {
                MutexType::Lock lock(m_mutex);
                lastapplied = lastApplied;
                commitindex = commitIndex;
                baseIndex = log[0].index;
        }
        //m_mutex.lock();
        //int baseIndex = log[0].index;
        for(int i=lastapplied-baseIndex+1;i<=commitindex-baseIndex && i < (int)log.size();++i){
                m_chan << log[i];
        }
        {
                MutexType::Lock lock(m_mutex);
                lastApplied = commitindex;
        }
        //m_con.notifyAll();
}

InstallSnapshotReply Raft_Server::installsnapshot(InstallSnapshotArgs args){
        MutexType::Lock lock(m_mutex);
        InstallSnapshotReply reply;
        if (args.term < currentTerm) {
                reply.term = currentTerm;
                return reply;
        }

        if (args.term > currentTerm) {
                state = State::Follower_State;
                update();
                currentTerm = args.term;
                votedFor = -1;
        }

        update();

        reply.term = currentTerm;
        if(args.lastIncludeIndex > commitIndex) {
                lastApplied = args.lastIncludeIndex;
                commitIndex = args.lastIncludeIndex;
                if(ApplySnapshot != nullptr)
                        ApplySnapshot(args.data);
                else{
                        STAR_LOG_FATAL(STAR_LOG_ROOT()) << "Can not apply snapshot,ApplySnapshot is nullptr!";
                }
        }
        return reply;
}

bool Raft_Server::sendInstallSnapshot(int server,InstallSnapshotArgs args){
        MutexType::Lock lock(m_mutex);


        rpc::Result<InstallSnapshotReply> res = servers[server]->call<InstallSnapshotReply>("installsnapshot",args);

        if(res.getCode() != rpc::RpcState::RPC_SUCCESS || state != State::Leader_State || args.term != currentTerm)
                return false;
        
        InstallSnapshotReply reply = res.getVal();

        if(reply.term > currentTerm){
                currentTerm = reply.term;
                state = State::Follower_State;
                votedFor = -1;
                update();
                return true;
        }

        nextIndex[server] = args.lastIncludeIndex+1;
        matchIndex[server] = args.lastIncludeIndex;
        return true;
}

int Raft_Server::getState(){
        return state;
}

int Raft_Server::getCommitIndex(){
        return commitIndex;
}

void Raft_Server::persisent() {
        //static MutexType p_mutex;
        //int fd=-1;
        {
                MutexType::Lock p_lock(p_mutex);
                // if((int)log.size() <= maxLogSize || lastApplied-log[0].index <= maxLogSize) {
                //         return ;
                // }
                if(lastApplied - persisentLog < maxLogSize)
                        return ;
        }
        // int baseIndex = log[0].index;
        // std::vector<LogEntry> data;
        // int lastTerm=0,lastIndex=0;
        // {
        //         MutexType::Lock lock(m_mutex);
        //         LogEntry tmp = log.back();
        //         data = std::vector<LogEntry>(&log[0],&log[lastApplied-baseIndex]);
        //         log = std::vector<LogEntry>(&log[lastApplied-baseIndex],&log[log.size()]);
        //         lastTerm = log[lastApplied-baseIndex-1].term;
        //         lastIndex = log[lastApplied-baseIndex-1].index;
        //         if(log.size() == 0)
        //                 log.push_back(LogEntry{lastIndex,lastTerm,""});
        // }
        // std::string filename = "log-data-server["+std::to_string(id)+"]"+".log\0";
        // int fd = open(filename.data(),O_WRONLY | O_APPEND | O_CREAT,0777);
        // if(fd == -1) {
        //         STAR_LOG_ERROR(STAR_LOG_ROOT()) << "LogEntry persisent error : can not open output file < "<<filename<<" >";
        //         return ;
        // }
        // std::shared_ptr<int> a(&fd,[](int* fd){
        //         close(*fd);
        // });
        // char buf[100]={0};
        // for(auto it : data){
        //         sprintf(buf,"<%d><%d><%s>\n",it.index,it.term,it.value.c_str());
        //         //STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "persient buf is " << buf;
        //         size_t n=write(fd,buf,strlen(buf));
        //         (void)n;
        //         memset(buf,0,sizeof(buf));
        // }
        // fsync(fd);
        persisentlog();
        return ;
}


void Raft_Server::recover(){
        std::string filename = "log-data-server["+std::to_string(id)+"]"+".log\0";
        int fd = open(filename.data(),O_RDONLY);
        if(fd == -1){
                STAR_LOG_ERROR(STAR_LOG_ROOT()) << "Not Snapshot!";
                return ;
        }
        std::shared_ptr<int> a(&fd,[](int* fd){
                close(*fd);
        });
        char buf[65535];
        int size = read(fd,buf,sizeof(buf));
        if(size == -1) {
                STAR_LOG_ERROR(STAR_LOG_ROOT()) << "Snapshot read error! error< " << strerror(errno) <<" >";
                return ;
        }
        std::string str = buf;
        int pos = 0;
        while(pos < (int)str.size()) {
                try{
                        int n_pos = str.find("\n",pos);
                        std::string tmp = str.substr(pos,n_pos-pos);
                        //STAR_LOG_DEBUG(STAR_LOG_ROOT()) << tmp;
                        int pos1 = tmp.find(">");
                        std::string index = tmp.substr(1,pos1-1);
                        tmp = tmp.substr(pos1+2);
                        pos1 = tmp.find(">");
                        std::string term = tmp.substr(0,pos1);
                        std::string value = tmp.substr(pos1+2);
                        value = value.substr(0,value.size()-1);
                        log.push_back({atoi(index.c_str()),atoi(term.c_str()),value});
                        // }
                        pos = n_pos+1;
                        //lastApplied = atoi(index.c_str());
                        commitIndex = atoi(index.c_str());
                }catch(...){
                        //pos = n_pos+1;
                }
        }
        applyLog();
        LogEntry tmp = log.back();
        tmp.value="";
        log = std::vector<LogEntry>();
        log.push_back(tmp);
        return ;
}

LogEntry Raft_Server::start(std::string logval){
        LogEntry entry;
        {
                MutexType::Lock lock(m_mutex);
                if(state != State::Leader_State)
                        return {};
                entry.index =log.back().index+1;
                entry.term = currentTerm;
                entry.value = logval;
                log.push_back(entry);
        }
        go [this] {
                this->boardcastHeartBeat();
        };
        return entry;
}


void Raft_Server::persisentlog(){
        //int baseIndex = log[0].index;
        std::vector<LogEntry> data;
        // int lastTerm=0,lastIndex=0;
        {
                MutexType::Lock lock(m_mutex);

                data = std::vector<LogEntry>(&log[persisentLog],&log[lastApplied]);
                persisentLog = lastApplied;

                // LogEntry tmp = log.back();
                // if(log.size() != 1) {
                //         data = std::vector<LogEntry>(&log[0],&log[lastApplied-baseIndex]);
                //         log = std::vector<LogEntry>(&log[lastApplied-baseIndex],&log[log.size()]);
                //         //STAR_LOG_DEBUG(STAR_LOG_ROOT()) << lastApplied << " " << baseIndex << " " << data.size();
                //         // lastTerm = log[lastApplied-baseIndex-1].term;
                //         // lastIndex = log[lastApplied-baseIndex-1].index;
                //         if(log.size() == 0)
                //                 log.push_back(LogEntry{tmp.index+1,tmp.term,""});
                // }else if(log.size() == 1 && log[0].value != ""){
                //         data.push_back(tmp);
                //         log[0].index ++;
                //         log[0].value = "";
                // }
        }
        std::string filename = "log-data-server["+std::to_string(id)+"]"+".log\0";
        int fd = open(filename.data(),O_WRONLY | O_APPEND | O_CREAT,0777);
        if(fd == -1) {
                STAR_LOG_ERROR(STAR_LOG_ROOT()) << "LogEntry persisent error : can not open output file < "<<filename<<" >";
                return ;
        }
        std::shared_ptr<int> a(&fd,[](int* fd){
                close(*fd);
        });
        char buf[100]={0};
        for(auto it : data){
                sprintf(buf,"<%d><%d><%s>\n",it.index,it.term,it.value.c_str());
                size_t n=write(fd,buf,strlen(buf));
                (void)n;
                memset(buf,0,sizeof(buf));
        }
        fsync(fd);
}

int Raft_Server::GetLiveNode(){
        int ret = 0;
        for(size_t i=0;i<servers.size();++i){
                if(id == (int)i)
                        ret++;
                else if(servers[i]->isConnected())
                        ret++;
        }
        return ret;
}

}
