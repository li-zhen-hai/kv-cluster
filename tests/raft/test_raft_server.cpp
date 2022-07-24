#include "star/raft/raft_server.h"
#include "star/config.h"
#include "star/sync.h"

// static star::ConfigVar<std::vector<std::string>>::ptr g_raft_servers =
//         star::Config::Lookup<std::vector<std::string>>("server",{},"server");

void test_raft_server(int argc,char** argv) {
    star::Config::LoadFromFile("../../config/raft_server.yaml");
    std::string ip="127.0.0.1:8000";
    if(argc>1)
        ip = argv[1];
    star::Raft_Server ser(ip,star::Channel<LogEntry>(50));
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