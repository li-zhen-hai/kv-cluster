#include "star/db/kv_server.h"

#include <functional>
#include <cstdlib>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace star{

static Logger::ptr g_logger = STAR_LOG_NAME("KV-Server");

static ConfigVar<std::string>::ptr snapshot_file_name = Config::Lookup<std::string>("snapshot_file_name","./kv-snapshot","snapshot_file_name");

kv_server::kv_server(std::string m_ip,std::string r_ip,size_t capacity,int maxlogsize)
    :m_chan(Channel<LogEntry>(capacity))
    ,r_server(nullptr)
    ,m_server(nullptr)
    ,is_stop(false)
    ,m_seqid({0}){

    m_server.reset(new star::rpc::RpcServer());
    star::Address::ptr address = star::Address::LookupAny(m_ip);
    m_server->setName("kv-server");

    auto fun1 = std::function<bool(std::string,std::string,uint64_t)>(std::bind(&kv_server::set,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    auto fun2 = std::function<std::string(std::string)>(std::bind(&kv_server::get,this,std::placeholders::_1));

    // auto fun3 = std::function<Data(std::string)>(std::bind(&kv_server::erase,this,std::placeholders::_1));
    // auto fun4 = std::function<std::vector<std::string>()>(std::bind(&kv_server::GetAllKey,this));

    auto fun5 = std::function<std::unordered_map<std::string,std::pair<std::string,uint64_t>>()>(std::bind(&kv_server::GetSnapshot,this));
    auto fun6 = std::function<bool(std::unordered_map<std::string,std::pair<std::string,uint64_t>>)>(std::bind(&kv_server::ApplySnapshot,this,std::placeholders::_1,false));
    auto fun7 = std::function<std::vector<std::string>(std::vector<std::string> keys,std::vector<std::string> vals,uint64_t version)>(std::bind(&kv_server::TCC_Try,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    auto fun8 = std::function<bool(std::vector<std::string> keys,uint64_t version)>(std::bind(&kv_server::TCC_Commit,this,std::placeholders::_1,std::placeholders::_2));
    auto fun9 = std::function<bool(std::vector<std::string> keys,std::vector<std::string> vals,uint64_t version)>(std::bind(&kv_server::TCC_Cancel,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    auto fun10 = std::function<bool()>(std::bind(&kv_server::clean,this));

    m_server->registerMethod("set",fun1);
    m_server->registerMethod("get",fun2);
    
    // m_server->registerMethod("erase",fun3);
    // m_server->registerMethod("GetAllKey",fun4);
    m_server->registerMethod("GetSnapshot",fun5);
    m_server->registerMethod("ApplySnapshot",fun6);
    m_server->registerMethod("TCC_Try",fun7);
    m_server->registerMethod("TCC_Commit",fun8);
    m_server->registerMethod("TCC_Cancel",fun9);
    m_server->registerMethod("clean",fun10);

    while(!m_server->bind(address)){
        sleep(1);
    }
    r_server.reset(new star::Raft_Server(r_ip,m_chan,fun5,fun6));
}

void kv_server::start(){
    r_server->start();
    m_server->start();

    while(!is_stop){
        LogEntry entry;
        m_chan >> entry;
        //STAR_LOG_DEBUG(g_logger) << "term : " << entry.term << ",index : " << entry.index <<",value : " << entry.value;
        // go [this,entry] {
        //     this->applylog(entry);
        // };
        if(entry.value=="")
                continue;
        try{
            int p1 = entry.value.find("[");
            int p2 = entry.value.find("]");
            int p3 = entry.value.find("]",p2+1);
            std::string id = entry.value.substr(0,p1);
            std::string key = entry.value.substr(p1+1,p2-p1-1);
            std::string value = entry.value.substr(p2+2,p3-p2-2);
            std::string mode = entry.value.substr(p3+1,3);
            std::string version = entry.value.substr(p3+4);
            if(mode == "Del")
                value =std::get<0>(m_values[key]);
            uint64_t s_id = (uint64_t)atoll(id.c_str());
            {
                std::unordered_map<uint64_t,std::shared_ptr<Channel<LogEntry>>>::iterator it2;
                {
                    MutexType::Lock lock(m_mutex);
                    it2 = channels.find(s_id);
                }
                //STAR_LOG_DEBUG(g_logger) << "id : " << id << ",term : " << entry.term <<",index : " << entry.index <<",value : " << entry.value;
                if(it2 != channels.end()){
                    // if(states[key] == KeyState::DELETE) {
                    //     (*it2->second) << LogEntry(-1,-1,"");
                    //     continue;
                    // }
                    go [this,it2,entry,mode,key,value,version] {
                        if(applylog(mode,key,value,(uint64_t)atoll(version.c_str())))
                            (*it2->second) << LogEntry{entry.index,entry.term,entry.value};
                        else
                            (*it2->second) << LogEntry{};
                    };
                }
            }

            //STAR_LOG_DEBUG(g_logger) << "key : " << key <<",value : " << value;
        }catch(...){
            continue;
        }
    }
}

// void kv_server::applylog(LogEntry entry){
//     if(entry.value=="")
//         return ;
//     try{
//         int p1 = entry.value.find("[");
//         int p2 = entry.value.find("]");
//         int p3 = entry.value.find("]",p2+1);
//         std::string id = entry.value.substr(0,p1);
//         std::string key = entry.value.substr(p1+1,p2-p1-1);
//         std::string value = entry.value.substr(p2+2,p3-p2-2);
//         std::string mode = entry.value.substr(p3+1,3);
//         std::string version = entry.value.substr(p3+4);
//         int s_id = atoi(id.c_str());
//         std::unordered_map<int,std::shared_ptr<Channel<LogEntry>>>::iterator it2;
//         {
//             MutexType::Lock lock(m_mutex);
//             it2 = channels.find(s_id);
//         }
//         //STAR_LOG_DEBUG(g_logger) << "id : " << id << ",term : " << entry.term <<",index : " << entry.index <<",value : " << entry.value;
//         if(it2 != channels.end()){
//         // if(states[key] == KeyState::DELETE) {
//         //     (*it2->second) << LogEntry(-1,-1,"");
//         //     continue;
//         // }
//             if(applylog(mode,key,value,(uint64_t)atoll(version.c_str())))
//                 (*it2->second) << LogEntry{entry.index,entry.term,entry.value};
//             else
//                 (*it2->second) << LogEntry{};
//         }

//         //STAR_LOG_DEBUG(g_logger) << "key : " << key <<",value : " << value;
//     }catch(...){
//         return ;
//     }
// }

bool kv_server::set(std::string key,std::string value,uint64_t version){
    // if(r_server->getState() != Raft_Server::State::Leader_State)
    //     throw std::logic_error("Not Leader");
    // //STAR_LOG_DEBUG(g_logger) << "key : " << key<<"; "<<"value : " << value;
    // std::shared_ptr<Channel<LogEntry>> chan(new Channel<LogEntry>(1));

    // {
    //     MutexType::Lock lock(m_mutex);
    //     if(m_states[key] != Key_State::UNLOCK)
    //         return false;
    // }

    // uint64_t id = m_seqid++;
    // {
    //     MutexType::Lock lock(m_mutex);
    //     channels.insert({id,chan});
    // }
    // LogEntry entry = r_server->start(std::to_string(id)+"["+key+"]["+value+"]"+"Set"+std::to_string(version));
    // LogEntry tmp;
    // bool isrun = true;
    // //Channel<LogEntry>* ctmp = &chan;
    // star::Timer::ptr timer = IOManager::GetThis()->addTimer(5000,[id,chan,&isrun](){
    //     if(isrun && chan.get()){
    //         //STAR_LOG_DEBUG(g_logger) << "<"<<id<<">"<<" TimeOut,has been closed!";
    //         chan->close();
    //     }
    // });
    // (*chan) >> tmp;
    // isrun = false;
    // //STAR_LOG_DEBUG(g_logger) <<"term : "<<tmp.term<<",index : " << tmp.index <<",value : " << tmp.value;
    // {
    //     MutexType::Lock lock(m_mutex);
    //     channels.erase(id);
    //     //timer->cancel();
    // }
    // if(tmp == entry)
    //     return true;
    // else    
    //     return false; 

    return appendlog(key,value,"Set",version);

}



std::string kv_server::get(std::string key){
    if(r_server->getState() != Raft_Server::State::Leader_State)
        throw std::logic_error("Not Leader");
    auto it = m_values.find(key);
    if(it != m_values.end() /** && states[key] != KeyState::DELETE **/)
        return std::get<0>(m_values[key]);
    else
        throw "Not Find";
}

// Data kv_server::erase(std::string key){
//     Data ret{"",""};
//     if(r_server->getState() != Raft_Server::State::Leader_State)
//         return ret;
//     //STAR_LOG_DEBUG(g_logger) << "key : " << key<<"; "<<"value : " << value;
//     std::shared_ptr<Channel<LogEntry>> chan(new Channel<LogEntry>(1));

//     uint64_t id =m_seqid++;
//     {
//         MutexType::Lock lock(m_mutex);
//         channels.insert({id,chan});
//     }
//     LogEntry entry = r_server->start(std::to_string(id)+"["+key+"]["+"]"+"Del");
//     LogEntry tmp;
//     bool isrun = true;
//     //Channel<LogEntry>* ctmp = &chan;
//     star::Timer::ptr timer = IOManager::GetThis()->addTimer(5000,[id,chan,&isrun](){
//         if(isrun && chan.get()){
//             //STAR_LOG_DEBUG(g_logger) << "<"<<id<<">"<<" TimeOut,has been closed!";
//             chan->close();
//         }
//     });
//     (*chan) >> tmp;
//     isrun = false;
//     //STAR_LOG_DEBUG(g_logger) <<"term : "<<tmp.term<<",index : " << tmp.index <<",value : " << tmp.value;
//     {
//         MutexType::Lock lock(m_mutex);
//         channels.erase(id);
//         //timer->cancel();
//     }
//     ret.value = tmp.value;
//     return ret;
// }

bool kv_server::applylog(std::string mode,std::string key,std::string value,uint64_t version){
    if(mode == "Set"){
        MutexType::Lock lock(m_mutex);
        auto it = m_values.find(key);
        if(it == m_values.end()) {
            std::shared_ptr<MutexType> new_lock(new MutexType());
            Value val = {value,new_lock,version};
            m_values[key] = val;
            //states[key] = KeyState::EXIST;
        }else{
            lock.unlock();
            MutexType::Lock lock2(*((std::get<1>(it->second)).get()));
            //it->second.first = value;
            if(std::get<2>(it->second) < version) {
                std::get<0>(it->second) = value;
                return true;
            }else if(std::get<2>(it->second) == version)
                return true;
            else
                return false;
        }
        return true;
    }else if(mode == "Del"){
        //states[key] = KeyState::DELETE;
        auto it = m_values.find(key);
        if(it == m_values.end())
            return false;
        MutexType::Lock lock(m_mutex);
        m_values.erase(it);
        return true;
    }else if(mode == "Try") {
        return Try(key,value,version);
    }else if(mode == "Com") {
        return Commit(key,version);
    }else if(mode == "Cel") {
        return Cancel(key,value,version);
    }
    return false;
}

// std::vector<std::string> kv_server::GetAllKey(){
//     if(r_server->getState() != Raft_Server::State::Leader_State)
//         return {};
//     std::unordered_map<std::string,Value> tmp;
//     {
//         MutexType::Lock lock(m_mutex);
//         tmp = m_values;
//     }
//     std::vector<std::string> ret;
//     for(auto val : tmp){
//         ret.push_back(val.first);
//     }
//     return ret;
// }

kv_server::KVSnapshot kv_server::GetSnapshot(){
    if(r_server->getState() != Raft_Server::State::Leader_State)
        throw std::logic_error("Not Leader");
    
    // int fd[2]={0};
    // if(pipe(fd) == -1) {
    //     STAR_LOG_FATAL(STAR_LOG_ROOT()) << "Pipe create error,can't create Snapshot!!!";
    //     std::logic_error("create error");
    // }
    // pid_t pid = fork();
    // if(pid == 0){
    //     KVSnapshot shot;
    //     for(auto q : m_values)
    //         shot[q.first] = {std::get<0>(q.second),std::get<2>(q.second)};
    //     star::rpc::Serializer s;
    //     s << shot;
    //     s.reset();
    //     close(fd[0]);
    //     int size = s.size();
    //     ssize_t ret = write(fd[1],&size,4);
    //     ret = write(fd[1],s.toString().c_str(),s.size());
    //     (void)ret;
    //     close(fd[1]);
    //     exit(-1);
    // }else{
    //     close(fd[1]);
    //     std::string tmp="";
    //     int size = 0;
    //     ssize_t n = read(fd[0],&size,4);
    //     tmp.resize(size);
    //     n = read(fd[0],tmp.data(),size);
    //     (void)n;
    //     star::rpc::Serializer s(tmp);
    //     KVSnapshot shot;
    //     s.reset();
    //     s >> shot;
    //     // while(waitpid(pid,nullptr,WNOHANG | WUNTRACED) == 0)
    //     //     sleep(1);
    //     wait(nullptr);
    //     close(fd[0]);
    //     return shot;
    // }

    KVSnapshot shot;
    std::unordered_map<std::string,Value> tmp;
    {
        MutexType::Lock lock(m_mutex);
        tmp = m_values;
    }

    for(auto q : tmp)
        shot[q.first] = {std::get<0>(q.second),std::get<2>(q.second)};

    return shot;
}

bool kv_server::ApplySnapshot(kv_server::KVSnapshot shot,bool f){
    if(r_server->getState() != Raft_Server::State::Leader_State)
        throw std::logic_error("Not Leader");
    {
        MutexType::Lock lock(m_mutex);
        for(auto q : shot) {
            std::shared_ptr<MutexType> mutex(new MutexType());
            auto it = m_values.find(q.first);
            if(it == m_values.end() || std::get<2>(it->second) < q.second.second)
                m_values[q.first] = Value{q.second.first , mutex, q.second.second};
        }
    }
    bool flag = f;
    while(!flag){
        //int fd = open("./kv-snapshot-tmp",O_WRONLY | O_CREAT,0777);
        int fd = open("./kv-snapshot-tmp",O_WRONLY | O_CREAT,0777);
        star::rpc::Serializer s;
        s << shot;
        s.reset();
        ssize_t ret = write(fd,s.toString().c_str(),s.size());
        (void)ret;
        fsync(fd);
        char command[100]={0};
        snprintf(command,sizeof(command),"ln -f ./kv-snapshot-tmp %s",snapshot_file_name->getValue().c_str());
        flag = (system(command)==0?true:false);
    }
    return true;
}

std::vector<std::string> kv_server::TCC_Try(std::vector<std::string> keys,std::vector<std::string> vals,uint64_t version){

    // if(r_server->getState() != Raft_Server::State::Leader_State)
    //     throw std::logic_error("Not Leader");

    // if(keys.size() != vals.size())
    //     throw std::logic_error("Length not equal!");

    // std::vector<std::string> ret;

    // // std::shared_ptr<Channel<LogEntry>> chan(new Channel<LogEntry>(1));

    // // std::string akey="",aval="";
    // // for(size_t i=0;i<keys.size();++i) {
    // //     akey += keys[i]+",";
    // //     aval += vals[i]+",";
    // // }
    // // uint64_t id = m_seqid++;
    // // {
    // //     MutexType::Lock lock(m_mutex);
    // //     channels[id]=chan;
    // // }

    // // r_server->start(std::to_string(id)+"["+akey+"]["+aval+"]"+"Try"+std::to_string(version));

    // // LogEntry tmp;
    // // bool isrun = true;
    // // star::Timer::ptr timer = IOManager::GetThis()->addTimer(5000,[id,chan,&isrun](){
    // //     if(isrun && chan.get()){
    // //         chan->close();
    // //     }
    // // });
    // // (*chan) >> tmp;
    // // isrun = false;

    // for(size_t i=0;i<keys.size();++i) {
    //     auto key = keys[i],val = vals[i];
    //     auto it = m_values.find(key);
    //     if(it != m_values.end()) {
    //         MutexType::Lock lock(*((std::get<1>(it->second)).get()));
    //         if(m_states[key] == Key_State::LOCK)
    //             throw std::logic_error("Can not Lock key : " + key);
    //         ret.push_back(std::get<0>(it->second));
    //     }else{
    //         MutexType::Lock lock(m_mutex);
    //         ret.push_back("");
    //     }
    //     m_states[key] = Key_State::LOCK;
    //     mutli_version[key] = Value{val,nullptr,version};
    // }

    if(r_server->getState() != Raft_Server::State::Leader_State)
        throw std::logic_error("Not Leader");

    std::vector<std::string> ret;

    std::string akey="",aval="";
    for(size_t i=0;i<keys.size();++i) {
        akey += keys[i]+",";
        aval += vals[i]+",";
    }
    if(appendlog(akey,aval,"Try",version))
        for(auto key : keys)
            ret.push_back(std::get<0>(m_values[key]));
    return ret;
}

bool kv_server::TCC_Commit(std::vector<std::string> keys,uint64_t version){

    // if(r_server->getState() != Raft_Server::State::Leader_State)
    //     throw std::logic_error("Not Leader");

    // for(auto key : keys) {
    //     if(std::get<2>(mutli_version[key]) != version)
    //         return false;
    //     auto it = m_values.find(key);
    //     if(it != m_values.end()) {
    //         it->second = Value(std::get<0>(mutli_version[key]),std::get<1>(it->second),version);
    //     }else{
    //         MutexType::Lock lock(m_mutex);
    //         Value val = {std::get<0>(mutli_version[key]),std::shared_ptr<MutexType>(new MutexType()),version};
    //         m_values[key] = val;
    //     }
    // }

    if(r_server->getState() != Raft_Server::State::Leader_State)
        throw std::logic_error("Not Leader");

    std::string akey="";
    for(size_t i=0;i<keys.size();++i)
        akey += keys[i]+",";
    if(appendlog(akey,"","Com",version)) {
        for(auto key : keys)
            m_states[key] = Key_State::UNLOCK;
        return true;
    }
    return false;
}

bool kv_server::TCC_Cancel(std::vector<std::string> keys,std::vector<std::string> vals,uint64_t version){


    // if(r_server->getState() != Raft_Server::State::Leader_State)
    //     throw std::logic_error("Not Leader");

    // for(size_t i=0;i<keys.size();++i){
    //     auto key = keys[i],val = vals[i];
    //     auto it = m_values.find(key);
    //     if(it != m_values.end() && std::get<2>(it->second) == version)
    //         std::get<0>(it->second) = val;
    // }

    if(r_server->getState() != Raft_Server::State::Leader_State)
        throw std::logic_error("Not Leader");

    std::string akey="",aval="";
    for(size_t i=0;i<keys.size();++i) {
        akey += keys[i]+",";
        aval += vals[i]+",";
    }
    if(appendlog(akey,aval,"Cel",version))
        for(auto key : keys) 
            m_states[key] = Key_State::UNLOCK;
    return true;
}

bool kv_server::Try(std::string akey,std::string aval,uint64_t version){
    std::vector<std::string> keys = split(akey,","),vals = split(aval,",");

    for(size_t i=0;i<keys.size();++i) {
        auto key = keys[i],val = vals[i];
        auto it = m_values.find(key);
        if(it != m_values.end()) {
            MutexType::Lock lock(*((std::get<1>(it->second)).get()));
            if(m_states[key] == Key_State::LOCK)
                return false;
            m_states[key] = Key_State::LOCK;
        }else{
            MutexType::Lock lock(m_mutex);
            m_states[key] = Key_State::LOCK;
        }
        mutli_version[key] = Value{val,nullptr,version};
    }

    return true;
}

bool kv_server::Commit(std::string akey,uint64_t version){
    std::vector<std::string> keys = split(akey,",");
    for(auto key : keys) {
        if(std::get<2>(mutli_version[key]) != version)
            return false;
        auto it = m_values.find(key);
        if(it != m_values.end()) {
            if(std::get<2>(m_values[key]) != std::get<2>(mutli_version[key])) {
                MutexType::Lock lock(*((std::get<1>(it->second)).get()));
                it->second = Value(std::get<0>(mutli_version[key]),std::get<1>(it->second),version);
            }
        }else{
            MutexType::Lock lock(m_mutex);
            Value val = {std::get<0>(mutli_version[key]),std::shared_ptr<MutexType>(new MutexType()),version};
            m_values[key] = val;
        }
    }

    for(auto key : keys) {
        m_states[key] = Key_State::UNLOCK;
    }
    return true;
}

bool kv_server::Cancel(std::string akey,std::string aval,uint64_t version){
    std::vector<std::string> keys = split(akey,","),vals = split(aval,",");
    for(size_t i=0;i<keys.size();++i){
        auto key = keys[i],val = vals[i];
        auto it = m_values.find(key);
        if(it != m_values.end() && std::get<2>(it->second) == version){
            MutexType::Lock lock(*((std::get<1>(it->second)).get()));
            std::get<0>(it->second) = val;
        }
    }
    for(auto key : keys)
        m_states[key] = Key_State::UNLOCK;
    return true;
}

bool kv_server::appendlog(std::string key,std::string val,std::string mode,uint64_t version){
    std::shared_ptr<Channel<LogEntry>> chan(new Channel<LogEntry>(1));

    {
        MutexType::Lock lock(m_mutex);
        if(m_states[key] != Key_State::UNLOCK)
            return false;
    }

    uint64_t id = m_seqid++;
    {
        MutexType::Lock lock(m_mutex);
        channels.insert({id,chan});
    }
    LogEntry entry = r_server->start(std::to_string(id)+"["+key+"]["+val+"]"+mode+std::to_string(version));
    LogEntry tmp;
    bool isrun = true;
    star::Timer::ptr timer = IOManager::GetThis()->addTimer(5000,[id,chan,&isrun](){
        if(isrun && chan.get()){
            chan->close();
        }
    });
    (*chan) >> tmp;
    isrun = false;
    {
        MutexType::Lock lock(m_mutex);
        channels.erase(id);
    }
    if(tmp == entry)
        return true;
    else    
        return false; 
}

bool kv_server::clean(){
    if(r_server->getState() != Raft_Server::State::Leader_State)
        throw std::logic_error("Not Leader");
    STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "clean was called!";
    for(auto q : m_states){
        MutexType::Lock lock(*((std::get<1>(m_values[q.first])).get()));
        q.second = Key_State::UNLOCK;
    }
    return true;
}

void kv_server::recover_from_snapshot() {
    int fd = open(snapshot_file_name->getValue().c_str(),O_WRONLY,0777);
    if(fd == -1){
        STAR_LOG_ERROR(g_logger) << "Snapshot was not find!";
        return ;
    }
    int len = lseek(fd,0,SEEK_END);
    char* addr = (char*)mmap(NULL,len,PROT_READ,MAP_PRIVATE,fd,0);
    star::rpc::Serializer ser(addr,len);
    kv_server::KVSnapshot shot;
    ser.reset();
    ser >> shot;
    while(!ApplySnapshot(shot,true)) {}
    return ;
}

bool kv_server::checkpoint() {
    KVSnapshot shot;
    std::unordered_map<std::string,Value> tmp;
    //int commitIndex,lastapplied;
    {
        MutexType::Lock lock(m_mutex);
        tmp = m_values;
    }

    for(auto q : tmp)
        shot[q.first] = {std::get<0>(q.second),std::get<2>(q.second)};

    return true;
}

std::vector<std::string> split(std::string str,std::string flag){
    std::vector<std::string> ret;
    size_t pos = 0;
    do{
        size_t next = str.find(flag,pos);
        std::string tmp = str.substr(pos,next-pos);
        pos = next+1;
        ret.push_back(tmp);
    }while(pos < str.size());
    return ret;
}


}