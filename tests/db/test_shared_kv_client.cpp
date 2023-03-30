#include "star/db/shared_kv.h"
#include "star/time_measure.h"

static star::Logger::ptr g_logger = STAR_LOG_NAME("shared-kv-client");

void test_shard_kv_client() {
    star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
    star::Address::ptr address = star::Address::LookupAny("127.0.0.1:9999");
    client->connect(address);
    for(int i=0;i<100;++i){
        auto res = client->call<bool>("set",std::to_string(i),std::to_string(i),false);
        if(res.getCode() == star::rpc::RPC_SUCCESS && res.getVal())
            STAR_LOG_DEBUG(g_logger) << i << "set successful!";
    }
    STAR_LOG_DEBUG(g_logger) << "k-v set end!";
    return ;
}

int main(){
    go test_shard_kv_client;
    return 0;
}