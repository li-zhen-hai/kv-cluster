
#include "star/star.h"

static auto g_logger = STAR_LOG_ROOT();
auto a = new star::IOManager(4,"workIO");
void run() {
    auto addr = star::Address::LookupAny("0.0.0.0:8080");
    //auto addr = star::UnixAddress::ptr(new star::UnixAddress("/tmp/star/unix"));
    STAR_LOG_DEBUG(g_logger) << addr->toString();

    //auto b = star::IOManager::GetThis();
    star::TcpServer::ptr tcpServer(new star::TcpServer(a));

    while(!tcpServer->bind(addr)){
        sleep(3);
    }

    tcpServer->start();
    STAR_LOG_DEBUG(g_logger) << "start";
    //sleep(10000);
}
int main(){
    star::IOManager::ptr ioManager = star::IOManager::ptr(new star::IOManager(4,"mainIO"));
    ioManager->submit(&run);
    //sleep(1000);
}
