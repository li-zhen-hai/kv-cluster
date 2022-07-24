
#include <fstream>
#include "star/star.h"
static star::Logger::ptr g_logger = STAR_LOG_ROOT();
void run(){
    star::Address::ptr addr = star::IPAddress::Create("www.baidu.com", 80);
    if(!addr) {
        STAR_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    star::Socket::ptr sock = star::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt) {
        STAR_LOG_INFO(g_logger) << "connect " << addr->toString() << " failed";
        return;
    }

    star::http::HttpConnection::ptr conn(new star::http::HttpConnection(sock));
    star::http::HttpRequest::ptr req(new star::http::HttpRequest);
    req->setPath("/index");
    req->setHeader("host", "www.baidu.com");
    STAR_LOG_INFO(g_logger) << "req:" << std::endl
                             << req->toString();
    int n = conn->sendRequest(req);
    STAR_LOG_INFO(g_logger) << n;
    auto res = conn->recvResponse();
    if(!res) {
        STAR_LOG_INFO(g_logger) << "recvResponse " << addr->toString() << " failed";
        return;
    }
    STAR_LOG_INFO(g_logger) << res->toString();

}
void test_request() {
    star::http::HttpResult::ptr result = star::http::HttpConnection::DoGet("www.baidu.com");
                                //star::http::HttpConnection::DoGet("localhost:8080/index.html");
    STAR_LOG_INFO(g_logger) << result->toString();
}
void test_pool(){
    star::http::HttpConnectionPool::ptr pool(new star::http::HttpConnectionPool(
            "www.baidu.com", "", 80, false, 10, 1000 * 30, 5));

    star::IOManager::GetThis()->addTimer(1000, [pool](){
        auto r = pool->doGet("/", -1, {{"connection", "keep-alive"}});
        STAR_LOG_INFO(g_logger) << r->toString();

    }, true);
}
int main(){
    star::IOManager iom(2);
    iom.submit(test_request);
    //iom.submit(run);
    //iom.submit(test_pool);
    return 0;
}
