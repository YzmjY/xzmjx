#include "log.h"
#include <iostream>
#include <vector>
#include <algorithm>

int main(){
    xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
    std::cout<<g_logger->getName()<<std::endl;
    XZMJX_LOG_DEBUG(g_logger)<<"This is a debug message";
    XZMJX_LOG_ERROR(g_logger)<<"This is a error message";
    XZMJX_LOG_INFO(g_logger)<<"This is a info message";
    XZMJX_LOG_FATAL(g_logger)<<"This is a fatal message";
    XZMJX_LOG_WARN(g_logger)<<"This is a warn message";
    return 0;
}