
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>

#include "star/http/servlets/file_servlet.h"

static star::Logger::ptr g_logger = STAR_LOG_NAME("http");

namespace star::http {
FileServlet::FileServlet(const std::string &path)
    : Servlet("FileServlet"), m_path(path) {

}

int32_t FileServlet::handle(HttpRequest::ptr request, HttpResponse::ptr response, HttpSession::ptr session) {
    // STAR_LOG_DEBUG(g_logger) << request->getPath();
    if (request->getPath().find("..") != std::string::npos) {
        NotFoundServlet("morningstar").handle(request, response, session);
        return 1;
    }

    std::string filename = m_path + request->getPath();

    // STAR_LOG_DEBUG(g_logger) << filename;

    struct stat filestat;
    if(stat(filename.c_str(), &filestat) < 0){
        NotFoundServlet("morningstar").handle(request, response, session);
        return 1;
    }

    // STAR_LOG_DEBUG(g_logger) << request->getPath() << " second!";

    if(!S_ISREG(filestat.st_mode) || !(S_IRUSR & filestat.st_mode)){
        NotFoundServlet("morningstar").handle(request, response, session);
        return 1;
    }

    STAR_LOG_DEBUG(g_logger) << request->getPath() << " third!";

    response->setStatus(http::HttpStatus::OK);
    //response->setHeader("Content-Type","text/plain");
    response->setHeader("Server", "morningstar/1.0.0");
    response->setHeader("Content-length",std::to_string(filestat.st_size));

    session->sendResponse(response);

    int filefd = open(filename.c_str(), O_RDONLY);
    sendfile(session->getSocket()->getSocket(), filefd, 0, filestat.st_size);

    return 1;
}

}