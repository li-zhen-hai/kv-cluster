#include "star/db/shared_kv.h"
#include "star/time_measure.h"

static star::ConfigVar<std::vector<std::string>>::ptr kv_servers =
        star::Config::Lookup<std::vector<std::string>>("kv_server",{},"server");

void test_shared_kv() {
    star::Config::LoadFromFile("../../config/raft_server.yaml");
    star::Config::LoadFromFile("../../config/service.yaml");
    star::shared_kv server;
    server.addserver(kv_servers->getValue());
    // std::vector<std::string> addr = { "127.0.0.1:7001","127.0.0.1:7002","127.0.0.1:7003"};
    // server.addserver(addr);
    server.start("127.0.0.1:9999");
    while(true){
        sleep(10);
    }
    // while(1) {
        // star::TimeMeasure b_time;
        // STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "A (k-v) Set:";
        // for(int i=0;i<100;++i){
        //     STAR_LOG_DEBUG(STAR_LOG_ROOT()) << server.set(std::to_string(i),std::to_string(i),true);
        // }

        // // STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "atomic (k-v)s Set:";
        // // std::vector<std::string> keys{"1","2","3","4","5"};
        // // std::vector<std::string> vals{"5","4","3","2","1"};

        // // server.atomic_set(keys,vals);

        // STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "k-vs Get:";
        // for(int i=0;i<100;++i){
        //     STAR_LOG_DEBUG(STAR_LOG_ROOT()) << server.get(std::to_string(i));
        // }
    //     sleep(1);
    // }
    return ;
}

int main() {
    go test_shared_kv;
    return 0;
}