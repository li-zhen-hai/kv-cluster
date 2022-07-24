
#include "star/log.h"
#include "star/io_manager.h"
#include "star/net/socket.h"
#include "star/net/socket_stream.h"
#include "star/byte_array.h"
static star::Logger::ptr g_logger = STAR_LOG_ROOT();
void test_socket(){
    star::Address::ptr addr = star::IPAddress::Create("vvip.icu",80);
    if(addr){
        STAR_LOG_DEBUG(g_logger) << addr->toString();
    } else {
        return;
    }
    star::Socket::ptr sock = star::Socket::CreateTCP(addr);
    if(sock->connect(addr)){
        STAR_LOG_DEBUG(g_logger) << addr->toString() << " connected";
    } else {
        return;
    }
    char buff[]="GET / HTTP/1.0\r\n\r\n";
    sock->send(buff,sizeof buff);
    std::string str;
    str.resize(4096);
    sock->recv(&str[0],str.size());
    STAR_LOG_INFO(g_logger)<<str;
}
std::set<star::Socket::ptr> clients;
void sendAll(star::Socket::ptr sock, const char *msg, int len){
    char* buff = new char[4096];
    sprintf(buff,"%d says: ",sock->getSocket());
    strcat(buff,msg);
    puts(buff);
    for(auto i : clients){
        if(i != sock){
            i->send(buff, strlen(buff));
        }
    }
    delete[] buff;
}
void test_server(){
    star::Address::ptr address = star::IPv4Address::Create("127.0.0.1", 8080);
    star::Socket::ptr sock = star::Socket::CreateTCPSocket();
    sock->bind(address);
    sock->listen();
    while(1){
        star::Socket::ptr client = sock->accept();
        clients.insert(client);
        sendAll(client,"New User, welcome him\n", 21);
        star::IOManager::GetThis()->submit([client]{
            char *buff = new char[4096];

            while(1){
                int n = client->recv(buff,4096);
                if(n == 0){
                    STAR_LOG_INFO(g_logger) << "user quit";
                    break;
                }
                buff[n] = 0;
                //send(fd,buff,n,0);
                sendAll(client,buff,n);
            }
            sendAll(client,"user quit\n", 10);
            delete[] buff;
            clients.erase(client);
            client->close();
        });
    }
}
void test_byte_array() {
    star::Address::ptr addr = star::IPAddress::Create("localhost",8080);
    if(addr){
        STAR_LOG_DEBUG(g_logger) << addr->toString();
    } else {
        return;
    }
    star::Socket::ptr sock = star::Socket::CreateTCP(addr);
    if(sock->connect(addr)){
        STAR_LOG_DEBUG(g_logger) << addr->toString() << " connected";
    } else {
        return;
    }

    char buff[]="hello";
    star::ByteArray::ptr bt(new star::ByteArray());
    bt->write(buff, 6);
    bt->setPosition(0);
    //sock->send(buff,sizeof buff);

    star::SocketStream ss(sock);
    ss.writeFixSize(bt, 6);
    return;
    std::string str;
    str.resize(4096);
    sock->recv(&str[0],str.size());
    STAR_LOG_INFO(g_logger)<<str;
}
int main(){
    star::IOManager ioManager{};
    ioManager.submit(&test_byte_array);
    //test_socket();
}