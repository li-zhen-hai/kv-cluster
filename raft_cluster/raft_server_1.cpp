#include "star/raft/raft_server.h"
#include "star/config.h"
#include "star/db/kv_server.h"

static star::ConfigVar<std::vector<std::string>>::ptr kv_servers =
        star::Config::Lookup<std::vector<std::string>>("kv_server",{},"server");

static star::ConfigVar<std::vector<std::string>>::ptr raft_servers =
        star::Config::Lookup<std::vector<std::string>>("raft_server",{},"server");

void test_raft_server(int argc,char** argv) {
    // STAR_LOG_ROOT()->setLevel(star::LogLevel::FATAL);
    // STAR_LOG_NAME("system")->setLevel(star::LogLevel::FATAL);
    star::Config::LoadFromFile("../config/raft_server.yaml");
    // std::string ip="127.0.0.1:8001"; kv_servers->getValue()[0];
    // if(argc>1)
    //     ip = argv[1];
    star::kv_server ser(kv_servers->getValue()[0],raft_servers->getValue()[0],65535,50,true);
    // STAR_LOG_INFO(STAR_LOG_ROOT()) << g_raft_servers->getValue().size();
    ser.start();
    while(true){
        sleep(10);
    };
    return ;
}

int main(int argc,char** argv){
    go [argc,argv] {
        test_raft_server(argc,argv);
    };
    return 0;
}