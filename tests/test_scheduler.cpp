
#include <unistd.h>
#include "star/star.h"
static star::Logger::ptr g_logger = STAR_LOG_ROOT();

void test_fiber() {
    static int s_count = 5;
    STAR_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    while(--s_count >= 0) {
        //star::Fiber::GetThis()->YieldToReady();

        //star::Fiber::GetThis()->YieldToReady();
        //star::Scheduler::GetThis()->submit(&test_fiber, star::GetThreadId());
        star::Fiber::YieldToReady();
        STAR_LOG_INFO(g_logger) << "resum  " << s_count;
    }
    STAR_LOG_INFO(g_logger) << "test end" << s_count;
}

void test_fiber2(){
    while(1){
        STAR_LOG_INFO(g_logger) << "while " ;
        sleep(2);
        star::Fiber::YieldToReady();
    }
}

void test3(){
    STAR_LOG_INFO(g_logger) << "main";
    star::Scheduler sc(3,"test");
    sc.start();
    sleep(2);
    STAR_LOG_INFO(g_logger) << "schedule";
    sc.submit(&test_fiber);
    //sc.submit(star::Fiber::ptr (new star::Fiber(&test_fiber)));
    //sc.submit(&test_fiber2);
    //sleep(8);
    //sc.stop();
    while(1);
    STAR_LOG_INFO(g_logger) << "over";
}

int main(int argc, char** argv) {
    STAR_LOG_DEBUG(g_logger) << "main";
    star::Scheduler sc(3,"test");
    sc.start();
    sc.submit([]{
        STAR_LOG_INFO(g_logger) << "hello world";
    });
    return 0;
}