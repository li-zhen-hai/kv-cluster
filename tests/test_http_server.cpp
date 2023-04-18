
#include "star/star.h"
#include "star/rpc/rpc.h"
#include "star/rpc/rpc_client.h"

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

        server->getServletDispatch()->addServlet("/getAllkv",[](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            std::string s= "<!DOCTYPE html>\n<html lang=\"en\"\n<head>\n\t<meta charset= \"utf-8\">\n\t<title>kv<title>\n\t<style>\n\t\ttable{\n\t\t\tborder: 1px solid black;\n\t\t\tmargin:auto\n\t\t\twidth:500px;\n\t\t}\n\t\tth{\n\t\t\tborder:1px solid black;\n\t\t\theight:30px;\n\t\t}";
            s += "\n\t\ttd{\n\t\t\tborder: 1px solid black;\n\t\t\theight:20px;\n\t\t\ttext-align:center;\n\t\tdiv{\n\t\t\ttext-align:center\n\t\t\tmargin:50px\n\t\t}";
            s += "\n\t</style>\n</head>\n<body>\n<table align=\"left\">\n\t<tr>\n\t\t<th>key</th>\n\t\t<th>value</th>\n\t</tr>";
            star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
            star::Address::ptr address = star::Address::LookupAny("127.0.0.1:9999");
            client->connect(address);
            for(int i=0;i<1000;++i){
                auto res = client->call<std::string>("get",std::to_string(i),false);
                if(res.getCode() == star::rpc::RPC_SUCCESS) {
                    STAR_LOG_DEBUG(g_logger) << i << " value is " << res.getVal();
                    s +="\n\t<tr>\n\t\t<th>"+std::to_string(i)+"</th>\n\t\t<th>"+res.getVal()+"</th>\n\t</tr>";
                }
            }
            s+="\n</table>\n</body>\n</html>";
            response->setBody(s);
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