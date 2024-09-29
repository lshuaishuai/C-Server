#pragma once

#include <string.h>
#include <assert.h>

#include "util.h"
#include "log.h"

#define SHUAI_ASSERT(x) \
    if(!(x)) { \
        SHUAI_LOG_ERROR(SHUAI_LOG_ROOT()) << " ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << shuai::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#define SHUAI_ASSERT2(x, w) \
    if(!(x)) { \
        SHUAI_LOG_ERROR(SHUAI_LOG_ROOT()) << " ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << shuai::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

// 宏不受命名空间约束