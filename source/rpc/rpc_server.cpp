#include "star/config.h"
#include "star/rpc/rpc_server.h"

namespace star::rpc {
static Logger::ptr g_logger = STAR_LOG_NAME("system");

static ConfigVar<uint64_t>::ptr ser_heartbeat_timeout =
        Config::Lookup<uint64_t>("ser_heartbeat_timeout",30'000,
                               "heartbeat timeout (ms)");

static ConfigVar<uint64_t>::ptr ser_client_alivetime =
        Config::Lookup<uint64_t>("ser_client_alivetime",40'000,
                                 "heartbeat timeout (ms)");

static uint64_t s_heartbeat_timeout = 0;

struct _RpcServerIniter{
    _RpcServerIniter(){
        //star::Config::LoadFromFile("../../config/service.yaml");
        s_heartbeat_timeout = ser_heartbeat_timeout->getValue();
        ser_heartbeat_timeout->addListener([](const uint64_t& old_val, const uint64_t& new_val){
            STAR_LOG_INFO(g_logger) << "rpc server heartbeat timeout changed from "
                                    << old_val << " to " << new_val;
            s_heartbeat_timeout = new_val;
        });
        ser_client_alivetime->addListener([](const uint64_t& old_val,const uint64_t& new_val){
            STAR_LOG_INFO(g_logger) << "rpc server client alivetime changed from "
                                    << old_val << " to " << new_val;
        });
    }
};

static _RpcServerIniter s_initer;

RpcServer::RpcServer(IOManager *worker, IOManager *accept_worker)
        : TcpServer(worker, accept_worker)
        , m_AliveTime(ser_client_alivetime->getValue()){

}

RpcServer::~RpcServer() {
    {
        MutexType::Lock lock(m_sub_mtx);
        m_stop_clean = true;
    }
    bool join = false;
    m_clean_chan >> join;
}

bool RpcServer::bind(Address::ptr address) {
    m_port = std::dynamic_pointer_cast<IPv4Address>(address)->getPort();
    return TcpServer::bind(address);
}

void RpcServer::setName(const std::string &name) {
    TcpServer::setName(name);
}

bool RpcServer::bindRegistry(Address::ptr address) {
    Socket::ptr sock = Socket::CreateTCP(address);

    if (!sock) {
        return false;
    }
    if (!sock->connect(address)) {
        STAR_LOG_WARN(g_logger) << "can't connect to registry";
        m_registry = nullptr;
        return false;
    }
    m_registry = std::make_shared<RpcSession>(sock);

    Serializer s;
    s << m_port;
    s.reset();

    // 向服务中心声明为provider，注册服务端口
    Protocol::ptr proto = Protocol::Create(Protocol::MsgType::RPC_PROVIDER, s.toString());
    m_registry->sendProtocol(proto);
    return true;
}

bool RpcServer::start() {
    if (m_registry) {
        for(auto& item: m_handlers) {
            registerService(item.first);
        }
        auto server = std::dynamic_pointer_cast<RpcServer>(shared_from_this());
        // 服务中心心跳定时器 30s
        m_registry->getSocket()->setRecvTimeout(ser_heartbeat_timeout->getValue());
        m_heartTimer = m_worker->addTimer(ser_heartbeat_timeout->getValue(), [server]{
            STAR_LOG_DEBUG(g_logger) << "heart beat";
            Serializer s;
            s << (IOManager::GetThis()->GetPendingEventCount());
            s.reset();
            Protocol::ptr proto = Protocol::Create(Protocol::MsgType::HEARTBEAT_PACKET, s.toString());
            server->m_registry->sendProtocol(proto);
            Protocol::ptr response = server->m_registry->recvProtocol();

            if (!response) {
                STAR_LOG_WARN(g_logger) << "Registry close";
                //server->stop();
                //放弃服务中心，独自提供服务
                server->m_heartTimer->cancel();
            }
        }, true);
    }

    // 开启协程定时清理订阅列表
    Go {
        while (!m_stop_clean) {
            sleep(5);
            MutexType::Lock lock(m_sub_mtx);
            for (auto it = m_subscribes.cbegin(); it != m_subscribes.cend();) {
                auto conn = it->second.lock();
                if (conn == nullptr || !conn->isConnected()) {
                    it = m_subscribes.erase(it);
                } else {
                    ++it;
                }
            }
        }
        m_clean_chan << true;
    };

    return TcpServer::start();
}

void RpcServer::handleClient(Socket::ptr client) {
    STAR_LOG_DEBUG(g_logger) << "handleClient: " << client->toString();
    RpcSession::ptr session = std::make_shared<RpcSession>(client);

    Timer::ptr heartTimer;
    // 开启心跳定时器
    update(heartTimer, client);
    while (session->isConnected()) {
        Protocol::ptr request = session->recvProtocol();
        if (!request) {
            break;
        }
        // 更新定时器
        update(heartTimer, client);

        auto self = shared_from_this();
        // 启动一个任务协程
        go [request, session, self, this]() mutable {
            if(!session->isConnected()) {
                self = nullptr;
                return ;
            }
            Protocol::ptr response;
            Protocol::MsgType type = request->getMsgType();
            switch (type) {
                case Protocol::MsgType::HEARTBEAT_PACKET:
                    response = handleHeartbeatPacket(request);
                    break;
                case Protocol::MsgType::RPC_METHOD_REQUEST:
                    response = handleMethodCall(request);
                    break;
                case Protocol::MsgType::RPC_SUBSCRIBE_REQUEST:
                    response = handleSubscribe(request, session);
                    break;
                case Protocol::MsgType::RPC_PUBLISH_RESPONSE:
                    return;
                default:
                    STAR_LOG_DEBUG(g_logger) << "protocol:" << request->toString();
                    break;
            }

            if (response) {
                session->sendProtocol(response);
                //STAR_LOG_INFO(STAR_LOG_ROOT()) << "return response";
            }
            self = nullptr;
        };
    }

}

void RpcServer::update(Timer::ptr& heartTimer, Socket::ptr client) {
    STAR_LOG_DEBUG(g_logger) << "update heart";
    if (!heartTimer) {
        heartTimer = m_worker->addTimer(m_AliveTime, [client]{
            STAR_LOG_DEBUG(g_logger) << "client:" << client->toString() << " closed";
            client->close();
        });
        return;
    }
    // 更新定时器
    heartTimer->reset(m_AliveTime, true);
}

Serializer RpcServer::call(const std::string &name, const std::string& arg) {
    Serializer serializer;
    if (m_handlers.find(name) == m_handlers.end()) {
        return serializer;
    }
    auto fun = m_handlers[name];
    fun(serializer, arg);
    serializer.reset();
    return serializer;
}

Protocol::ptr RpcServer::handleMethodCall(Protocol::ptr p) {
    std::string func_name;
    Serializer request(p->getContent());
    request >> func_name;
    Serializer rt = call(func_name, request.toString());
    Protocol::ptr response = Protocol::Create(
            Protocol::MsgType::RPC_METHOD_RESPONSE, rt.toString(), p->getSequenceId());
    return response;
}

void RpcServer::registerService(const std::string &name) {
    Protocol::ptr proto = Protocol::Create(Protocol::MsgType::RPC_SERVICE_REGISTER, name);
    m_registry->sendProtocol(proto);

    Protocol::ptr rp = m_registry->recvProtocol();
    if (!rp) {
        STAR_LOG_WARN(g_logger) << "registerService:" << name << " fail, registry socket:" << m_registry->getSocket()->toString();
        return;
    }

    Result<std::string> result;
    Serializer s(rp->getContent());
    s >> result;

    if (result.getCode() != RPC_SUCCESS) {
        STAR_LOG_WARN(g_logger) << result.toString();
    }
    STAR_LOG_INFO(g_logger) << result.toString();
}

Protocol::ptr RpcServer::handleHeartbeatPacket(Protocol::ptr p) {
    return Protocol::HeartBeat();
}

Protocol::ptr RpcServer::handleSubscribe(Protocol::ptr proto, RpcSession::ptr client) {
    MutexType::Lock lock(m_sub_mtx);
    std::string key;
    Serializer s(proto->getContent());
    s >> key;
    m_subscribes.emplace(key, std::weak_ptr<RpcSession>(client));
    Result<> res = Result<>::Success();
    s.reset();
    s << res;
    return Protocol::Create(Protocol::MsgType::RPC_SUBSCRIBE_RESPONSE, s.toString(), 0);
}


}