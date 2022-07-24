
#include "star/io_manager.h"
#include "star/log.h"
#include "star/sync.h"
#include "star/time_measure.h"
// #include "star/sync/distri_lock.h"

using namespace star;
// 测试协程同步原语
static Logger::ptr g_logger = STAR_LOG_ROOT();

using MutexType = CoMutex;
MutexType mutexType;
CoCondVar condVar;
volatile int n = 0;
void a() {
    TimeMeasure timeMesure;
    for (int i = 0; i < 100000; ++i) {
        MutexType::Lock lock(mutexType);
        ++n;
        //STAR_LOG_INFO(g_logger) << n;
    }
    STAR_LOG_INFO(g_logger) << n;
}

void b() {
    for (int i = 0; i < 100000; ++i) {
        MutexType::Lock lock(mutexType);
        ++n;
        //STAR_LOG_INFO(g_logger) << n;
    }
    STAR_LOG_INFO(g_logger) << n;
}

void test_mutex() {
    IOManager loop{};
    loop.submit(a);//->submit(b);
    loop.submit(b);
}

void cond_a() {
    MutexType::Lock lock(mutexType);
    STAR_LOG_INFO(g_logger) << "cond a wait";
    condVar.wait(lock);
    STAR_LOG_INFO(g_logger) << "cond a notify";
}
void cond_b() {
    MutexType::Lock lock(mutexType);
    STAR_LOG_INFO(g_logger) << "cond b wait";
    condVar.wait(lock);
    STAR_LOG_INFO(g_logger) << "cond b notify";
}
void cond_c() {
    sleep(2);
    STAR_LOG_INFO(g_logger) << "notify cone";
    condVar.notify();
    sleep(2);
    STAR_LOG_INFO(g_logger) << "notify cone";
    condVar.notify();
}
void test_condvar() {
    IOManager loop{};
    loop.submit(cond_a);//->submit(b);
    loop.submit(cond_b);
    loop.submit(cond_c);
}
CoSemaphore sem(5);
void sem_a() {
    for (int i = 0; i < 5; ++i) {
        sem.wait();
    }
    STAR_LOG_INFO(g_logger) << "sem_a sleep 2s";
    sleep(2);
    for (int i = 0; i < 5; ++i) {
        sem.notify();
    }
}
void sem_b() {
    STAR_LOG_INFO(g_logger) << "sem_b sleep 1s";
    sleep(1);
    for (int i = 0; i < 5; ++i) {
        sem.wait();
    }
    STAR_LOG_INFO(g_logger) << "sem_b notify";
    for (int i = 0; i < 5; ++i) {
        sem.notify();
    }
}
void test_sem() {
    IOManager loop{};
    loop.submit(sem_a);//->submit(b);
    loop.submit(sem_b);
}

void chan_a(Channel<int> chan) {
    for (int i = 0; i < 10; ++i) {
        chan << i;
        STAR_LOG_INFO(g_logger) << "provider " << i;
    }
    STAR_LOG_INFO(g_logger) << "close";
    chan.close();
}

void chan_b(Channel<int> chan) {
    int i=0;
    while (chan >> i) {
        STAR_LOG_INFO(g_logger) << "consumer " << i;
    }
    STAR_LOG_INFO(g_logger) << "close";
}
void test_channel() {
    IOManager loop{};
    Channel<int> chan(5);
    loop.submit(std::bind(chan_a, chan));
    loop.submit(std::bind(chan_b, chan));
}


// void test_distri_lock() {
//     distri_lock::ptr dis_mutex(new distri_lock());
//     STAR_LOG_INFO(g_logger) <<"test_distri_lock()";
//     dis_mutex->lock("/lock-test");
//     go [dis_mutex] {
//         dis_mutex->lock("/lock-test");
//         STAR_LOG_INFO(STAR_LOG_ROOT()) <<"lock go test 1";
//         dis_mutex->unlock("/lock-test");
//     };
//     go [dis_mutex] {
//         dis_mutex->lock("/lock-test");
//         STAR_LOG_INFO(STAR_LOG_ROOT()) <<"lock go test 2";
//         dis_mutex->unlock("/lock-test");
//     };
//     go [dis_mutex] {
//         dis_mutex->lock("/lock-test");
//         STAR_LOG_INFO(STAR_LOG_ROOT()) <<"lock go test 3";
//         dis_mutex->unlock("/lock-test");
//     };
//     go [dis_mutex] {
//         dis_mutex->lock("/lock-test");
//         STAR_LOG_INFO(STAR_LOG_ROOT()) <<"lock go test 4";
//         dis_mutex->unlock("/lock-test");
//     };
//     sleep(1);
//     STAR_LOG_INFO(STAR_LOG_ROOT()) <<"lock main test";
//     dis_mutex->unlock("/lock-test");
//     return ;
// }

int main() {
    //test_mutex();
    //test_condvar();
    //test_sem();
    test_channel();
    // go test_distri_lock;
    return 0;
}