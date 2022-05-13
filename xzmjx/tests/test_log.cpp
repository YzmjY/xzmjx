#include "log.h"
#include "config.h"
#include <iostream>
#include <vector>
#include <algorithm>

int main(){
    xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
    YAML::Node n = YAML::LoadFile("/mnt/e/01-Code/xzmjx/xzmjx/bin/test/log.yml");
    YAML::Node n1 = YAML::LoadFile("/mnt/e/01-Code/xzmjx/xzmjx/bin/test/redis.yml");
    xzmjx::Config::LoadFromYaml(n);
    xzmjx::Config::LoadFromYaml(n1);
    //std::cout<<g_logger->getName()<<std::endl;
    XZMJX_LOG_DEBUG(g_logger)<<"This is a debug message";
    XZMJX_LOG_ERROR(g_logger)<<"This is a error message";
    XZMJX_LOG_INFO(g_logger)<<"This is a info message";
    XZMJX_LOG_FATAL(g_logger)<<"This is a fatal message";
    XZMJX_LOG_WARN(g_logger)<<"This is a warn message";

    g_logger = XZMJX_LOG_ROOT();
    XZMJX_LOG_DEBUG(g_logger)<<"This is a debug message";
    XZMJX_LOG_ERROR(g_logger)<<"This is a error message";
    XZMJX_LOG_INFO(g_logger)<<"This is a info message";
    XZMJX_LOG_FATAL(g_logger)<<"This is a fatal message";
    XZMJX_LOG_WARN(g_logger)<<"This is a warn message";
    return 0;
}