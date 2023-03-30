
#ifndef STAR_HTTP_SERVER_H
#define STAR_HTTP_SERVER_H
#include <memory>
#include "http.h"
#include "http_session.h"
#include "servlet.h"
#include "star/net/tcp_server.h"

namespace star::http {
class HttpServer : public TcpServer {
public:
    using ptr = std::shared_ptr<HttpServer>;
    HttpServer(bool keepalive = false,
               IOManager* worker = IOManager::GetThis(),
               IOManager* accept_worker = IOManager::GetThis());

    ServletDispatch::ptr getServletDispatch() const { return m_dispatch;}
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v;}
    void setName(const std::string& name) override;
protected:
    void handleClient(Socket::ptr client) override;

private:
    bool m_isKeepalive;
    ServletDispatch::ptr m_dispatch;
};
}
#endif //STAR_HTTP_SERVER_H
