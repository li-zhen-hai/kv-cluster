
#ifndef STAR_HTTP_SESSION_H
#define STAR_HTTP_SESSION_H
#include <memory>
#include "star/net/socket_stream.h"
#include "http.h"

namespace star::http {
class HttpSession : public SocketStream {
public:
    using ptr = std::shared_ptr<HttpSession>;

    HttpSession(Socket::ptr socket, bool owner = true);

    HttpRequest::ptr recvRequest();

    ssize_t sendResponse(HttpResponse::ptr response);
};
}

#endif //STAR_HTTP_SESSION_H
