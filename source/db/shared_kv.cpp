#include "star/db/shared_kv.h"
#include "star/log.h"
#include "star/config.h"

#include <iostream>
#include <algorithm>

namespace star{

shared_kv::shared_kv(bool autoload,hash_function func)
    :is_stop(false)
    ,is_start(false)
    ,auto_reload(autoload)
    ,m_server(new rpc::RpcServer())
    ,m_hash(func)
    ,idx({0}){
}

void shared_kv::start(std::string ip){
    star::Address::ptr addr = star::Address::LookupAny(ip);
    start(addr);
}

void shared_kv::start(star::Address::ptr addr){
    is_start = true;
    if(m_sessions.size() == 0) {
        STAR_LOG_FATAL(STAR_LOG_ROOT()) << "Not Server,please first call addserver to add server!";
        // return ;
    }
    auto func1 = std::function<bool(std::string,std::string,bool)>(std::bind(&shared_kv::set,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    auto func2 = std::function<std::string(std::string)>(std::bind(&shared_kv::get,this,std::placeholders::_1));
    auto func3 = std::function<bool(std::vector<std::string>,std::vector<std::string>)>(std::bind(&shared_kv::atomic_set,this,std::placeholders::_1,std::placeholders::_2));
    auto func4 = std::function<std::map<std::string,std::string>()>(std::bind(&shared_kv::GetAllKV,this));
    auto func5 = std::function<std::map<int,std::vector<std::string>>()>(std::bind(&shared_kv::GetAllCluster,this));
    auto func6 = std::function<std::map<int,std::vector<std::string>>(int)>(std::bind(&shared_kv::GetCluster,this,std::placeholders::_1));
    auto func7 = std::function<int(std::vector<std::string>)>(std::bind(&shared_kv::addserver,this,std::placeholders::_1));
    auto func8 = std::function<bool(int,int)>(std::bind(&shared_kv::addpart,this,std::placeholders::_1,std::placeholders::_2));
    auto func9 = std::function<bool(int)>(std::bind(&shared_kv::DelGroup,this,std::placeholders::_1));
    auto func10 = std::function<bool(int,int)>(std::bind(&shared_kv::delpart,this,std::placeholders::_1,std::placeholders::_2));
    auto func11 = std::function<std::vector<std::string>(int)>(std::bind(&shared_kv::GetClusterHash,this,std::placeholders::_1));


    m_server->registerMethod("set",func1);
    m_server->registerMethod("get",func2);
    m_server->registerMethod("atomic_set",func3);
    m_server->registerMethod("GetAllKV",func4);
    m_server->registerMethod("GetAllCluster",func5);
    m_server->registerMethod("GetCluster",func6);
    m_server->registerMethod("AddGroup",func7);
    m_server->registerMethod("AddPart",func8);
    m_server->registerMethod("DelGroup",func9);
    m_server->registerMethod("DelPart",func10);
    m_server->registerMethod("GetClusterHash",func11);


    m_server->setName("SharedKv");
    while(!m_server->bind(addr)){
        sleep(1);
    }
    m_server->start();
    STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "Shared-kv begin!";
}

int shared_kv::addserver(std::vector<std::string> ips){
    std::string key = "";
    for(size_t i=0;i<ips.size();++i)
        key += ips[i];
    unsigned int hash = gethash(key);
    kv_client::ptr session(new kv_client());
    session->start(ips);

    // while(!session->clean())
    //     STAR_LOG_ERROR(STAR_LOG_ROOT()) << "session can not clean!";

    size_t pos = -1;
    {
        MutexType::Lock lock(m_mutex);
        pos = m_sessions.size();
        m_sessions[pos] = session;
        
        std::sort(points.begin(),points.end());
    }
    return addpart(hash,pos);
}

bool shared_kv::addpart(int id,int pos){
    partition p{(unsigned int)id,-1};
    if(points.size() == 0){
        {
            MutexType::Lock lock(m_mutex);
            p.pos = pos;
            points.push_back(p);
        }
        return true;
    }
    if(is_start) {
        STAR_LOG_INFO(STAR_LOG_ROOT()) << "addpart run!";
        STAR_LOG_INFO(STAR_LOG_ROOT()) << "id "<<id << ",pos "<<pos;
        kv_client::ptr next = GetClient(id);

        for(auto p : points) {
            STAR_LOG_INFO(STAR_LOG_ROOT()) << "p.pos "<<p.pos << ",p.hash "<<p.hash;  
        }

        auto shot = next->GetSnapshot();
        {
            MutexType::Lock lock(m_mutex);
            points.push_back(p);
            std::sort(points.begin(),points.end());
        }
        m_sessions[pos]->ApplySnapshot(shot);
        STAR_LOG_INFO(STAR_LOG_ROOT()) << "first apply end!";
        for(auto p : points) {
            STAR_LOG_INFO(STAR_LOG_ROOT()) << "p.pos "<<p.pos << ",p.hash "<<p.hash;  
        }
        sleep(3);
        shot = next->GetSnapshot();
        {
            MutexType::Lock lock(m_mutex);
            for(auto& p : points) {
                //STAR_LOG_INFO(STAR_LOG_ROOT()) << "p.hash "<<p.hash << ",id "<<id;
                if((int)p.hash == id){
                    p.pos = pos;
                    break;
                }
            }
            std::sort(points.begin(),points.end());
        }
        STAR_LOG_INFO(STAR_LOG_ROOT()) << "addpart end!";
        for(auto p : points) {
            STAR_LOG_INFO(STAR_LOG_ROOT()) << p.pos <<" " << p.hash;
        }
        m_sessions[pos]->ApplySnapshot(shot);
    }else{
        p.pos = pos;
        points.push_back(p);
    }
    return true;
}

bool shared_kv::delpart(int id,int pos){
    partition p;
    for(auto it : points){
        if(it.pos == id && (int)it.hash==pos)
            p = it;
    }
    if(p.pos != id || (int)p.hash != pos)
        return false;
    return delserver(p.hash);
}

bool shared_kv::DelGroup(int id){
    STAR_LOG_INFO(STAR_LOG_ROOT()) << "DelGroup run";
    std::vector<partition> re;
    for(auto it : points) {
        if(it.pos == id)
            re.push_back(it);
    }
    for(auto it: re){
        if(!delserver(it.hash))
            return false;
    }
    m_sessions.erase(id);
    STAR_LOG_INFO(STAR_LOG_ROOT()) << "DelGroup successful!";
    return true;
}

bool shared_kv::delserver(int pos){
    int next = -1;
    for(auto it : points){
        if((int)it.hash > pos) {
            MutexType::Lock lock(m_mutex);
            next = it.pos;
            it.pos = -1;
        }
    }
    if(next == -1 && points.size() == 1) {
        points.clear();
        return true;
    }
    if(next == -1)
        next = 0;
    int now = -1;
    for(auto it= points.begin();it!=points.end();it++){
        if((int)it->hash == pos){
            now = it->pos;
            {
                MutexType::Lock lock(m_mutex);
                points.erase(it);
            }
            break;
        }
    }
    if(now == -1)
        return false;
    auto snap = m_sessions[now]->GetSnapshot();
    m_sessions[next]->ApplySnapshot(snap);
    sleep(3);
    snap = m_sessions[now]->GetSnapshot();
    m_sessions[next]->ApplySnapshot(snap);
    {
        for(auto& it : points){
            if((int)it.hash > pos ) {
                MutexType::Lock lock(m_mutex);
                it.pos = next;
            }
        }
    }
    return true;
}

bool shared_kv::set(std::string key,std::string value,bool flag){
    kv_client::ptr session = GetClient(gethash(key));
    if(session == nullptr)
        return false;
    uint64_t version = idx++;
    STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "key : " << key<< ", value : " << value;
    if(flag){
        session->set_until_success(key,value,version);
        return true;
    }else
        return session->set(key,value,version);
}

std::string shared_kv::get(std::string key){
    kv_client::ptr session = GetClient(gethash(key));
    if(session == nullptr)
        return "";
    return session->get(key);
}

bool shared_kv::atomic_set(std::vector<std::string> keys,std::vector<std::string> vals){
    uint64_t version = idx++;
    std::unordered_map<size_t,std::pair<std::vector<std::string>,std::vector<std::string>>> part;
    for(size_t i=0;i<keys.size();++i){
        auto key = keys[i],val = vals[i];
        size_t pos = GetClientPos(gethash(key));
        part[pos].first.push_back(key);
        part[pos].second.push_back(val);
    }
    std::vector<size_t> clients;
    std::vector<std::vector<std::string>> old_vals;
    bool flag = true;
    for(auto p : part) {
        clients.push_back(p.first);
        try{
            old_vals.push_back(m_sessions[p.first]->TCC_Try(p.second.first,p.second.second,version));
            if(old_vals.back().size() == 0){
                flag = false;
                break;
            }
        }catch(std::exception& e) {
            flag = false;
            break;
        }
    }
    if(flag) {
        for(auto p : part) {
            try
            {
                while(!m_sessions[p.first]->TCC_Commit(p.second.first,version)){}
            }
            catch(const std::exception& e)
            {
                while(!m_sessions[p.first]->TCC_Commit(p.second.first,version)){}
            }
            
        }
    }else{
        for(size_t i=0;i<clients.size();++i) {
            try
            {
                while(!m_sessions[clients[i]]->TCC_Cancel(part[clients[i]].second,old_vals[i],version)){}
            }
            catch(const std::exception& e)
            {
                while(!m_sessions[clients[i]]->TCC_Cancel(part[clients[i]].second,old_vals[i],version)){}
            }
            
        }
    }
    return flag;
}

std::map<std::string,std::string> shared_kv::GetAllKV(){
    std::map<std::string,std::string> ret;
    for(int i=0;i<(int)m_sessions.size();++i){
        std::map<std::string,std::string> tmp;
        try{
            tmp = m_sessions[i]->GetAllKV();
        }catch(...){
            continue;
        }
        for(auto it:tmp)
            ret[it.first] = it.second;
    }
    return ret;
}

std::map<int,std::vector<std::string>> shared_kv::GetAllCluster(){
    std::map<int,std::vector<std::string>> ret;
    for(int i=0;i<(int)(m_sessions.size());++i){
        try{
            ret[i].push_back(std::to_string(m_sessions[i]->GetServerSize()));
            std::pair<uint64_t,uint64_t> tmp = m_sessions[i]->GetOps();
            STAR_LOG_INFO(STAR_LOG_ROOT()) << "ops read "<<tmp.first<<", write "<<tmp.second;
            ret[i].push_back(std::to_string(i));
            ret[i].push_back(std::to_string(tmp.first));
            ret[i].push_back(std::to_string(tmp.second));
            ret[i].push_back(std::to_string(m_sessions[i]->GetAllKV().size()));
        }catch(...){
            continue;
        }
    }
    return ret;
}

std::map<int,std::vector<std::string>> shared_kv::GetCluster(int id){
    //STAR_LOG_INFO(STAR_LOG_ROOT()) << "GetCluster run";
    std::map<int,std::vector<std::string>> ret;
    std::vector<std::pair<std::string,std::string>> tmp = m_sessions[id]->GetCluster();
    for(int i=0;i<(int)tmp.size();++i){
        //STAR_LOG_INFO(STAR_LOG_ROOT()) << tmp[i].first <<","<<tmp[i].second;
        ret[i].push_back(tmp[i].first);
        ret[i].push_back(tmp[i].second);
    }
    return ret;
}

std::vector<std::string> shared_kv::GetClusterHash(int id){
    std::vector<std::string> ret;
    for(auto p : points){
        if((int)p.pos == id){
            ret.push_back(std::to_string(p.hash));
        }
    }
    return ret;
}

void shared_kv::close(){
    is_stop = true;
    for(auto session : m_sessions)
        session.second->close();
}

unsigned int shared_kv::gethash(std::string key){
    if(m_hash)
        return m_hash(key);
    return murMurHash(key.c_str(),key.size());
}

// UNDO
bool shared_kv::reloadkey(int part,int client){
    return true;
}

unsigned int shared_kv::murMurHash(const void *key, int len)
{
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    const int seed = 97;
    unsigned int h = seed ^ len;
    // Mix 4 bytes at a time into the hash
    const unsigned char *data = (const unsigned char *)key;
    while(len >= 4)
    {
        unsigned int k = *(unsigned int *)data;
        k *= m; 
        k ^= k >> r; 
        k *= m; 
        h *= m; 
        h ^= k;
        data += 4;
        len -= 4;
    }
    switch(len)
    {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
        h *= m;
    };
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
}

kv_client::ptr shared_kv::GetClient(unsigned int hash_){
    for(auto p : points)
        if(p.hash > hash_){
            if(p.pos == -1)
                return nullptr;
            return m_sessions[p.pos];
        }
    return m_sessions[points[0].pos];
}

size_t shared_kv::GetClientPos(unsigned int hash_){
    for(auto p : points)
        if(p.hash > hash_)
            return p.pos;
    return points[0].pos;
}

bool shared_kv::partition::operator < (const partition& thr) const{
    return hash < thr.hash;
}

shared_kv::~shared_kv(){
    if(!is_stop) {
        is_stop = true;
        close();
    }
    //m_server->stop();
    STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "Shared-kv closed!";
}

}
