#include "star/star.h"
#include "star/rpc/serializer.h"
#include "star/db/kv_server.h"

static star::Logger::ptr g_logger = STAR_LOG_NAME("snapshot_test");

void test_snapshot(){
    star::kv_server::KVSnapshot shot;
    for(int i=0;i<5;++i)
        shot[std::to_string(i)] = {std::to_string(i),1};
    star::rpc::Serializer ser;
    ser<<shot;
    ser.reset();
    STAR_LOG_DEBUG(g_logger) << ser.size();
    star::kv_server::KVSnapshot tmp;
    ser>>tmp;
    for(auto q : tmp)
        STAR_LOG_DEBUG(g_logger) << q.first <<":"<<q.second;
    return ;
}

int main(){
    go test_snapshot;
    return 0;
}