#include "star/db/shared_kv.h"
#include "star/time_measure.h"

static star::Logger::ptr g_logger = STAR_LOG_NAME("shared-kv-client");

void test_shard_kv_client() {
    star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
    star::Address::ptr address = star::Address::LookupAny("127.0.0.1:9999");
    client->connect(address);

    auto res = client->call<std::map<std::string,std::string>>("GetAllKV");
    if(res.getCode() == star::rpc::RPC_SUCCESS) {
        std::map<std::string,std::string> kv = res.getVal();
        for(auto it:kv){
            STAR_LOG_DEBUG(STAR_LOG_ROOT()) << it.first <<" : "<<it.second;
        }
    }else{
        STAR_LOG_DEBUG(g_logger) <<"GetAllKV fail.";
    }

    // for(int i=0;i<1000;++i){
    //     auto res = client->call<std::string>("get",std::to_string(i),false);
    //     if(res.getCode() == star::rpc::RPC_SUCCESS) {
    //         STAR_LOG_DEBUG(g_logger) << i << " value is " << res.getVal();
    //     }else{
    //         STAR_LOG_DEBUG(g_logger) << i << " value is not exist!";
    //     }
    // }
    return ;
}

int main(){
    go test_shard_kv_client;
    return 0;
}