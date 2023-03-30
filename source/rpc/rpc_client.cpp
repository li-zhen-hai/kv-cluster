
#include "star/config.h"
#include "star/rpc/rpc_client.h"

static star::Logger::ptr g_logger = STAR_LOG_NAME("system");

namespace star::rpc {

static ConfigVar<size_t>::ptr g_channel_capacity =
        Config::Lookup<size_t>("rpc.client.channel_capacity",1024,"rpc client channel capacity");

static ConfigVar<uint64_t>::ptr client_hearbeat_timeout =
        Config::Lookup<uint64_t>("client_hearbeat_timeout",40'000,
                                 "client_hearbeat_timeout timeout (ms)");

static uint64_t s_channel_capacity = 1;

struct _RpcClientIniter{
    _RpcClientIniter(){
        s_channel_capacity = g_channel_capacity->getValue();
        g_channel_capacity->addListener([](const size_t& old_val, const size_t& new_val){
            STAR_LOG_INFO(g_logger) << "rpc client channel capacity changed from "
                                    << old_val << " to " << new_val;
            s_channel_capacity = new_val;
        });

        client_hearbeat_timeout->addListener([](const uint64_t& old_val,const uint64_t& new_val){
            STAR_LOG_INFO(g_logger) << "client_hearbeat_timeout changed from "
                                    << old_val << " to " << new_val;
        });
    }
};

static _RpcClientIniter s_initer;

RpcClient::RpcClient()
    : m_chan(s_channel_capacity){

}

RpcClient::~RpcClient() {
    STAR_LOG_INFO(g_logger) << "~RpcClient.";
    close();
}

void RpcClient::close() {
    STAR_LOG_DEBUG(g_logger) << "close";
    MutexType::Lock lock(m_mutex);

    if (m_isClose) {
        return;
    }

    m_isHeartClose = true;
    m_isClose = true;
    m_chan.close();

    for (auto i: m_responseHandle) {
        i.second << nullptr;
    }

    m_responseHandle.clear();

    if (m_heartTimer) {
        m_heartTimer->cancel();
        m_heartTimer = nullptr;
    }

    IOManager::GetThis()->delEvent(m_session->getSocket()->getSocket(), IOManager::READ);
    if (m_session->isConnected()) {
        m_session->close();
    }
}

bool RpcClient::connect(Address::ptr address){
    Socket::ptr sock = Socket::CreateTCP(address);

    if (!sock) {
        return false;
    }
    if (!sock->connect(address, m_timeout)) {
        m_session = nullptr;
        return false;
    }
    m_isHeartClose = false;
    m_isClose = false;
    m_session = std::make_shared<RpcSession>(sock);
    go [this] {
        // 开启 recv 协程
        handleRecv();
    };
    go [this] {
        // 开启 send 协程
        handleSend();
    };

    m_heartTimer = IOManager::GetThis()->addTimer(client_hearbeat_timeout->getValue(), [this]{
        STAR_LOG_DEBUG(g_logger) << "heart beat";
        if (m_isHeartClose) {
            STAR_LOG_DEBUG(g_logger) << "Server closed";
            close();
        }
        // 创建心跳包
        Protocol::ptr proto = Protocol::Create(Protocol::MsgType::HEARTBEAT_PACKET, "");
        // 向 send 协程的 Channel 发送消息
        m_chan << proto;
        m_isHeartClose = true;
    }, true);
    return true;
}

void RpcClient::setTimeout(uint64_t timeout_ms) {
    m_timeout = timeout_ms;
}

void RpcClient::handleSend() {
    Protocol::ptr request;
    // 通过 Channel 收集调用请求，如果没有消息时 Channel 内部会挂起该协程等待消息到达
    // Channel 被关闭时会退出循环
    while (!m_isClose && m_chan >> request) {
        if (!request) {
            STAR_LOG_WARN(g_logger) << m_session->getSocket() <<" RpcClient::handleSend() fail";
            continue;
        }
        // 发送请求
        m_session->sendProtocol(request);
    }
}

void RpcClient::handleRecv() {
    if (!m_session->isConnected()) {
        return;
    }
    while (!m_isClose) {
        // 接收响应
        Protocol::ptr response = m_session->recvProtocol();
        if (!response) {
            STAR_LOG_WARN(g_logger) << "RpcClient::handleRecv() fail";
            close();
            break;
        }
        m_isHeartClose = false;
        Protocol::MsgType type = response->getMsgType();
        // 判断响应类型进行对应的处理
        switch (type) {
            case Protocol::MsgType::HEARTBEAT_PACKET:
                m_isHeartClose = false;
                break;
            case Protocol::MsgType::RPC_METHOD_RESPONSE:
                // 处理调用结果
                handleMethodResponse(response);
                break;
            case Protocol::MsgType::RPC_PUBLISH_REQUEST:
                handlePublish(response);
                m_chan << Protocol::Create(Protocol::MsgType::RPC_PUBLISH_RESPONSE,"");
                break;
            case Protocol::MsgType::RPC_SUBSCRIBE_RESPONSE:
                break;
            default:
                STAR_LOG_DEBUG(g_logger) << "protocol:" << response->toString();
                break;
        }
    }
}

void RpcClient::handleMethodResponse(Protocol::ptr response) {
    // 获取该调用结果的序列号
    uint32_t id = response->getSequenceId();
    std::map<uint32_t, Channel<Protocol::ptr>>::iterator it;

    MutexType::Lock lock(m_mutex);
    // 查找该序列号的 Channel 是否还存在，如果不存在直接返回
    it = m_responseHandle.find(id);
    if (it == m_responseHandle.end()) {
        return;
    }
    // 通过序列号获取等待该结果的 Channel
    Channel<Protocol::ptr> chan = it->second;
    // 对该 Channel 发送调用结果唤醒调用者
    chan << response;
}

void RpcClient::handlePublish(Protocol::ptr proto) {
    Serializer s(proto->getContent());
    std::string key;
    s >> key;
    MutexType::Lock lock(m_sub_mtx);
    auto it = m_subHandle.find(key);
    if (it == m_subHandle.end()) return;
    it->second(s);
}

}