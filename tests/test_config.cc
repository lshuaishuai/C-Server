#include <iostream>

#include "../server/log.h"
#include "../server/config.h"

shuai::ConfigVar<int>::ptr g_int_value_config = 
    shuai::Config::Lookup("system.port", (int)8080, "system port");

 shuai::ConfigVar<float>::ptr g_int_float_config = 
    shuai::Config::Lookup("system.value", (float)10.2f, "system value");

int main(int argc, char** argv)
{
    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << g_int_value_config->getValue();
    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << g_int_value_config->toString();;

    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << g_int_float_config->getValue();
    SHUAI_LOG_INFO(SHUAI_LOG_ROOT()) << g_int_float_config->toString();;
    return 0; 
}