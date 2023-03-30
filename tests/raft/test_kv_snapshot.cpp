#include "star/star.h"
#include "star/rpc/serializer.h"
#include "star/db/kv_server.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

static star::Logger::ptr g_logger = STAR_LOG_NAME("kv-snapshot");

void test_kv_snapshot(){
    int fd = open("/home/star/kv-cluster/raft_cluster/kv-snapshot-0",O_RDONLY,0777);
    if(fd == -1){
        STAR_LOG_ERROR(g_logger) << "Snapshot was not find!";
        return ;
    }
    int len = lseek(fd,0,SEEK_END);
    char* addr = (char*)mmap(NULL,len,PROT_READ,MAP_PRIVATE,fd,0);
    star::rpc::Serializer ser(addr,len);
    star::kv_server::KVSnapshot shot;
    ser.reset();
    ser >> shot;
    int count = 0;
    for(auto q : shot) {
        count++;
        STAR_LOG_DEBUG(g_logger) << q.first << ":" << q.second;
    }
    STAR_LOG_DEBUG(g_logger) << "-----" << count << "-----";
    return ;
}

int main(){
    go test_kv_snapshot;
    return 0;
}