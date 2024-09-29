#include <assert.h>

#include "server/server.h"

shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();

void test_assert()
{
    SHUAI_LOG_INFO(g_logger) << shuai::BacktraceToString(10, 2, "        ");
    SHUAI_ASSERT2(0 == 1, "abcdefg xx");
}

int main(int argc, char** argv)
{
    test_assert();
    return 0;
}