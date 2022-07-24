#include "star/raft/raft_server.h"
#include "star/raft/raft_client.h"
#include "star/rpc/rpc_client.h"
#include "star/config.h"
#include "star/net/address.h"
#include "star/io_manager.h"
#include "star/log.h"
#include "star/raft/common.h"
#include "star/time_measure.h"
#include "star/db/kv_client.h"

#include <vector>
#include <string>
#include <atomic>

static star::ConfigVar<std::vector<std::string>>::ptr g_raft_servers =
        star::Config::Lookup<std::vector<std::string>>("kv_server",{},"server");

std::atomic<int> nums = 0;

void test_raft_client() {
    star::Config::LoadFromFile("../../config/raft_server.yaml");
    star::Config::LoadFromFile("../../config/service.yaml");
    
    for(int i=0;i<10;++i) {
        //sleep(i*100);
        for(size_t j=0;j<g_raft_servers->getValue().size();++j){
            std::string ip = g_raft_servers->getValue()[j];
            star::Address::ptr addr = star::Address::LookupAny(ip);
            star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
            if(!client->connect(addr)){
                continue;
            }
            auto res = client->call<bool>("put","The value is "+std::to_string(j));
            if(res.getCode() == star::rpc::RpcState::RPC_SUCCESS && res.getVal() == true){
                STAR_LOG_INFO(STAR_LOG_ROOT()) << "The value "<< i << " was successfully append!";
                break;
            }
        }
    }
    return ;
}

void test_raft_client_1() {
    star::kv_client client;
    client.start();
    star::TimeMeasure b_time;
    for(int i=0;i<250;++i){
        bool ret = client.set(std::to_string(i),std::to_string(i),0);
        if(ret == true) {
            STAR_LOG_DEBUG(STAR_LOG_ROOT()) <<  "<(key:"<<i<<",value:"<<i<<")>"<< " append successful";
            nums++;
        //usleep(10);
        }else{
            STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "Can not append "<<"<(key:"<<i<<",value:"<<i<<")>";
        }
    }
    //STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "All add "<<nums;
    return ;
}

int main() {
    star::TimeMeasure b_time;
    go test_raft_client_1;
    // go test_raft_client_1;
    // go test_raft_client_1;
    // go test_raft_client_1;
    
    // go test_raft_client_1;
    // go test_raft_client_1;
    // go test_raft_client_1;
    // go test_raft_client_1;
    return 0;
}