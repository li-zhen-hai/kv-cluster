#include "star/raft/raft_client.h"

static star::ConfigVar<std::vector<std::string>>::ptr g_raft_servers =
        star::Config::Lookup<std::vector<std::string>>("kv_server",{},"server");

namespace star{

Raft_Client::Raft_Client()
    :isclose(false){
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
    //go [this] {
        // for(size_t i=0;i<this->m_servers.size();++i) {
        //     star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
        //     new_client->connect(addrs[i]);
        //     m_servers[i]=new_client;
        // }
    //};
}

void Raft_Client::close() {
    isclose = true;
    for(size_t i=0;i<m_servers.size();++i)
        if(m_servers[i]->isConnected())
            m_servers[i]->close();
    return ;
}

bool Raft_Client::set(const std::string& key,const std::string& val){
    for(size_t i=0;i<m_servers.size() && !isclose;++i){
        if(!m_servers[i]->isConnected()){
            star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
            if(new_client->connect(addrs[i])){
                    m_servers[i]=new_client;
            }else{
                continue;
            }
        }
        auto res = m_servers[i]->call<bool>("set",key,val);
        if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS){
            if(res.getVal()==true)
                return true;
        }
    }
    return false;
}

std::string Raft_Client::get(const std::string& key){
    for(size_t i=0;i<m_servers.size();++i){
        if(!m_servers[i]->isConnected()){
            star::rpc::RpcClient::ptr new_client(new star::rpc::RpcClient());
            if(new_client->connect(addrs[i])){
                    m_servers[i]=new_client;
                }else{
                    continue;
                }
        }
        auto res = m_servers[i]->call<std::string>("get",key);
        if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS && res.getVal() != "Redirect!"){
            if(res.getVal() == "Not Found!")
                return "";
            return res.getVal();
        }
    }
    return "";
}

Raft_Client::~Raft_Client(){
    if(!isclose)
        close();
}

}