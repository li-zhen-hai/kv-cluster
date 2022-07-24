
#include "star/star.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include "stdio.h"
static star::Logger::ptr  g_logger = STAR_LOG_ROOT();


int setnonblocking( int fd )   //自定义函数，用于设置套接字为非阻塞套接字
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );        //设置为非阻塞套接字
    return old_option;
}

void test_fiber(){
    STAR_LOG_DEBUG(g_logger) << "test_fiber start";
    star::Fiber::YieldToReady();
    sleep(3);
    STAR_LOG_DEBUG(g_logger) << "test_fiber end";
}

void test1(){
    star::IOManager ioManager(2);
    ioManager.submit(&test_fiber);
    ioManager.submit(std::make_shared<star::Fiber>(test_fiber));
}
void test2(){
    STAR_LOG_DEBUG(g_logger) << star::Fiber::GetThis()->getState();
    star::IOManager ioManager(2);
    //int sock = socket(AF_INET, SOCK_STREAM, 0);
    //setnonblocking(sock);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);
//    ioManager.addEvent(sock,star::IOManager::READ,[sock](){
//        while(1){
//            char buff[100];
//            memset(buff,0,100);
//            recv(sock,buff,100,0);
//            //puts(buff);
//            STAR_LOG_INFO(g_logger) << "Server: " <<buff;
//            star::Fiber::YieldToReady();
//        }
//    });
    star::IOManager::GetThis()->addEvent(STDIN_FILENO, star::IOManager::READ, [](){
        while (true){
            STAR_LOG_DEBUG(g_logger) << "connected r";
            char buff[100];
            STAR_LOG_INFO(g_logger) << "STDIN: " << fgets(buff,100,stdin);
            star::Fiber::YieldToReady();
            //break;

        }
    });
    //star::IOManager::GetThis()->cancelEvent(STDIN_FILENO,star::IOManager::READ);
    //int cfd = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    //STAR_LOG_INFO(g_logger)<<cfd;
    //setnonblocking(cfd);


}

void test3(){
    STAR_LOG_DEBUG(g_logger) << star::Fiber::GetThis()->getState();
    star::IOManager ioManager(3);
    star::IOManager::GetThis()->addEvent(STDIN_FILENO, star::IOManager::READ, [](){
        while (true){
            STAR_LOG_DEBUG(g_logger) << "connected r";
            char buff[100];
            STAR_LOG_INFO(g_logger) << fgets(buff,100,stdin);
            //star::Fiber::YieldToReady();
            //break;
            //star::IOManager::GetThis()->addEvent(STDIN_FILENO, star::IOManager::READ,)
        }
    });
}
void test_cb(){
    STAR_LOG_INFO(g_logger) << "test cb";
    char buff[100];
    STAR_LOG_INFO(g_logger) << fgets(buff,100,stdin);
    star::IOManager::GetThis()->addEvent(STDIN_FILENO, star::IOManager::READ, &test_cb);
}
void test4(){
    STAR_LOG_DEBUG(g_logger) << star::Fiber::GetThis()->getState();
    star::IOManager ioManager(2);
    //star::IOManager::GetThis()->addEvent(STDIN_FILENO, star::IOManager::READ, &test_cb);
}

int main(){
    //star::Fiber::EnableFiber();
    test1();
    //std::cout<<sizeof(D);

}