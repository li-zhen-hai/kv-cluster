#ifndef STAR_COMMON_H
#define STAR_COMMON_H

#include "star/log.h"
#include "star/rpc/serializer.h"

#include <iostream>
#include <functional>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>

// std::placeholders::_1;
// std::placeholders::_2;
// std::placeholders::_3;

class Log_Value {
public:
    Log_Value(){
    }

    virtual ~Log_Value(){
    }

    virtual std::string toString() = 0;

    virtual void fromString(std::string data)=0;
};

struct LogEntry{
    int index;
    int term;
    //Log_Value* value;
    std::string value;

    LogEntry()
        :index(-1)
        ,term(-1)
        ,value(""){
    }

    LogEntry(int i,int t,std::string s)
        :index(i)
        ,term(t)
        ,value(s){
    }

    bool operator == (const LogEntry& ths){
        return ths.index == index && ths.term == term && ths.value == value;
    }

    std::ostream& operator << (std::ostream& os){
        os << "LogEntry index : " << index << ",term : " << term << ",value" << value;
        return os;
    }

    // ~LogEntry() {
    //     STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "index : " << index <<",term : " << term <<",value : " << value <<" was delete";
    // }
    
    // star::rpc::Serializer& operator >> (star::rpc::Serializer& in){
    //     in >> index;
    //     in >> term;
    //     in >> value;
    //     return in;
    // }
};


struct AppendLogEntryArgs{
    int term;
    int leaderId;
    int prevLogIndex;
    int prevLogTerm;
    std::vector<LogEntry> entries;
    int leaderCommit;
};

struct AppendLogEntryReply{
    int term;
    bool success;

    int nextTryLog;
};

struct RequestVoteArgs{
    int term;
    int candidateId;
    int lastLogIndex;
    int lastLogTerm;

};

struct RequestVoteReply{
    int term;
    bool voteGranted;
};

struct InstallSnapshotArgs{
    int term;
    int leaderId;
    int lastIncludeIndex;
    int lastIncludeTerm;
    std::map<std::string,std::string> data;
};

struct InstallSnapshotReply{
    int term;
};

struct DB_log{
    uint64_t id;
    std::string key;
    std::string value;
};

struct Data{
    std::string key;
    std::string value;
};

#endif