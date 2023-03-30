#include "star/db/kv_client.h"

#include <exception>
#include <stdexcept>

static star::ConfigVar<std::vector<std::string>>::ptr g_raft_servers =
        star::Config::Lookup<std::vector<std::string>>("kv_server",{},"server");

namespace star{

kv_client::kv_client()
    :isclose(false)
    ,calls({0}){
}

bool kv_client::start(){
    star::Config::LoadFromFile("../../config/raft_server.yaml");
    star::Config::LoadFromFile("../../config/service.yaml");
    for(size_t j=0;j<g_raft_servers->getValue().size();++j){
        std::string ip = g_raft_servers->getValue()[j];
        star::Address::ptr addr = star::Address::LookupAny(ip);
        addrs.push_back(addr);
        star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
        client->connect(addrs[j]);
        m_servers.push_back(client);
    }
    return true;
}

bool kv_client::start(std::vector<star::Address::ptr> s_addrs){
    for(size_t j=0;j<s_addrs.size();++j){
        //star::Address::ptr addr = ;
        addrs.push_back(s_addrs[j]);
        star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
        client->connect(addrs[j]);
        m_servers.push_back(client);
    }
    return true;
}

bool kv_client::start(std::vector<std::string> s_ips){
    for(size_t j=0;j<s_ips.size();++j){
        std::string ip = s_ips[j];
        star::Address::ptr addr = star::Address::LookupAny(ip);
        addrs.push_back(addr);
        star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
        client->connect(addrs[j]);
        m_servers.push_back(client);
    }
    return true;
}

void kv_client::close() {
    isclose = true;
    while(calls!=0)
        sleep(1);
    for(size_t i=0;i<m_servers.size();++i)
        if(m_servers[i]->isConnected())
            m_servers[i]->close();
    return ;
}

bool kv_client::set(const std::string& key,const std::string& val,uint64_t version){
    if(isclose)
        throw std::logic_error("server close!");
    calls++;
    std::shared_ptr<int> flag(nullptr,[this](int*){
        (this->calls)--;
    });
    for(size_t i=0;i<m_servers.size() && !isclose;++i){
        if(!m_servers[i]->isConnected()){
            star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
            if(new_client->connect(addrs[i])){
                    m_servers[i]=new_client;
            }else{
                continue;
            }
        }
        if(!still_alive()){
            STAR_LOG_FATAL(STAR_LOG_ROOT()) << "Raft-Server occur error,Please reboot Raft-Server!";
            throw std::logic_error("Raft-Server Error!");
        }
        auto res = m_servers[i]->call<bool>("set",key,val,version);
        if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS){
            return res.getVal();
        }
    }
    return false;
}

void kv_client::set_until_success(const std::string& key,const std::string& val,uint64_t version){
    while(!isclose){
        try{
            if(set(key,val,version))
                break;
        }catch(...){
            sleep(2);
        }
    }
    return ;
}

std::string kv_client::get(const std::string& key){
    if(isclose)
        throw std::logic_error("server close!");
    calls++;
    std::shared_ptr<int> flag(nullptr,[this](int*){
        (this->calls)--;
    });
    for(size_t i=0;i<m_servers.size();++i){
        if(!m_servers[i]->isConnected()){
            star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
            if(new_client->connect(addrs[i])){
                    m_servers[i]=new_client;
                }else{
                    continue;
                }
        }
        if(!still_alive()){
            STAR_LOG_FATAL(STAR_LOG_ROOT()) << "Raft-Server occur error,Please reboot Raft-Server!";
            throw std::logic_error("Raft-Server Error!");
        }
        auto res = m_servers[i]->call<std::string>("get",key);
        if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS){
            if(res.getVal() == "")
                return "";
            return res.getVal();
        }
    }
    return "";
}

// std::vector<std::string> kv_client::GetAllKey(){
//     if(isclose)
//         throw "server close";
//     calls++;
//     std::shared_ptr<int> flag(nullptr,[this](int*){
//         (this->calls)--;
//     });
//     //std::vector<std::string> ret;
//     for(size_t i=0;i<m_servers.size();++i){
//         if(!m_servers[i]->isConnected()){
//             star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
//             if(new_client->connect(addrs[i])){
//                     m_servers[i]=new_client;
//                 }else{
//                     continue;
//                 }
//         }
//         auto res = m_servers[i]->call<std::vector<std::string>>("GetAllKey");
//         if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS){
//             if(res.getVal().size() == 0)
//                 continue;
//             return res.getVal();
//         }
//     }
//     return {};
// }

// Data kv_client::erase(std::string key){
//     if(isclose)
//         throw "server close";
//     calls++;
//     std::shared_ptr<int> flag(nullptr,[this](int*){
//         (this->calls)--;
//     });
//     //std::vector<std::string> ret;
//     for(size_t i=0;i<m_servers.size();++i){
//         if(!m_servers[i]->isConnected()){
//             star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
//             if(new_client->connect(addrs[i])){
//                     m_servers[i]=new_client;
//                 }else{
//                     continue;
//                 }
//         }
//         auto res = m_servers[i]->call<Data>("erase");
//         if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS){
//             if(res.getVal().key == "" && res.getVal().value =="")
//                 continue;
//             return res.getVal();
//         }
//     }
//     return {};
// }

