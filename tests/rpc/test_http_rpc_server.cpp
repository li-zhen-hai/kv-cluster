
#include "star/star.h"
#include "star/raft/raft_server.h"
#include "star/rpc/rpc_connection_pool.h"
#include <iostream>
static star::Logger::ptr g_logger = STAR_LOG_ROOT();

int main(){
    //STAR_LOG_DEBUG(g_logger) << STAR_LOG_NAME("system").get()->getLevel();
    //STAR_LOG_NAME("system").get()->setLevel(star::LogLevel::FATAL);
    //star::Config::LoadFromFile("../config/log.yaml");
    star::Config::LoadFromFile("../../config/service.yaml");
    star::Address::ptr pool_address = star::Address::LookupAny("127.0.0.1:8070");
    //star::rpc::RpcConnectionPool::ptr con = std::make_shared<star::rpc::RpcConnectionPool>(5);
    star::rpc::RpcConnectionPool::ptr con = std::make_shared<star::rpc::RpcConnectionPool>();
    con->connect(pool_address);

    star::IOManager::ptr ioManager(new star::IOManager{4});
    ioManager->submit([con]{
        star::Address::ptr address = star::Address::LookupAny("0.0.0.0:8081");
        star::http::HttpServer::ptr server(new star::http::HttpServer(true));

        server->getServletDispatch()->addServlet("/add",[con](star::http::HttpRequest::ptr request
                , star::http::HttpResponse::ptr response
                , star::http::HttpSession::ptr session) ->uint32_t {
            int a = request->getParamAs("a", 0ll);
            int b = request->getParamAs("b", 0ll);
            //con->call
            auto rt = con->call<int>("add",a,b);
            STAR_LOG_DEBUG(g_logger) << rt.toString();
            response->setBody(std::to_string(a) + " + " + std::to_string(b) + "=" + std::to_string(rt.getVal()));
            return 0;
        });

        while (!server->bind(address)){
            sleep(1);
        }

        server->start();
    });
}