
#include "star/star.h"
#include <memory>
#include <cstdlib>
int n=0;
star::RWMutex rwmutex;
star::Mutex mutex;
void f(){
    STAR_LOG_WARN(STAR_LOG_NAME("system"))
        <<"star::Thread::GetName()  "<<star::Thread::GetName()
        <<"star::Thread::GetThis()->getName()  "<<star::Thread::GetThis()->getName()
        <<"star::Thread::GetThis()->getId(); " <<star::Thread::GetThis()->getId()
        <<"star::GetThreadId() "<<star::GetThreadId();
    //star::ScopedLock<star::Mutex> a(mutex);
    //star::RWMutex::ReadLock a(rwmutex);
    //star::RWMutex::WriteLock a(rwmutex);
    for(int i=0;i<10000000;i++){
        //star::ScopedLock<star::Mutex> a(mutex);
        //star::RWMutex::ReadLock a(rwmutex);
        //star::RWMutex::WriteLock a(rwmutex);
        n++;
    }
}
void f2(){
    //star::Thread t;
    star::TimeMeasure time;
    star::Thread::ptr thread[10];

    for(int i=0;i<10;i++){
        thread[i]=std::make_shared<star::Thread>(std::to_string(i)+" t",&f);
    }

    for(int i=0;i<10;i++){
        thread[i]->join();
    }
    std::cout<<n;
}
void p1(){
    for (int i = 0; ; ++i) {
        STAR_LOG_WARN(STAR_LOG_ROOT())<<"++++++++++++++++++++++++++";
    }

};
void p2(){
    for (int i = 0; ; ++i) {
        STAR_LOG_ERROR(STAR_LOG_ROOT())<<"-----------------------------";
    }

};
int main(){
    star::Thread b("f1",&p1);
    star::Thread a("f2",&p2);
    a.join();
    b.join();
}