
#include "star/log.h"
#include "star/file_monitor.h"
using namespace star;
int main() {

    FileMonitor fileMonitor;
    fileMonitor.addFileWatch("/home/morningstar/star/config/my.yaml", FileMonitor::ALL,
                             [](FileMonitor::EventContext ctx){
        LOG_DEBUG << "log file modify";
    });

    // // fileMonitor.addDirectoryWatch("/home/zavier/tmp", FileMonitor::ALL,
    // //                           [](std::vector<FileMonitor::EventContext> vec){
    // //     for(auto e: vec) {
    // //       LOG_DEBUG << e.path;
    // //     }
    // // });

    sleep(100);

    return 0;

}