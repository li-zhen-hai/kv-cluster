
#include "star/star.h"
#include "star/rpc/rpc.h"
#include "star/rpc/rpc_client.h"

#include <iostream>
#include <string>
static star::Logger::ptr g_logger = STAR_LOG_ROOT();

int main(){
    //STAR_LOG_DEBUG(g_logger) << STAR_LOG_NAME("system").get()->getLevel();
    STAR_LOG_NAME("system").get()->setLevel(star::LogLevel::FATAL);
    //g_logger->setLevel(star::LogLevel::FATAL);
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
            std::string key = request->getParamAs<std::string>("key");
            std::string value = request->getParamAs<std::string>("value");
            star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
            star::Address::ptr address = star::Address::LookupAny("127.0.0.1:9999");
            client->setTimeout(500);
            if(!client->connect(address)){
                response->setBody("!"+key);
                return 0;
            }
            auto res = client->call<bool>("set",key,value,false);
            if(res.getCode() == star::rpc::RPC_SUCCESS && res.getVal())
                response->setBody(key);
            else
                response->setBody("!"+key);
            return 0;
        });

        server->getServletDispatch()->addServlet("/getAllkv",[](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            std::string s= "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n\t<meta charset= \"utf-8\">\n\t<title>kv</title>\n\t<style>\n\t\ttable{\n\t\t\tborder: 1px solid black;\n\t\t\tmargin:auto;\n\t\t\twidth:500px;\n\t\t}\n\t\tth{\n\t\t\tborder:1px solid black;\n\t\t\theight:30px;\n\t\t}";
            s += "\n\t\ttd{\n\t\t\tborder: 1px solid black;\n\t\t\theight:20px;\n\t\t\ttext-align:center;\n\t\t}\n\t\tdiv{\n\t\t\ttext-align:left;\n\t\t\tmargin:50px;\n\t\t}";
            s += "\n\t</style>\n</head>\n<body>";
            s += "\n\t<div>\n\t\t<input type=\"text\" placeholder=\"请输入key\" id=\"key\">";
            s += "\n\t\t<input type=\"text\" placeholder=\"请输入value\" id=\"value\">";
            s += "\n\t\t<input type=\"button\" id = \"add\" value = \"添加\">\n\t</div>";
            s += "\n<table align=\"left\">\n\t<tr>\n\t\t<th>key</th>\n\t\t<th>value</th>\n\t</tr>";
            star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
            star::Address::ptr address = star::Address::LookupAny("127.0.0.1:9999");
            client->connect(address);
            auto res = client->call<std::map<std::string,std::string>>("GetAllKV");
            if(res.getCode() == star::rpc::RPC_SUCCESS){
                std::map<std::string,std::string> tmp = res.getVal();
                for(auto it:tmp){
                    s += "\n\t<tr>\n\t\t<th>"+it.first+"</th>\n\t\t<th>"+it.second+"</th>\n\t</tr>";
                }
            }
            // for(int i=0;i<100;++i){
            //     auto res = client->call<std::string>("get",std::to_string(i),false);
            //     if(res.getCode() == star::rpc::RPC_SUCCESS) {
            //         STAR_LOG_DEBUG(g_logger) << i << " value is " << res.getVal();
            //         s +="\n\t<tr>\n\t\t<th>"+std::to_string(i)+"</th>\n\t\t<th>"+res.getVal()+"</th>\n\t</tr>";
            //     }
            // }
            s+="\n</table>\n</body>";
            s+="\n<script>\n\tvar add = document.getElementById(\"add\");\n\tadd.onclick = function() {";
            s+="\n\t\tvar key = document.getElementById(\"key\").value;\n\t\tvar value = document.getElementById(\"value\").value;";
            s+="\n\t\tvar url = \"/add?&key=\"+key+\"&value=\"+value;\n\t\tvar xhr = new XMLHttpRequest();";
            s+="\n\t\txhr.open(\"POST\",url,true);";
            s+="\n\t\txhr.onreadystatechange = function() {\n\t\t\tif(xhr.readyState === 4 && xhr.status === 200){";
            s+="\n\t\t\t\tvar ret = xhr.responseText;\n\t\t\t\tif(ret === key){\n\t\t\t\t\talert(\"add sucess,please refresh the page.\");";
            s+="\n\t\t\t\t}else{\n\t\t\t\t\talert(\"add fail,please wait and try again.\");\n\t\t\t\t}\n\t\t\t}\n\t\t};";
            s+="\n\t\txhr.send();\n\t}\n</script>";
            s+="\n</html>";
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