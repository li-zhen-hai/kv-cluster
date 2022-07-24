#ifndef STAR_REDIS_H
#define STAR_REDIS_H

#include <stdlib.h>
#include <hiredis-vip/hiredis.h>
#include <hiredis-vip/hircluster.h>
#include <hiredis-vip/adapters/libevent.h>
#include <sys/time.h>
#include <string>
#include <memory>
#include <vector>
#include <map>

#include "star/scheduler.h"

namespace star {

typedef std::shared_ptr<redisReply> ReplyPtr;

class IRedis {
public:
    enum Type {
        REDIS = 1,
        REDIS_CLUSTER = 2,
        FOX_REDIS = 3,
        FOX_REDIS_CLUSTER = 4
    };
    typedef std::shared_ptr<IRedis> ptr;
    IRedis() : m_logEnable(true) { }
    virtual ~IRedis() {}

    virtual ReplyPtr cmd(const char* fmt, ...) = 0;
    virtual ReplyPtr cmd(const char* fmt, va_list ap) = 0;
    virtual ReplyPtr cmd(const std::vector<std::string>& argv) = 0;

    const std::string& getName() const { return m_name;}
    void setName(const std::string& v) { m_name = v;}

    const std::string& getPasswd() const { return m_passwd;}
    void setPasswd(const std::string& v) { m_passwd = v;}

    Type getType() const { return m_type;}
protected:
    std::string m_name;
    std::string m_passwd;
    Type m_type;
    bool m_logEnable;
};

class ISyncRedis : public IRedis {
public:
    typedef std::shared_ptr<ISyncRedis> ptr;
    virtual ~ISyncRedis() {}

    virtual bool reconnect() = 0;
    virtual bool connect(const std::string& ip, int port, uint64_t ms = 0) = 0;
    virtual bool connect() = 0;
    virtual bool setTimeout(uint64_t ms) = 0;

    virtual int appendCmd(const char* fmt, ...) = 0;
    virtual int appendCmd(const char* fmt, va_list ap) = 0;
    virtual int appendCmd(const std::vector<std::string>& argv) = 0;

    virtual ReplyPtr getReply() = 0;

    uint64_t getLastActiveTime() const { return m_lastActiveTime;}
    void setLastActiveTime(uint64_t v) { m_lastActiveTime = v;}

protected:
    uint64_t m_lastActiveTime;
};

class Redis : public ISyncRedis {
public:
    typedef std::shared_ptr<Redis> ptr;
    Redis();
    Redis(const std::map<std::string, std::string>& conf);

    virtual bool reconnect();
    virtual bool connect(const std::string& ip, int port, uint64_t ms = 0);
    virtual bool connect();
    virtual bool setTimeout(uint64_t ms);

    virtual ReplyPtr cmd(const char* fmt, ...);
    virtual ReplyPtr cmd(const char* fmt, va_list ap);
    virtual ReplyPtr cmd(const std::vector<std::string>& argv);

    virtual int appendCmd(const char* fmt, ...);
    virtual int appendCmd(const char* fmt, va_list ap);
    virtual int appendCmd(const std::vector<std::string>& argv);

    virtual ReplyPtr getReply();
private:
    std::string m_host;
    uint32_t m_port;
    uint32_t m_connectMs;
    struct timeval m_cmdTimeout;
    std::shared_ptr<redisContext> m_context;
};

class RedisCluster : public ISyncRedis {
public:
    typedef std::shared_ptr<RedisCluster> ptr;
    RedisCluster();
    RedisCluster(const std::map<std::string, std::string>& conf);

    virtual bool reconnect();
    virtual bool connect(const std::string& ip, int port, uint64_t ms = 0);
    virtual bool connect();
    virtual bool setTimeout(uint64_t ms);

    virtual ReplyPtr cmd(const char* fmt, ...);
    virtual ReplyPtr cmd(const char* fmt, va_list ap);
    virtual ReplyPtr cmd(const std::vector<std::string>& argv);

    virtual int appendCmd(const char* fmt, ...);
    virtual int appendCmd(const char* fmt, va_list ap);
    virtual int appendCmd(const std::vector<std::string>& argv);

    virtual ReplyPtr getReply();
private:
    std::string m_host;
    uint32_t m_port;
    uint32_t m_connectMs;
    struct timeval m_cmdTimeout;
    std::shared_ptr<redisClusterContext> m_context;
};

class RedisManager {
public:
    RedisManager();
    IRedis::ptr get(const std::string& name);

    std::ostream& dump(std::ostream& os);
private:
    void freeRedis(IRedis* r);
    void init();
private:
    star::RWMutex m_mutex;
    std::map<std::string, std::list<IRedis*> > m_datas;
    std::map<std::string, std::map<std::string, std::string> > m_config;
};

typedef star::Singleton<RedisManager> RedisMgr;

class RedisUtil {
public:
    static ReplyPtr Cmd(const std::string& name, const char* fmt, ...);
    static ReplyPtr Cmd(const std::string& name, const char* fmt, va_list ap); 
    static ReplyPtr Cmd(const std::string& name, const std::vector<std::string>& args); 

    static ReplyPtr TryCmd(const std::string& name, uint32_t count, const char* fmt, ...);
    static ReplyPtr TryCmd(const std::string& name, uint32_t count, const std::vector<std::string>& args); 
};

}

#endif
