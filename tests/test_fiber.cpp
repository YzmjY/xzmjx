//
// Created by 20132 on 2022/3/18.
//

#include "fiber.h"
#include "log.h"
#include <iostream>
static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");

void fun(){
    XZMJX_LOG_INFO(g_logger)<<"fiber start";
    xzmjx::Fiber::YieldToReady();
    XZMJX_LOG_INFO(g_logger)<<"fiber resume";
}

int main(){
    XZMJX_LOG_INFO(g_logger)<<"begin";
    g_logger->setLevel(xzmjx::LogLevel::DEBUG);
    XZMJX_LOG_INFO(g_logger)<<"begin";
    xzmjx::Fiber::EnableFiber();
    XZMJX_LOG_INFO(g_logger)<<"Mian Start";
    //xzmjx::Fiber::ptr fiber(new xzmjx::Fiber(fun,0));
    xzmjx::Fiber::ptr fiber( new xzmjx::Fiber(fun,0));
    XZMJX_LOG_INFO(g_logger)<<"Fiber Cotr";
    fiber->resume();
    XZMJX_LOG_INFO(g_logger)<<"fiber back";
    fiber->resume();
    XZMJX_LOG_INFO(g_logger)<<"fiber back";
    XZMJX_LOG_INFO(g_logger)<<"subfiber use_count"<<fiber.use_count();
    return 0;
}