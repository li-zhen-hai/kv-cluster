// #include "star/sync/distri_lock.h"
// #include "star/config.h"
// #include "star/log.h"
// #include "star/hook.h"
// #include "star/io_manager.h"

// #include <functional>

// static star::ConfigVar<std::string>::ptr g_zk_servers =
//         star::Config::Lookup<std::string>("zk_server","127.0.0.1:21811,127.0.0.1:21812,127.0.0.1:21813","server");     
    

// namespace star {

// distri_lock::distri_lock(const std::string& ips,star::ZKClient::watcher_callback func)
//     :zk_server(new ZKClient())
//     ,on_watcher(func)
//     ,hosts(ips){
//     hook_init();
//     if(hosts == ""){
//         hosts = g_zk_servers->getValue();
//         // hosts = "127.0.0.1:21811,127.0.0.1:21812,127.0.0.1:21813";
//     }
//     zk_server->init(hosts,3000,std::bind(&distri_lock::watcher,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4));
// }

// bool distri_lock::lock(const std::string& path){
//     std::string new_val="";
//     new_val.resize(256);
// run:
//     new_val = "";
//     int rt = zk_server->create(path,"",new_val,&ZOO_OPEN_ACL_UNSAFE,ZOO_EPHEMERAL);
//     //(void)rt;
//     STAR_LOG_DEBUG(STAR_LOG_ROOT()) << path <<" create return "<<rt <<" "<< new_val;
//     if(rt == ZOK){
//         return true;
//     }else{
//         auto it = m_locks.find(path);
//         if(it == m_locks.end()){
//             m_locks[path] = new CoCondVar();
//         }
//         // go [this,new_val,path] {
//         this->zk_server->exists(path,true);
//         // };
//         m_locks[path]->wait();
//         goto run;
//     }
//     return false;
// }

// bool distri_lock::unlock(const std::string& path){
//     STAR_LOG_DEBUG(STAR_LOG_ROOT()) <<"del";
//     int rt = zk_server->del(path);
//     if(rt == ZOK)
//         return true;
//     STAR_LOG_DEBUG(STAR_LOG_ROOT()) <<"del error!";
//     return false;
// }

// void distri_lock::watcher(int type, int stat, const std::string& path, star::ZKClient::ptr client){
//     if(stat == ZOO_CONNECTED_STATE){
//         // if(client == zk_server)
//         //     STAR_LOG_DEBUG(STAR_LOG_ROOT()) <<"is same!";
//         if(on_watcher) {
//                 on_watcher(type,stat,path,client);
//         }
//         auto it = m_locks.find(path);
//         if(it != m_locks.end()) {
//             it->second->notify();
//         }
//         if(stat == ZOO_CONNECTED_STATE)
//             STAR_LOG_INFO(STAR_LOG_ROOT()) << " type=" << type
//                                             << " stat=" << stat
//                                             << " path=" << path
//                                             << " client=" << client;
//         this->zk_server->exists(path,true);
//     }
//     return ;
// }

// distri_lock::~distri_lock(){
//     for(auto it:m_locks) {
//         it.second->notifyAll();
//     }
//     zk_server->close();
// }

// }