

#include "star/rpc/rpc_client.h"
#include "star/io_manager.h"
#include "star/log.h"
static star::Logger::ptr g_logger = STAR_LOG_ROOT();

void test1() {
    star::Address::ptr address = star::Address::LookupAny("127.0.0.1:8070");
    star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());

    if (!client->connect(address)) {
        STAR_LOG_DEBUG(g_logger) << address->toString();
        return;
    }
    int n=0;
    //std::vector<std::future<star::rpc::Result<int>>> vec;
    while (n!=1000) {
        //STAR_LOG_DEBUG(g_logger) << n++;
        n++;
        client->async_call<void>([](star::rpc::Result<> res) {
            STAR_LOG_DEBUG(g_logger) << res.toString();
        }, "sleep");
    }
    auto rt = client->call<int>("add", 0, n);
    STAR_LOG_DEBUG(g_logger) << rt.toString();
    //sleep(3);
    //client->close();
    client->setTimeout(1000);
    auto sl = client->call<void>("sleep");
    STAR_LOG_DEBUG(g_logger) << "sleep 2s " << sl.toString();

}
void subscribe() {
    star::Address::ptr address = star::Address::LookupAny("127.0.0.1:8080");
    star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());

    if (!client->connect(address)) {
        STAR_LOG_DEBUG(g_logger) << address->toString();
        return;
    }
    client->subscribe("iloveyou",[](star::rpc::Serializer s){
        std::string str;
        s >> str;
        LOG_DEBUG << str;
    }) ;
    while(true)
    sleep(100);
}
int main() {
    go test1;
    //go subscribe;
}
