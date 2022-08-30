#include <iostream>
#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "star/star.h"

void test_fiber_exit(){
    try{
        STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "I'am fiber,running!";
        exit(-1);
    }catch(...){

    }
}

void test_multithread_process() {
    int fd = open("/home/morningstar/test.txt",O_APPEND | O_WRONLY);
    if(fd == -1){
        STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "open error!";
        return ;
    }
    auto func = [fd](int k) {
        int ret = 0;
        char buf[100]={0};
        for(int i=0;i<10000;++i){
            snprintf(buf,sizeof(buf),"%d %d\n",i,k);
            //STAR_LOG_DEBUG(STAR_LOG_ROOT()) << buf;
            ret = write(fd,buf,sizeof(buf));
        }
        (void)ret;
        return 0;
    };
    go [func]{
        func(1);
    };
    go [func]{
        func(2);
    };
    go [func]{
        func(3);
    };
    go [func]{
        func(4);
    };
    pid_t pid = fork();
    if(pid == 0){
        close(fd);
        STAR_LOG_DEBUG(STAR_LOG_ROOT()) << "child process exit!";
        exit(1);
    }
    wait(nullptr);
    return ;
}

int main(){
    //go test_fiber_exit;
    go test_multithread_process;
    return 0;
}