
#include <execinfo.h>
#include "star/fiber.h"
#include "star/log.h"
#include "star/util.h"
namespace star{
static Logger::ptr g_logger = STAR_LOG_NAME("system");

pid_t GetThreadId(){
    return syscall(SYS_gettid);
}

int64_t GetFiberId(){
    return Fiber::GetFiberId();
}

void Backtrace(std::vector<std::string>& bt, int size, int skip) {
    void** array = (void**)malloc((sizeof(void*) * size));
    size_t s = backtrace(array, size);

    char** strings = backtrace_symbols(array, s);
    if(strings == NULL) {
        STAR_LOG_ERROR(g_logger) << "backtrace_synbols error";
        free(array);
        return;
    }

    for(size_t i = skip; i < s; ++i) {
        bt.push_back(strings[i]);
    }

    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for(size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}

uint64_t GetCurrentMS(){
    struct timeval tm;
    gettimeofday(&tm,0);
    return tm.tv_sec * 1000ul + tm.tv_usec / 1000;
}

uint64_t GetCurrentUS(){
    struct timeval tm;
    gettimeofday(&tm,0);
    return tm.tv_sec * 1000ul * 1000ul + tm.tv_usec;
}

int8_t  TypeUtil::ToChar(const std::string& str) {
    if(str.empty()) {
        return 0;
    }
    return *str.begin();
}

int64_t TypeUtil::Atoi(const std::string& str) {
    if(str.empty()) {
        return 0;
    }
    return strtoull(str.c_str(), nullptr, 10);
}

double  TypeUtil::Atof(const std::string& str) {
    if(str.empty()) {
        return 0;
    }
    return atof(str.c_str());
}

int8_t  TypeUtil::ToChar(const char* str) {
    if(str == nullptr) {
        return 0;
    }
    return str[0];
}

int64_t TypeUtil::Atoi(const char* str) {
    if(str == nullptr) {
        return 0;
    }
    return strtoull(str, nullptr, 10);
}

double  TypeUtil::Atof(const char* str) {
    if(str == nullptr) {
        return 0;
    }
    return atof(str);
}

std::string replace(const std::string &str1, char find, char replaceWith) {
    auto str = str1;
    size_t index = str.find(find);
    while (index != std::string::npos) {
        str[index] = replaceWith;
        index = str.find(find, index + 1);
    }
    return str;
}

std::string replace(const std::string &str1, char find, const std::string &replaceWith) {
    auto str = str1;
    size_t index = str.find(find);
    while (index != std::string::npos) {
        str = str.substr(0, index) + replaceWith + str.substr(index + 1);
        index = str.find(find, index + replaceWith.size());
    }
    return str;
}

std::string replace(const std::string &str1, const std::string &find, const std::string &replaceWith) {
    auto str = str1;
    size_t index = str.find(find);
    while (index != std::string::npos) {
        str = str.substr(0, index) + replaceWith + str.substr(index + find.size());
        index = str.find(find, index + replaceWith.size());
    }
    return str;
}


}