std::unordered_map<std::string,std::pair<std::string,uint64_t>> kv_client::GetSnapshot(){
    if(isclose)
        throw "server close";
    calls++;
    std::shared_ptr<int> flag(nullptr,[this](int*){
        (this->calls)--;
    });
    std::unordered_map<std::string,std::pair<std::string,uint64_t>> ret;
    for(size_t i=0;i<m_servers.size();++i){
        if(!m_servers[i]->isConnected()){
            star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
            if(new_client->connect(addrs[i])){
                    m_servers[i]=new_client;
                }else{
                    continue;
                }
        }
        auto res = m_servers[i]->call<std::unordered_map<std::string,std::pair<std::string,uint64_t>>>("GetSnapshot");
        if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS){
            return res.getVal();
        }
    }
    return ret;
}

bool kv_client::ApplySnapshot(std::unordered_map<std::string,std::pair<std::string,uint64_t>> shot){
    if(isclose)
        throw "server close";
    calls++;
    std::shared_ptr<int> flag(nullptr,[this](int*){
        (this->calls)--;
    });
    for(size_t i=0;i<m_servers.size();++i){
        if(!m_servers[i]->isConnected()){
            star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
            if(new_client->connect(addrs[i])){
                    m_servers[i]=new_client;
                }else{
                    continue;
                }
        }
        auto res = m_servers[i]->call<bool>("ApplySnapshot",shot);
        if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS){
            return res.getVal();
        }
    }
    return false;
}

std::vector<std::string> kv_client::TCC_Try(std::vector<std::string> keys,std::vector<std::string> vals,uint64_t version){
    if(isclose)
        throw std::logic_error("server close!");
    calls++;
    std::shared_ptr<int> flag(nullptr,[this](int*){
        (this->calls)--;
    });
    for(size_t i=0;i<m_servers.size() && !isclose;++i){
        if(!m_servers[i]->isConnected()){
            star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
            if(new_client->connect(addrs[i])){
                    m_servers[i]=new_client;
            }else{
                continue;
            }
        }
        if(!still_alive()){
            STAR_LOG_FATAL(STAR_LOG_ROOT()) << "Raft-Server occur error,Please reboot Raft-Server!";
            throw std::logic_error("Raft-Server Error!");
        }
        auto res = m_servers[i]->call<std::vector<std::string>>("TCC_Try",keys,vals,version);
        if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS){
            return res.getVal();
        }
    }
    return {};
}

bool kv_client::TCC_Commit(std::vector<std::string> keys,uint64_t version){
    if(isclose)
        throw std::logic_error("server close!");
    calls++;
    std::shared_ptr<int> flag(nullptr,[this](int*){
        (this->calls)--;
    });
    for(size_t i=0;i<m_servers.size() && !isclose;++i){
        if(!m_servers[i]->isConnected()){
            star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
            if(new_client->connect(addrs[i])){
                    m_servers[i]=new_client;
            }else{
                continue;
            }
        }
        if(!still_alive()){
            STAR_LOG_FATAL(STAR_LOG_ROOT()) << "Raft-Server occur error,Please reboot Raft-Server!";
            throw std::logic_error("Raft-Server Error!");
        }
        auto res = m_servers[i]->call<bool>("TCC_Commit",keys,version);
        if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS){
            return res.getVal();
        }
    }
    return false;
}

bool kv_client::TCC_Cancel(std::vector<std::string> keys,std::vector<std::string> vals,uint64_t version){
    if(isclose)
        throw std::logic_error("server close!");
    calls++;
    std::shared_ptr<int> flag(nullptr,[this](int*){
        (this->calls)--;
    });
    for(size_t i=0;i<m_servers.size() && !isclose;++i){
        if(!m_servers[i]->isConnected()){
            star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
            if(new_client->connect(addrs[i])){
                    m_servers[i]=new_client;
            }else{
                continue;
            }
        }
        if(!still_alive()){
            STAR_LOG_FATAL(STAR_LOG_ROOT()) << "Raft-Server occur error,Please reboot Raft-Server!";
            throw std::logic_error("Raft-Server Error!");
        }
        auto res = m_servers[i]->call<bool>("TCC_Cancel",keys,vals,version);
        if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS){
            return res.getVal();
        }
    }
    return false;
}

bool kv_client::clean(){
    if(isclose)
        throw std::logic_error("server close!");
    calls++;
    std::shared_ptr<int> flag(nullptr,[this](int*){
        (this->calls)--;
    });
    for(size_t i=0;i<m_servers.size() && !isclose;){
        if(!m_servers[i]->isConnected()){
            star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
            if(new_client->connect(addrs[i])){
                    m_servers[i]=new_client;
            }else{
                continue;
            }
        }
        if(!still_alive()){
            STAR_LOG_FATAL(STAR_LOG_ROOT()) << "Raft-Server occur error,Please reboot Raft-Server!";
            throw std::logic_error("Raft-Server Error!");
        }
        auto res = m_servers[i]->call<bool>("clean");
        //STAR_LOG_DEBUG(STAR_LOG_ROOT()) << i << "server " << res.getCode();
        if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS){
            i++;
        }
        STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "server [ "<<i<<" ] clean error!";
    }
    return true;
}

bool kv_client::still_alive(){
    size_t num=0;
    for(size_t i=0;i<m_servers.size();++i)
        if(m_servers[i]->isConnected())
            num+=1;
    return num > m_servers.size()/2;
}

kv_client::~kv_client(){
    if(!isclose){
        isclose = true;
        close();
    }
}

}