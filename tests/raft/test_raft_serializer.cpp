#include "star/rpc/serializer.h"
#include "star/raft/common.h"
#include "star/log.h"

class value : public Log_Value
{
public:
    value(int val):val(val){}

    ~value(){}

    std::string toString() override {
        return std::to_string(val);
    }

    void fromString(std::string data) override {
        val = atoi(data.c_str());
    }

    int getValue(){
        return val;
    }

private:
    int val;
};

void test_AppendLogEntryArgs(){
    AppendLogEntryArgs log;
    log.term = 1;
    log.leaderId = 1;
    log.prevLogIndex = 1;
    log.prevLogTerm = 1;
    value val(10);
    log.entries.push_back(LogEntry(1,1,"test"));
    log.leaderCommit = 1;
    star::rpc::Serializer s;
    s << log;
    s.reset();
    AppendLogEntryArgs t;
    s >> t;
    STAR_LOG_INFO(STAR_LOG_ROOT()) << "term is : " << t.term
                                   << ", leaderId is : " << t.leaderId
                                   << ", prevLogIndex is :" << t.prevLogIndex
                                   << ", prevLogTerm is " << t.prevLogTerm
                                   << ", entries size is " << t.entries.size()
                                   << ", leaderCommit is " << t.leaderCommit;
    for(size_t i=0;i<t.entries.size();++i)
        STAR_LOG_INFO(STAR_LOG_ROOT()) << t.entries[i].index <<" " << t.entries[i].term <<" "<<t.entries[i].value;
    return ;
}

void test_AppendLogEntryReply(){
    AppendLogEntryReply reply;
    reply.term = 1;
    reply.success = false;
    reply.nextTryLog = 1;
    star::rpc::Serializer s;
    s << reply;
    s.reset();
    AppendLogEntryReply tmp;
    s >> tmp;
    STAR_LOG_INFO(STAR_LOG_ROOT()) << "term is : " << tmp.term
                                   <<", success is " << tmp.success
                                   <<", nextTryLog is " << tmp.nextTryLog;
    return ;
}

void test_RequestVoteArgs(){
    RequestVoteArgs args;
    args.term = 1;
    args.candidateId = 1;
    args.lastLogIndex = 1;
    args.lastLogIndex =1;
    args.lastLogTerm = 1;

    star::rpc::Serializer s;
    s << args;
    s.reset();
    RequestVoteArgs tmp;
    s >> tmp;
    STAR_LOG_INFO(STAR_LOG_ROOT()) << "term is : " << tmp.term
                                   <<", candidateId is " << tmp.candidateId
                                   <<", lastLogIndex is " << tmp.lastLogIndex
                                   <<", lastLogTerm is " << tmp.lastLogTerm;
    
    return ;
}

void test_RequestVoteReply(){
    RequestVoteReply reply;
    reply.term = 1;
    reply.voteGranted = false;

    star::rpc::Serializer s;
    s << reply;
    s.reset();
    RequestVoteReply tmp;
    s >> tmp;
    STAR_LOG_INFO(STAR_LOG_ROOT()) << "term is " << tmp.term
                                   << "voteGranted is " << tmp.voteGranted;
    return ;
}

int main() {
    test_AppendLogEntryArgs();
    test_AppendLogEntryReply();
    test_RequestVoteArgs();
    test_RequestVoteReply();
    return 0;
}