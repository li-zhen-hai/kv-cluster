

#include "star/rpc/rpc_service_registry.h"
#include "star/config.h"

void rpc_service_registry() {
    star::Address::ptr address = star::Address::LookupAny("127.0.0.1:8070");
    star::rpc::RpcServiceRegistry::ptr server(new star::rpc::RpcServiceRegistry());
    while (!server->bind(address)){
        sleep(1);
    }
    server->start();
}
void test_publish() {
    star::Address::ptr address = star::Address::LookupAny("127.0.0.1:8070");
    star::rpc::RpcServiceRegistry::ptr server(new star::rpc::RpcServiceRegistry());
    while (!server->bind(address)){
        sleep(1);
    }
    server->start();
    Go {
        int n = 0;
        std::vector<int> vec;
        while (true) {
            vec.push_back(n);
            sleep(3);
            server->publish("data", vec);
            ++n;
        }
    };
}
int main() {
    star::Config::LoadFromFile("../../config/service.yaml");
    go test_publish;
}