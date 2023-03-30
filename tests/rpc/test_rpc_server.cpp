
#include "star/rpc/rpc_server.h"
#include "star/config.h"

int add(int a,int b){
    return a + b;
}
std::string getStr() {
    return  "hello world";
}
std::string CatString(std::vector<std::string> v){
    std::string res;
    for(auto& s:v){
        res+=s;
    }
    return res;
}
int main(int argc, char** argv) {
    star::Config::LoadFromFile("../../config/service.yaml");
    int port = 8080;
    if (argv[1]) {
        port = std::stoi(argv[1]);
    }

    star::Address::ptr address = star::IPv4Address::Create("127.0.0.1",port);
    star::Address::ptr registry = star::Address::LookupAny("127.0.0.1:8070");
    star::rpc::RpcServer::ptr server(new star::rpc::RpcServer());
    std::string str = "lambda";
    //star::Address::ptr address = star::Address::LookupAny("127.0.0.1:8081");
    server->registerMethod("add",add);
    server->registerMethod("getStr",getStr);
    server->registerMethod("CatString", CatString);
    server->registerMethod("sleep", []{
        sleep(2);
    });
    while (!server->bind(address)){
        sleep(1);
    }
    server->bindRegistry(registry);
    server->start();
    Go {
        LOG_DEBUG << "start publish";
        while (true) {
            server->publish("iloveyou","Yes, i love you too");
            sleep(1);
        }
    };
}