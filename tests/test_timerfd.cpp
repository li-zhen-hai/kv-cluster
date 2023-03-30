#include "star/star.h"
#include "star/time_measure.h"

#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

static star::Logger::ptr g_logger = STAR_LOG_NAME("timerfd");

void test_timerfd() {
    star::TimeMeasure t;
    struct timespec now;
    if(clock_gettime(CLOCK_MONOTONIC,&now) == -1){
        STAR_LOG_DEBUG(g_logger) << "Get now time error";
        return ;
    }
    struct itimerspec new_value = {};
    new_value.it_value.tv_sec  = 0;
    new_value.it_value.tv_nsec = 50000000; // 50ms

    new_value.it_interval.tv_sec  = 0;
    new_value.it_interval.tv_nsec = 50000000; // 50ms

    int fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if(fd == -1){
        STAR_LOG_DEBUG(g_logger) << "timerfd_create error!";
        return ;
    }
    if(timerfd_settime(fd,0,&new_value,NULL) == -1){
        STAR_LOG_DEBUG(g_logger) << "timerfd_settime:";
        perror("\terror:\n");
        return ;
    }
    uint64_t val;
    size_t n=0;
    do{ 
        n=read(fd,&val,sizeof(val));
    }while(n!=sizeof(uint64_t));
    STAR_LOG_DEBUG(g_logger) << val;
    return ;
}

int main(){
    go test_timerfd;
    return 0;
}