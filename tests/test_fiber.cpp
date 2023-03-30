

#include "star/fiber.h"
#include "star/log.h"

star::Logger::ptr g_logger = STAR_LOG_ROOT();
int count = 0;
void run_in_fiber(){
    STAR_LOG_INFO(g_logger)<<"run_in_fiber begin";
    star::Fiber::YieldToReady();
    STAR_LOG_INFO(g_logger)<<"run_in_fiber end";
}

void test1(){
    //star::Fiber::EnableFiber();
    STAR_LOG_INFO(g_logger)<<"begin";
    star::Fiber::ptr fiber(new star::Fiber(run_in_fiber));
    fiber->resume();
    STAR_LOG_INFO(g_logger)<<"after swap in";
    fiber->resume();
    STAR_LOG_INFO(g_logger)<<"end";
}
void test2(){
    star::Fiber::ptr fiber(new star::Fiber([](){
        while (1){
            count++;
            star::Fiber::YieldToReady();
        }
    }));
    while (1){
        fiber->resume();
        STAR_LOG_DEBUG(g_logger)<<count;
    }

}
void test_called_fiber(){

    STAR_LOG_DEBUG(g_logger) << "I was called";
    //star::Fiber::YieldToReady();
    STAR_LOG_WARN(g_logger) << "I was called";
}
void test_caller_fiber(){

    star::Fiber::ptr fiber(new star::Fiber(&test_called_fiber));
    //fiber->resume();
    fiber->resume();
    STAR_LOG_DEBUG(g_logger) << " Im calling";
}



void f3() {
    STAR_LOG_DEBUG(g_logger) << "Im fiber 3";
    star::Fiber::YieldToReady();
    STAR_LOG_DEBUG(g_logger) << "Im fiber 3 resume from fiber 2";
}

void f2() {
    STAR_LOG_DEBUG(g_logger) << "Im fiber 2";
    star::Fiber::ptr fiber(new star::Fiber(f3));
    fiber->resume();
    fiber->resume();
}

void f1() {
    STAR_LOG_DEBUG(g_logger) << "Im fiber 1";
    star::Fiber::ptr fiber(new star::Fiber(f2));
    fiber->resume();
}

void test() {
    int a[256*1024];
    a[256*1023] = 1000;
    STAR_LOG_DEBUG(STAR_LOG_ROOT()) << a[256*1023];
    //delete a;
    return ;
}

int main(int argc, char **argv){
    star::Fiber::EnableFiber();
    star::Fiber::ptr fiber(new star::Fiber(&test));
    fiber->resume();
    STAR_LOG_DEBUG(g_logger) << "Im main";
}