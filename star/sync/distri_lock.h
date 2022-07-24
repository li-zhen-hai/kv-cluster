// #ifndef STAR_SYNC_DISTRI_LOCK_H
// #define STAR_SYNC_DISTRI_LOCK_H

// // #include "star/log.h"
// // #include "star/config.h"
// #include "star/CRQA/zk_client.h"
// #include "star/noncopyable.h"
// #include "star/sync/co_condvar.h"
// #include "star/sync/mutex.h"

// #include <unordered_map>
// #include <string>
// #include <memory>

// namespace star {

// class distri_lock : Noncopyable {
// public:
//     using ptr = std::shared_ptr<distri_lock>;

//     distri_lock(const std::string& ips="",star::ZKClient::watcher_callback func=nullptr);

//     bool lock(const std::string& path);

//     bool unlock(const std::string& path);

//     void watcher(int type, int stat, const std::string& path, star::ZKClient::ptr client);

//     void setWatchar(star::ZKClient::watcher_callback func) { on_watcher = func; }

//     ~distri_lock();

// private:
//     star::ZKClient::ptr zk_server;

//     //std::unordered_map<std::string,std::pair<CoCondVar*,CoMutex::Lock*>> m_locks;
//     std::unordered_map<std::string,CoCondVar*> m_locks;

//     star::ZKClient::watcher_callback on_watcher;

//     std::string hosts;

// };

// }

// #endif 
