#include "star/db/redis.h"
#include "star/scheduler.h"
#include "star/io_manager.h"

#include <string>

void test_redis(){
    star::Redis re;
    if(!re.connect("127.0.0.1",6379)){
        STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "connect redis error!";
        return ;
    }
    std::string str="Hello redis";
    star::ReplyPtr reply = re.cmd("set ooo %b",str.c_str(),str.size());
    if(reply->type == REDIS_REPLY_ERROR)
        std::cout<<"set error\n";
    else
        std::cout<<"set successfully\n";
    reply = re.cmd("get ooo");
    if(reply->type != REDIS_REPLY_ERROR)
        std::cout << reply->str << std::endl;
    return ;
}

int main(){
    go test_redis;
    return 0;
}