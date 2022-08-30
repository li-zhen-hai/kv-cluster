
#include "star/star.h"
#include <iostream>
static star::Logger::ptr g_logger = STAR_LOG_ROOT();

int main(){
    //STAR_LOG_DEBUG(g_logger) << STAR_LOG_NAME("system").get()->getLevel();
    STAR_LOG_NAME("system").get()->setLevel(star::LogLevel::FATAL);
    g_logger->setLevel(star::LogLevel::FATAL);
    //star::Config::LoadFromFile("../config/log.yaml");
    star::IOManager::ptr ioManager(new star::IOManager{8});
    ioManager->submit([]{
        star::Address::ptr address = star::Address::LookupAny("0.0.0.0:12345");
        star::http::HttpServer::ptr server(new star::http::HttpServer(true));
        server->getServletDispatch()->addGlobServlet("/*",
                     std::make_shared<star::http::FileServlet>("/"));
        server->getServletDispatch()->addServlet("/a",[](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            //std::string str = request->toString();
            response->setBody("hello world");
            //response->setBody(str + "<h1>hello world</h1>");
            //response->setContentType(star::http::HttpContentType::TEXT_HTML);
            return 0;
        });
        server->getServletDispatch()->addServlet("/add",[](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            int a = request->getParamAs("a", 0ll);
            int b = request->getParamAs("b", 0ll);
            response->setBody(std::to_string(a) + " + " + std::to_string(b) + "=" + std::to_string(a+b));
            return 0;
        });

        server->getServletDispatch()->addServlet("/json",[](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            star::Json json;
            switch (request->getMethod()) {
                case star::http::HttpMethod::GET:
                    json["bool"] = true;
                    json["number"] = 114514;
                    json["float"] = M_PI;
                    json["string"] = "abcdefg";
                    response->setJson(json);
                    break;
                case star::http::HttpMethod::POST:
                    json = request->getJson();
                    STAR_LOG_INFO(g_logger) << json.type_name() << "\n" << json;
                    response->setJson(json);
                    break;
                default:
                    break;
            }
            return 0;
        });

        while (!server->bind(address)){
            sleep(1);
        }

        server->start();
    });
}