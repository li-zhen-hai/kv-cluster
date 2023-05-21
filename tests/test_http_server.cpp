
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
                     std::make_shared<star::http::FileServlet>("../../html"));
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

        server->getServletDispatch()->addServlet("/addGroup",[](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            STAR_LOG_INFO(g_logger) << "addGroup run!";
            std::string ips = request->getParamAs<std::string>("ips");
            star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
            star::Address::ptr address = star::Address::LookupAny("127.0.0.1:9999");
            client->setTimeout(500);
            if(!client->connect(address)){
                response->setBody("!"+ips);
                return 0;
            }
            std::vector<std::string> vec;
            int pos = 0;
            for(int i=0;i<(int)ips.size();++i){
                if(ips[i] == ';'){
                    vec.push_back(ips.substr(pos,i-pos));
                    pos = i+1;
                }
            }
            if(pos != (int)ips.size())
                vec.push_back(ips.substr(pos));
            // for(int i=0;i<(int)vec.size();++i){
            //     STAR_LOG_DEBUG(g_logger) << " " <<vec[i];
            // }
            auto res = client->call<bool>("AddGroup",vec);
            if(res.getCode() == star::rpc::RPC_SUCCESS && res.getVal())
                response->setBody(ips);
            else
                response->setBody("!"+ips);
            return 0;
        });

        server->getServletDispatch()->addServlet("/delGroup",[](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            STAR_LOG_INFO(g_logger) << "delGroup run!";
            int id = request->getParamAs<int>("id");
            // STAR_LOG_INFO(g_logger) << id;
            star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
            star::Address::ptr address = star::Address::LookupAny("127.0.0.1:9999");
            client->setTimeout(500);
            if(!client->connect(address)){
                response->setBody("!"+id);
                return 0;
            }
            auto res = client->call<bool>("DelGroup",id);
            if(res.getCode() == star::rpc::RPC_SUCCESS && res.getVal())
                response->setBody(std::to_string(id));
            else
                response->setBody("!"+std::to_string(id));
            return 0;
        });

        server->getServletDispatch()->addServlet("/addPart",[](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            STAR_LOG_INFO(g_logger) << "addPart run!";
            int id = request->getParamAs<int>("id");
            int pos = request->getParamAs<int>("pos");
            STAR_LOG_INFO(STAR_LOG_ROOT()) << id <<" " << pos;
            star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
            star::Address::ptr address = star::Address::LookupAny("127.0.0.1:9999");
            client->setTimeout(500);
            if(!client->connect(address)){
                response->setBody("!"+id);
                return 0;
            }
            auto res = client->call<bool>("AddPart",pos,id);
            if(res.getCode() == star::rpc::RPC_SUCCESS && res.getVal())
                response->setBody(std::to_string(id));
            else
                response->setBody("!"+std::to_string(id));
            return 0;
        });

        server->getServletDispatch()->addServlet("/delPart",[](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            STAR_LOG_INFO(g_logger) << "delPart run!";
            int id = request->getParamAs<int>("id");
            int pos = request->getParamAs<int>("pos");
            star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
            star::Address::ptr address = star::Address::LookupAny("127.0.0.1:9999");
            client->setTimeout(500);
            if(!client->connect(address)){
                response->setBody("!"+id);
                return 0;
            }
            auto res = client->call<bool>("DelPart",id,pos);
            if(res.getCode() == star::rpc::RPC_SUCCESS && res.getVal())
                response->setBody(std::to_string(id));
            else
                response->setBody("!"+std::to_string(id));
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
            s+="\n\t\t\t\t}else{\n\t\t\t\t\talert(\"add fail,please wait and try again\");\n\t\t\t\t}\n\t\t\t\twindow.location.reload();\n\t\t\t}\n\t\t};";
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

        server->getServletDispatch()->addServlet("/GetCluster",[](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            STAR_LOG_INFO(g_logger) << "GetCluster run!";
            int id = request->getParamAs<int>("id");
            star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
            star::Address::ptr address = star::Address::LookupAny("127.0.0.1:9999");
            client->connect(address);
            auto res = client->call<std::map<int,std::vector<std::string>>>("GetCluster",id);
            std::map<int,std::vector<std::string>> tmp;
            if(res.getCode() == star::rpc::RPC_SUCCESS)
                tmp = res.getVal();
            star::Json json;
            std::vector<star::Json> data((int)tmp.size());
            for(int i=0;i<(int)tmp.size();++i){
                // STAR_LOG_INFO(STAR_LOG_ROOT()) << "GetCluster : id "<<i<<",state "<<tmp[i][0] << ",raft "<<tmp[i][1];
                data[i]["id"] = i;
                data[i]["state"] = tmp[i][0];
                data[i]["raft"] = tmp[i][1];
            }
            json["data"]=data;
            response->setJson(json);
            return 0;
        });

        server->getServletDispatch()->addServlet("/GetClusterHash",[](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            STAR_LOG_INFO(g_logger) << "GetClusterHash run!";
            int id = request->getParamAs<int>("id");
            star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
            star::Address::ptr address = star::Address::LookupAny("127.0.0.1:9999");
            client->connect(address);
            auto res = client->call<std::vector<std::string>>("GetClusterHash",id);
            std::vector<std::string> tmp;
            if(res.getCode() == star::rpc::RPC_SUCCESS)
                tmp = res.getVal();
            star::Json json;
            std::vector<star::Json> data((int)tmp.size());
            for(int i=0;i<(int)tmp.size();++i){
                // STAR_LOG_INFO(STAR_LOG_ROOT()) << "GetCluster : id "<<i<<",state "<<tmp[i][0] << ",raft "<<tmp[i][1];
                STAR_LOG_INFO(STAR_LOG_ROOT()) << tmp[i];
                data[i]["id"] = i;
                data[i]["hash"] = tmp[i];
            }
            json["data"]=data;
            response->setJson(json);
            return 0;
        });

        // server->getServletDispatch()->addServlet("/cluster",[](star::http::HttpRequest::ptr request
        //         , star::http::HttpResponse::ptr response
        //         , star::http::HttpSession::ptr session) ->uint32_t {
        //     std::string id = request->getParamAs<std::string>("id");
        //     STAR_LOG_INFO(STAR_LOG_ROOT()) << "cluster id "<<id;
        //     return 0;
        // });

        server->getServletDispatch()->addServlet("/cluster",std::make_shared<star::http::FileServlet>("../../html"));

        server->getServletDispatch()->addServlet("/GetAllCluster",[](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            STAR_LOG_INFO(g_logger) << "GetAllCluster run!";
            star::rpc::RpcClient::ptr client(new star::rpc::RpcClient());
            star::Address::ptr address = star::Address::LookupAny("127.0.0.1:9999");
            client->connect(address);
            auto res = client->call<std::map<int,std::vector<std::string>>>("GetAllCluster");
            std::map<int,std::vector<std::string>> tmp;
            if(res.getCode() == star::rpc::RPC_SUCCESS)
                tmp = res.getVal();
            star::Json json;
            std::vector<star::Json> data;
            for(auto it : tmp){
                if(it.second.size() != 4)
                    continue;
                star::Json q;
                q["id"] = it.first;
                q["size"] = it.second[0];
                q["read"] = it.second[1];
                q["write"] = it.second[2];
                q["sum"] = it.second[3];
                data.push_back(q);
            }
            // for(int i=0;i<(int)tmp.size();++i){
            //     data[i]["id"] = tmp[i][1];
            //     data[i]["size"] = tmp[i][0];
            //     data[i]["read"] = tmp[i][2];
            //     data[i]["write"] = tmp[i][3];
            //     data[i]["sum"] = tmp[i][4];
            // }
            json["data"]=data;
            response->setJson(json);
            return 0;
        });

        // server->getServletDispatch()->addServlet("/index.html",std::make_shared<star::http::FileServlet>("../../html"));

        while (!server->bind(address)){
            sleep(1);
        }

        server->start();
    });
}