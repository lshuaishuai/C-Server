#pragma once

#include <string.h>
#include <assert.h>

#include "util.h"
#include "log.h"

#if defined __GUNC__ || defined __llvm__
#   define SHUAI_LICKLY(x)        __builtin_expect(!!(x), 1)
#   define SHUAI_UNLICKLY(x)      __builtin_expect(!!(x), 0)
#else
#   define SHUAI_LICKLY(x)      (x)
#   define SHUAI_UNLICKLY(x)    (x)
#endif

#define SHUAI_ASSERT(x) \
    if(SHUAI_UNLICKLY(!(x))) { \
        SHUAI_LOG_ERROR(SHUAI_LOG_ROOT()) << " ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << shuai::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

// 如果x是假的话，输出其所在的函数栈，并断言停止程序的运行
#define SHUAI_ASSERT2(x, w) \
    if(SHUAI_UNLICKLY(!(x))) { \
        SHUAI_LOG_ERROR(SHUAI_LOG_ROOT()) << " ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << shuai::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

// 宏不受命名空间约束