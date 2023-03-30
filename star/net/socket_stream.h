
#ifndef STAR_SOCKET_STREAM_H
#define STAR_SOCKET_STREAM_H
#include <memory>
#include "socket.h"
#include "star/stream.h"
namespace star {
class SocketStream : public Stream {
public:
    using ptr = std::shared_ptr<SocketStream>;
    SocketStream(Socket::ptr socket, bool owner = true);
    ~SocketStream();

    bool isConnected() const { return m_socket && m_socket->isConnected();}
    Socket::ptr getSocket() { return m_socket;}

    ssize_t read(void* buffer, size_t length) override;
    ssize_t read(ByteArray::ptr buffer, size_t length) override;

    ssize_t write(const void* buffer, size_t length) override;
    ssize_t write(ByteArray::ptr buffer, size_t length) override;

    void close() override;

protected:
    Socket::ptr m_socket;
    bool m_isOwner;
};

}
#endif //STAR_SOCKET_STREAM_H
