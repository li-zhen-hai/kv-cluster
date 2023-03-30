
#include "star/star.h"
#include <iostream>
#include "sys/time.h"
static star::Logger::ptr g_logger = STAR_LOG_ROOT();
void test5(){
    star::IOManager ioManager{3};

    star::Timer::ptr timer1 = ioManager.addTimer(5000,[&timer1]{
        static int i =0;
        i++;
        if(i==3){
            //timer1->reset(2000, true);
            //timer1->refresh();
        }
        STAR_LOG_INFO(g_logger) << i <<" seconds 3";
    }, true);
    star::Timer::ptr timer = ioManager.addTimer(2000,[&timer,&timer1]{
        static int i =0;
        i++;
        if(i==3){
            //timer->reset(1000, true);
            //timer->cancel();
            //timer1->refresh();
        }
        STAR_LOG_INFO(g_logger) << i <<" seconds 2";
    }, true);
    ioManager.start();
    //sleep(100);

}
star::Timer::ptr s_timer;
void test1(){
    star::IOManager ioManager{};
    struct timeval t;
    gettimeofday(&t,NULL);
    STAR_LOG_INFO(g_logger) << "begin " <<t.tv_usec; 
    s_timer = ioManager.addTimer(2,[]{
        // static int i =0;
        // i++;
        // if(i%1000==0){
        //     //s_timer->reset(1,false);
        //     s_timer->cancel();
        //     //t->refresh();
        //     STAR_LOG_INFO(g_logger) << i <<" seconds";
        // }
        struct timeval t1;
        gettimeofday(&t1,NULL);
        STAR_LOG_INFO(g_logger) << "now time " <<t1.tv_usec; 
    }, false);
}
int main(){
    test1();
}