
#include "star/rpc/rpc_connection_pool.h"
#include "star/rpc/serializer.h"
#include "star/log.h"
#include "star/config.h"
static star::Logger::ptr g_logger = STAR_LOG_ROOT();

void test_discover() {
    star::Config::LoadFromFile("../../config/service.yaml");
    star::Address::ptr address = star::Address::LookupAny("127.0.0.1:8070");
    //star::rpc::RpcConnectionPool::ptr con = std::make_shared<star::rpc::RpcConnectionPool>(5);
    star::rpc::RpcConnectionPool::ptr con = std::make_shared<star::rpc::RpcConnectionPool>();
    con->connect(address);

//    auto aa = con->call<int>("add",1,1);
//    STAR_LOG_INFO(g_logger) << aa.toString();
//    aa = con->call<int>("add",2,2);
//    STAR_LOG_INFO(g_logger) << aa.toString();
//    aa = con->call<int>("add",3,3);
//    STAR_LOG_INFO(g_logger) << aa.toString();
    //std::future<star::rpc::Result<std::string>> b = con->async_call<std::string>("getStr");

    std::vector<std::string> vec{"a-","b-","c"};
    con->async_call<std::string>([](star::rpc::Result<std::string> str){
        STAR_LOG_INFO(g_logger) << str.toString();
    }, "CatString",vec);
    con->async_call<std::string>([](star::rpc::Result<std::string> str){
        STAR_LOG_INFO(g_logger) << str.toString();
    }, "CatString",vec);
    con->async_call<int>([](star::rpc::Result<int> res){
        STAR_LOG_INFO(g_logger) << res.toString();
    },"add",1,2);
    // while(true){
    //     int a,b;
    //     std::cin>>a>>b;
    //     auto rt = con->call<int>("add",a,b);
    //     STAR_LOG_DEBUG(g_logger) << rt.toString();
    // }
    sleep(20);
    auto rt = con->call<int>("add",1000,101);
    STAR_LOG_INFO(STAR_LOG_ROOT()) << "10s add value is "<<rt.getVal();
    //sleep(4);
//     int n=0;
//     while (n!=10000) {
//         STAR_LOG_DEBUG(g_logger) << n++;
//             con->async_call<int>([](star::rpc::Result<int> res){
//                 STAR_LOG_DEBUG(g_logger) << res.toString();
//             },"add",0,n);
// //        auto rt = con->call<int>("add",0,n);
// //        STAR_LOG_DEBUG(g_logger) << rt.toString();
//     }
    //sleep(5);
//    STAR_LOG_INFO(g_logger) << b.get().toString();
//    STAR_LOG_INFO(g_logger) << a.get().toString();
}

void test_subscribe() {
    star::Address::ptr address = star::Address::LookupAny("127.0.0.1:8070");
    //star::rpc::RpcConnectionPool::ptr con = std::make_shared<star::rpc::RpcConnectionPool>(5);
    star::rpc::RpcConnectionPool::ptr con = std::make_shared<star::rpc::RpcConnectionPool>(-1);
    con->connect(address);
    con->subscribe("data",[](star::rpc::Serializer s){
        std::vector<int> vec;
        s >> vec;
        std::string str;
        std::for_each(vec.begin(), vec.end(),[&str](int i) mutable { str += std::to_string(i);});
        LOG_DEBUG << "recv publish: " << str;
    });
    while (true) {
        sleep(5);
    }
}
int main() {
    //go test_subscribe;
    star::Config::LoadFromFile("../../config/service.yaml");
    go test_discover;
}