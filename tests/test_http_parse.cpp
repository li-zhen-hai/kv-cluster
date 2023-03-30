
#include "star/star.h"
using namespace star;
static star::Logger::ptr g_logger = STAR_LOG_ROOT();


void test_response_parse(){
    star::http::HttpResponseParser::ptr parser(new star::http::HttpResponseParser());
    char res[100] = "HTTP/1.0 404 NOT FOUND\r\nContent-Length: 23330\r\nDate: Thu,08 Mar 202107:17:51 GMT\r\n\r\n";
    parser->execute(res, strlen(res));
    STAR_LOG_INFO(g_logger) << "\n" << parser->getData()->toString();

}
const char test_request_data[] = "POST / HTTP/1.1\r\n"
                                 "Host: www.star.icu\r\n"
                                 "Content-Length: 10\r\n\r\n";

void test_request() {
    star::http::HttpRequestParser parser;
    std::string tmp = test_request_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    STAR_LOG_ERROR(g_logger) << "execute rt=" << s
                              << "has_error=" << parser.hasError()
                              << " is_finished=" << parser.isFinished()
                              << " total=" << tmp.size()
                              << " content_length=" << parser.getContentLength();
    tmp.resize(tmp.size() - s);
    STAR_LOG_INFO(g_logger) << parser.getData()->toString();
    STAR_LOG_INFO(g_logger) << tmp;
}

const char test_response_data[] = "HTTP/1.1 200 OK\r\n"
                                  "Date: Tue, 04 Jun 2050 15:43:56 GMT\r\n"
                                  "Server: Apache\r\n"
                                  "Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
                                  "ETag: \"51-47cf7e6ee8400\"\r\n"
                                  "Accept-Ranges: bytes\r\n"
                                  "Content-Length: 81\r\n"
                                  "Cache-Control: max-age=86400\r\n"
                                  "Expires: Wed, 05 Jun 2019 15:43:56 GMT\r\n"
                                  "Connection: Close\r\n"
                                  "Content-Type: text/html\r\n\r\n"
                                  "<html>\r\n"
                                  "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
                                  "</html>\r\n";

void test_response() {
    star::http::HttpResponseParser parser;
    std::string tmp = test_response_data;
    size_t s = parser.execute(&tmp[0], tmp.size());
    STAR_LOG_ERROR(g_logger) << "execute rt=" << s
                              << " has_error=" << parser.hasError()
                              << " is_finished=" << parser.isFinished()
                              << " total=" << tmp.size()
                              << " content_length=" << parser.getContentLength()
                              << " tmp[s]=" << tmp[s];

    tmp.resize(tmp.size() - s);

    STAR_LOG_INFO(g_logger) << parser.getData()->toString() << tmp;
    //STAR_LOG_INFO(g_logger) << tmp;
}
void test_vvip(){
    std::string str;
    str.resize(4096);
    star::http::HttpRequest request;
    request.setHeader("Host", "vvip.icu");
    auto sock = star::Socket::CreateTCPSocket();
    sock->connect(star::IPAddress::Create("vvip.icu",80));
    star::ByteArray buff;
    buff.writeStringWithoutLength(request.toString());
    //sock->send()
    buff.setPosition(0);
    //sock->send(buff);

    buff.clear();
    star::ByteArray buffer;
    //int n = sock->recv(buffer, 8000);
    int n = sock->recv(&str[0], 4096);
    STAR_LOG_INFO(g_logger) << n;
    STAR_LOG_INFO(g_logger) << str;
    n = sock->recv(&str[0], 4096);
    STAR_LOG_INFO(g_logger) << n;
    STAR_LOG_INFO(g_logger) << str;
    //str = buff.toString();
    //STAR_LOG_INFO(g_logger) << buffer.toString();
//    star::http::HttpResponseParser response;
//    response.execute(&str[0],4096);
//    STAR_LOG_INFO(g_logger) << response.getData()->toString() ;
//    STAR_LOG_INFO(g_logger) << response.getContentLength();
//    STAR_LOG_INFO(g_logger) << str.substr(0,response.getContentLength());
}
int main(){
    test_response();
    //IOManager ioManager{1};
    //ioManager.submit(&test_response);
    //ioManager.submit(&test_response_parse);
}