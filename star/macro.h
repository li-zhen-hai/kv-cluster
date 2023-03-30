
#ifndef STAR_MACRO_H
#define STAR_MACRO_H

#include <assert.h>
#include <stdlib.h>
#include <string>
#include "util.h"

#if defined(__GNUC__)  || defined(__llvm__)
#   define STAR_LIKELY(x)       __builtin_expect(!!(x), 1)
#   define STAR_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define STAR_LIKELY(x)       (x)
#   define STAR_UNLIKELY(x)     (x)
#endif

#define STAR_ASSERT(x) \
if(STAR_UNLIKELY(!(x))){          \
    STAR_LOG_ERROR(STAR_LOG_ROOT()) << "ASSERTION: " << #x \
        << "\nbacktrace:\n" << star::BacktraceToString(100,2,"    "); \
    assert(x);         \
    exit(1);                       \
}
#define STAR_ASSERT2(x,m) \
if(STAR_UNLIKELY(!(x))){                 \
    STAR_LOG_ERROR(STAR_LOG_ROOT()) << "ASSERTION: " << #x \
        << "\n" << m <<"\n"              \
        << "\nbacktrace:\n" << star::BacktraceToString(100,2,"    "); \
    assert(x);            \
    exit(1);                       \
}

#define STAR_STATIC_ASSERT(x) \
if constexpr(!(x)){          \
    STAR_LOG_ERROR(STAR_LOG_ROOT()) << "ASSERTION: " << #x \
        << "\nbacktrace:\n" << star::BacktraceToString(100,2,"    "); \
    exit(1);                          \
}

#endif //STAR_MACRO_H
