
#include "star/http/http.h"
#include "star/log.h"
static star::Logger::ptr g_logger = STAR_LOG_ROOT();
void test_request(){
    star::http::HttpRequest h;
    //h.setPath("/index.h");
    h.setHeader("host","vvip.icu");
    h.setBody("Hello, World");

    STAR_LOG_INFO(g_logger) << '\n' << h.toString();
}
void test_response(){
    star::http::HttpResponse r;
    r.setHeader("host","vvip.icu");
    r.setBody("Hello, World");
    r.setStatus(404);
    STAR_LOG_INFO(g_logger) << '\n' << r.toString();
}
int main(){

    test_response();
}