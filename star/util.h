
#ifndef STAR_UTIL_H
#define STAR_UTIL_H

#include <byteswap.h>
#include <unistd.h>
#include <syscall.h>
#include <sys/time.h>

#include <bit>
#include <concepts>
#include <cstdint>
#include <string_view>
#include <algorithm>
#include <iostream>

#include "log.h"

namespace star{

pid_t GetThreadId();
int64_t GetFiberId();

void Backtrace(std::vector<std::string>& bt, int size, int skip = 1 );
std::string BacktraceToString(int size, int skip = 2, const std::string& prefix = "");

//时间
uint64_t GetCurrentMS();
uint64_t GetCurrentUS();


//bit

/**
* @brief 字节序转换
*/
template<std::integral T>
constexpr T ByteSwap(T value) {
    if constexpr(sizeof(T) == sizeof(uint8_t)){
        return (T)bswap_16((uint16_t)value);
    } else if constexpr(sizeof(T) == sizeof(uint16_t)){
        return (T)bswap_16((uint16_t)value);
    } else if constexpr(sizeof(T) == sizeof(uint32_t)){
        return (T)bswap_32((uint32_t)value);
    } else if constexpr(sizeof(T) == sizeof(uint64_t)){
        return (T)bswap_64((uint64_t)value);
    }
}

/**
* @brief 网络序主机序转换
*/
template<std::integral T>
constexpr T EndianCast(T value) {
    if constexpr(sizeof(T) == sizeof(uint8_t)
                 || std::endian::native == std::endian::big){
        return value;
    } else if constexpr(std::endian::native == std::endian::little){
        return ByteSwap(value);
    }
}

template<class Iter>
std::string Join(Iter begin, Iter end, const std::string& tag) {
    std::stringstream ss;
    for(Iter it = begin; it != end; ++it) {
        if(it != begin) {
            ss << tag;
        }
        ss << *it;
    }
    return ss.str();
}

class TypeUtil {
public:
    static int8_t ToChar(const std::string& str);
    static int64_t Atoi(const std::string& str);
    static double Atof(const std::string& str);
    static int8_t ToChar(const char* str);
    static int64_t Atoi(const char* str);
    static double Atof(const char* str);
};

class Atomic {
public:
    template<class T, class S = T>
    static T addFetch(volatile T& t, S v = 1) {
        return __sync_add_and_fetch(&t, (T)v);
    }

    template<class T, class S = T>
    static T subFetch(volatile T& t, S v = 1) {
        return __sync_sub_and_fetch(&t, (T)v);
    }

    template<class T, class S>
    static T orFetch(volatile T& t, S v) {
        return __sync_or_and_fetch(&t, (T)v);
    }

    template<class T, class S>
    static T andFetch(volatile T& t, S v) {
        return __sync_and_and_fetch(&t, (T)v);
    }

    template<class T, class S>
    static T xorFetch(volatile T& t, S v) {
        return __sync_xor_and_fetch(&t, (T)v);
    }

    template<class T, class S>
    static T nandFetch(volatile T& t, S v) {
        return __sync_nand_and_fetch(&t, (T)v);
    }

    template<class T, class S>
    static T fetchAdd(volatile T& t, S v = 1) {
        return __sync_fetch_and_add(&t, (T)v);
    }

    template<class T, class S>
    static T fetchSub(volatile T& t, S v = 1) {
        return __sync_fetch_and_sub(&t, (T)v);
    }

    template<class T, class S>
    static T fetchOr(volatile T& t, S v) {
        return __sync_fetch_and_or(&t, (T)v);
    }

    template<class T, class S>
    static T fetchAnd(volatile T& t, S v) {
        return __sync_fetch_and_and(&t, (T)v);
    }

    template<class T, class S>
    static T fetchXor(volatile T& t, S v) {
        return __sync_fetch_and_xor(&t, (T)v);
    }

    template<class T, class S>
    static T fetchNand(volatile T& t, S v) {
        return __sync_fetch_and_nand(&t, (T)v);
    }

    template<class T, class S>
    static T compareAndSwap(volatile T& t, S old_val, S new_val) {
        return __sync_val_compare_and_swap(&t, (T)old_val, (T)new_val);
    }

    template<class T, class S>
    static bool compareAndSwapBool(volatile T& t, S old_val, S new_val) {
        return __sync_bool_compare_and_swap(&t, (T)old_val, (T)new_val);
    }
};

template<class T>
void nop(T*) {}

std::string replace(const std::string &str, char find, char replaceWith);
std::string replace(const std::string &str, char find, const std::string &replaceWith);
std::string replace(const std::string &str, const std::string &find, const std::string &replaceWith);


}

#endif //STAR_UTIL_H
