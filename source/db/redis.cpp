#include "star/db/redis.h"
#include "star/log.h"
#include "star/config.h"


#include <map>

namespace star {

static star::Logger::ptr g_logger = STAR_LOG_NAME("system");
static ConfigVar<std::map<std::string, std::map<std::string, std::string> > >::ptr g_redis =
    Config::Lookup("redis.config", std::map<std::string, std::map<std::string, std::string> >(), "redis config");

static std::string get_value(const std::map<std::string, std::string>& m
                             ,const std::string& key
                             ,const std::string& def = "") {
    auto it = m.find(key);
    return it == m.end() ? def : it->second;
}

redisReply* RedisReplyClone(redisReply* r) {
    redisReply* c = (redisReply*)calloc(1, sizeof(*c));
    c->type = r->type;

    switch(r->type) {
        case REDIS_REPLY_INTEGER:
            c->integer = r->integer;
            break;
        case REDIS_REPLY_ARRAY:
            if(r->element != NULL && r->elements > 0) {
                c->element = (redisReply**)calloc(r->elements, sizeof(r));
                c->elements = r->elements;
                for(size_t i = 0; i < r->elements; ++i) {
                    c->element[i] = RedisReplyClone(r->element[i]);
                }
            }
            break;
        case REDIS_REPLY_ERROR:
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_STRING:
            if(r->str == NULL) {
                c->str = NULL;
            } else {
                //c->str = strndup(r->str, r->len);
                c->str = (char*)malloc(r->len + 1);
                memcpy(c->str, r->str, r->len);
                c->str[r->len] = '\0';
            }
            c->len = r->len;
            break;
    }
    return c;
}

Redis::Redis() {
    m_type = IRedis::REDIS;
}

Redis::Redis(const std::map<std::string, std::string>& conf) {
    m_type = IRedis::REDIS;
    auto tmp = get_value(conf, "host");
    auto pos = tmp.find(":");
    m_host = tmp.substr(0, pos);
    m_port = star::TypeUtil::Atoi(tmp.substr(pos + 1));
    m_passwd = get_value(conf, "passwd");
    m_logEnable = star::TypeUtil::Atoi(get_value(conf, "log_enable", "1"));

    tmp = get_value(conf, "timeout_com");
    if(tmp.empty()) {
        tmp = get_value(conf, "timeout");
    }
    uint64_t v = star::TypeUtil::Atoi(tmp);

    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
}

bool Redis::reconnect() {
    return redisReconnect(m_context.get());
}

bool Redis::connect() {
    return connect(m_host, m_port, 50);
}

bool Redis::connect(const std::string& ip, int port, uint64_t ms) {
    m_host = ip;
    m_port = port;
    m_connectMs = ms;
    if(m_context) {
        return true;
    }
    timeval tv = {(int)ms / 1000, (int)ms % 1000 * 1000};
    auto c = new redisContext();
    c = redisConnectWithTimeout(ip.c_str(), port, tv);
    if(c != nullptr) {
        m_context.reset(c, redisFree);
        if(m_cmdTimeout.tv_sec || m_cmdTimeout.tv_usec) {
            setTimeout(m_cmdTimeout.tv_sec * 1000 + m_cmdTimeout.tv_usec / 1000);
        }

        if(!m_passwd.empty()) {
            auto r = (redisReply*)redisCommand(c, "auth %s", m_passwd.c_str());
            if(!r) {
                STAR_LOG_ERROR(g_logger) << "auth error:("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
            if(r->type != REDIS_REPLY_STATUS) {
                STAR_LOG_ERROR(g_logger) << "auth reply type error:" << r->type << "("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
            if(!r->str) {
                STAR_LOG_ERROR(g_logger) << "auth reply str error: NULL("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
            if(strcmp(r->str, "OK") == 0) {
                return true;
            } else {
                STAR_LOG_ERROR(g_logger) << "auth error: " << r->str << "("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
        }
        return true;
    }
    return false;
}

bool Redis::setTimeout(uint64_t v) {
    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
    redisSetTimeout(m_context.get(), m_cmdTimeout);
    return true;
}

ReplyPtr Redis::cmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ReplyPtr rt = cmd(fmt, ap);
    va_end(ap);
    return rt;
}

ReplyPtr Redis::cmd(const char* fmt, va_list ap) {
    auto r = (redisReply*)redisvCommand(m_context.get(), fmt, ap);
    if(!r) {
        if(m_logEnable) {
            STAR_LOG_ERROR(g_logger) << "redisCommand error: (" << fmt << ")(" << m_host << ":" << m_port << ")(" << m_name << ")";
        }
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR) {
        return rt;
    }
    if(m_logEnable) {
        STAR_LOG_ERROR(g_logger) << "redisCommand error: (" << fmt << ")(" << m_host << ":" << m_port << ")(" << m_name << ")"
                    << ": " << r->str;
    }
    return nullptr;
}

ReplyPtr Redis::cmd(const std::vector<std::string>& argv) {
    std::vector<const char*> v;
    std::vector<size_t> l;
    for(auto& i : argv) {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }

    auto r = (redisReply*)redisCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
    if(!r) {
        if(m_logEnable) {
            STAR_LOG_ERROR(g_logger) << "redisCommandArgv error: (" << m_host << ":" << m_port << ")(" << m_name << ")";
        }
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR) {
        return rt;
    }
    if(m_logEnable) {
        STAR_LOG_ERROR(g_logger) << "redisCommandArgv error: (" << m_host << ":" << m_port << ")(" << m_name << ")"
                    << r->str;
    }
    return nullptr;
}

ReplyPtr Redis::getReply() {
    redisReply* r = nullptr;
    if(redisGetReply(m_context.get(), (void**)&r) == REDIS_OK) {
        ReplyPtr rt(r, freeReplyObject);
        return rt;
    }
    if(m_logEnable) {
        STAR_LOG_ERROR(g_logger) << "redisGetReply error: (" << m_host << ":" << m_port << ")(" << m_name << ")";
    }
    return nullptr;
}

int Redis::appendCmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rt = appendCmd(fmt, ap);
    va_end(ap);
    return rt;

}

int Redis::appendCmd(const char* fmt, va_list ap) {
    return redisvAppendCommand(m_context.get(), fmt, ap);
}

int Redis::appendCmd(const std::vector<std::string>& argv) {
    std::vector<const char*> v;
    std::vector<size_t> l;
    for(auto& i : argv) {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }
    return redisAppendCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
}

RedisCluster::RedisCluster() {
    m_type = IRedis::REDIS_CLUSTER;
}

RedisCluster::RedisCluster(const std::map<std::string, std::string>& conf) {
    m_type = IRedis::REDIS_CLUSTER;
    m_host = get_value(conf, "host");
    m_passwd = get_value(conf, "passwd");
    m_logEnable = star::TypeUtil::Atoi(get_value(conf, "log_enable", "1"));
    auto tmp = get_value(conf, "timeout_com");
    if(tmp.empty()) {
        tmp = get_value(conf, "timeout");
    }
    uint64_t v = star::TypeUtil::Atoi(tmp);

    m_cmdTimeout.tv_sec = v / 1000;
    m_cmdTimeout.tv_usec = v % 1000 * 1000;
}


////RedisCluster
bool RedisCluster::reconnect() {
    return true;
    //return redisReconnect(m_context.get());
}

bool RedisCluster::connect() {
    return connect(m_host, m_port, 50);
}

bool RedisCluster::connect(const std::string& ip, int port, uint64_t ms) {
    m_host = ip;
    m_port = port;
    m_connectMs = ms;
    if(m_context) {
        return true;
    }
    timeval tv = {(int)ms / 1000, (int)ms % 1000 * 1000};
    auto c = redisClusterConnectWithTimeout(ip.c_str(), tv, 0);
    if(c) {
        m_context.reset(c, redisClusterFree);
        if(!m_passwd.empty()) {
            auto r = (redisReply*)redisClusterCommand(c, "auth %s", m_passwd.c_str());
            if(!r) {
                STAR_LOG_ERROR(g_logger) << "auth error:("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
            if(r->type != REDIS_REPLY_STATUS) {
                STAR_LOG_ERROR(g_logger) << "auth reply type error:" << r->type << "("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
            if(!r->str) {
                STAR_LOG_ERROR(g_logger) << "auth reply str error: NULL("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
            if(strcmp(r->str, "OK") == 0) {
                return true;
            } else {
                STAR_LOG_ERROR(g_logger) << "auth error: " << r->str << "("
                    << m_host << ":" << m_port << ", " << m_name << ")";
                return false;
            }
        }
        return true;
    }
    return false;
}

bool RedisCluster::setTimeout(uint64_t ms) {
    timeval tv = {(int)ms / 1000, (int)ms % 1000 * 1000};
    redisClusterSetOptionConnectTimeout(m_context.get(),tv);
    return true;
}

ReplyPtr RedisCluster::cmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ReplyPtr rt = cmd(fmt, ap);
    va_end(ap);
    return rt;
}

ReplyPtr RedisCluster::cmd(const char* fmt, va_list ap) {
    auto r = (redisReply*)redisClustervCommand(m_context.get(), fmt, ap);
    if(!r) {
        if(m_logEnable) {
            STAR_LOG_ERROR(g_logger) << "redisCommand error: (" << fmt << ")(" << m_host << ":" << m_port << ")(" << m_name << ")";
        }
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR) {
        return rt;
    }
    if(m_logEnable) {
        STAR_LOG_ERROR(g_logger) << "redisCommand error: (" << fmt << ")(" << m_host << ":" << m_port << ")(" << m_name << ")"
                    << ": " << r->str;
    }
    return nullptr;
}

ReplyPtr RedisCluster::cmd(const std::vector<std::string>& argv) {
    std::vector<const char*> v;
    std::vector<size_t> l;
    for(auto& i : argv) {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }

    auto r = (redisReply*)redisClusterCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
    if(!r) {
        if(m_logEnable) {
            STAR_LOG_ERROR(g_logger) << "redisCommandArgv error: (" << m_host << ":" << m_port << ")(" << m_name << ")";
        }
        return nullptr;
    }
    ReplyPtr rt(r, freeReplyObject);
    if(r->type != REDIS_REPLY_ERROR) {
        return rt;
    }
    if(m_logEnable) {
        STAR_LOG_ERROR(g_logger) << "redisCommandArgv error: (" << m_host << ":" << m_port << ")(" << m_name << ")"
                    << r->str;
    }
    return nullptr;
}

ReplyPtr RedisCluster::getReply() {
    redisReply* r = nullptr;
    if(redisClusterGetReply(m_context.get(), (void**)&r) == REDIS_OK) {
        ReplyPtr rt(r, freeReplyObject);
        return rt;
    }
    if(m_logEnable) {
        STAR_LOG_ERROR(g_logger) << "redisGetReply error: (" << m_host << ":" << m_port << ")(" << m_name << ")";
    }
    return nullptr;
}

int RedisCluster::appendCmd(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rt = appendCmd(fmt, ap);
    va_end(ap);
    return rt;

}

int RedisCluster::appendCmd(const char* fmt, va_list ap) {
    return redisClustervAppendCommand(m_context.get(), fmt, ap);
}

int RedisCluster::appendCmd(const std::vector<std::string>& argv) {
    std::vector<const char*> v;
    std::vector<size_t> l;
    for(auto& i : argv) {
        v.push_back(i.c_str());
        l.push_back(i.size());
    }
    return redisClusterAppendCommandArgv(m_context.get(), argv.size(), &v[0], &l[0]);
}
IRedis::ptr RedisManager::get(const std::string& name) {
    star::RWMutex::WriteLock lock(m_mutex);
    auto it = m_datas.find(name);
    if(it == m_datas.end()) {
        return nullptr;
    }
    if(it->second.empty()) {
        return nullptr;
    }
    auto r = it->second.front();
    it->second.pop_front();
    if(r->getType() == IRedis::FOX_REDIS
            || r->getType() == IRedis::FOX_REDIS_CLUSTER) {
        it->second.push_back(r);
        return std::shared_ptr<IRedis>(r, star::nop<IRedis>);
    }
    lock.unlock();
    auto rr = dynamic_cast<ISyncRedis*>(r);
    if((time(0) - rr->getLastActiveTime()) > 30) {
        if(!rr->cmd("ping")) {
            if(!rr->reconnect()) {
                star::RWMutex::WriteLock lock(m_mutex);
                m_datas[name].push_back(r);
                return nullptr;
            }
        }
    }
    rr->setLastActiveTime(time(0));
    return std::shared_ptr<IRedis>(r, std::bind(&RedisManager::freeRedis
                        ,this, std::placeholders::_1));
}

void RedisManager::freeRedis(IRedis* r) {
    star::RWMutex::WriteLock lock(m_mutex);
    m_datas[r->getName()].push_back(r);
}

RedisManager::RedisManager() {
    init();
}

void RedisManager::init() {
    m_config = g_redis->getValue();
    size_t done = 0;
    size_t total = 0;
    for(auto& i : m_config) {
        auto type = get_value(i.second, "type");
        auto pool = star::TypeUtil::Atoi(get_value(i.second, "pool"));
        auto passwd = get_value(i.second, "passwd");
        total += pool;
        for(int n = 0; n < pool; ++n) {
            if(type == "redis") {
                star::Redis* rds(new star::Redis(i.second));
                rds->connect();
                rds->setLastActiveTime(time(0));
                star::RWMutex::WriteLock lock(m_mutex);
                m_datas[i.first].push_back(rds);
                star::Atomic::addFetch(done, 1);
            } else if(type == "redis_cluster") {
                star::RedisCluster* rds(new star::RedisCluster(i.second));
                rds->connect();
                rds->setLastActiveTime(time(0));
                star::RWMutex::WriteLock lock(m_mutex);
                m_datas[i.first].push_back(rds);
                star::Atomic::addFetch(done, 1);
            } else {
                star::Atomic::addFetch(done, 1);
            }
        }
    }

    while(done != total) {
        usleep(5000);
    }
}

std::ostream& RedisManager::dump(std::ostream& os) {
    os << "[RedisManager total=" << m_config.size() << "]" << std::endl;
    for(auto& i : m_config) {
        os << "    " << i.first << " :[";
        for(auto& n : i.second) {
            os << "{" << n.first << ":" << n.second << "}";
        }
        os << "]" << std::endl;
    }
    return os;
}

ReplyPtr RedisUtil::Cmd(const std::string& name, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ReplyPtr rt = Cmd(name, fmt, ap);
    va_end(ap);
    return rt;
}

ReplyPtr RedisUtil::Cmd(const std::string& name, const char* fmt, va_list ap) {
    auto rds = RedisMgr::GetInstance()->get(name);
    if(!rds) {
        return nullptr;
    }
    return rds->cmd(fmt, ap);
}

ReplyPtr RedisUtil::Cmd(const std::string& name, const std::vector<std::string>& args) {
    auto rds = RedisMgr::GetInstance()->get(name);
    if(!rds) {
        return nullptr;
    }
    return rds->cmd(args);
}


ReplyPtr RedisUtil::TryCmd(const std::string& name, uint32_t count, const char* fmt, ...) {
    for(uint32_t i = 0; i < count; ++i) {
        va_list ap;
        va_start(ap, fmt);
        ReplyPtr rt = Cmd(name, fmt, ap);
        va_end(ap);

        if(rt) {
            return rt;
        }
    }
    return nullptr;
}

ReplyPtr RedisUtil::TryCmd(const std::string& name, uint32_t count, const std::vector<std::string>& args) {
    for(uint32_t i = 0; i < count; ++i) {
        ReplyPtr rt = Cmd(name, args);
        if(rt) {
            return rt;
        }
    }
    return nullptr;
}

}
