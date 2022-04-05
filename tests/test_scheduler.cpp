//
// Created by 20132 on 2022/3/23.
//
#include "scheduler.h"
#include "log.h"
static xzmjx::Logger::ptr g_logger = XZMJX_LOG_ROOT();
 void fun(){
    int i = 0;
    while(i++<10){
        sleep(5);
        XZMJX_LOG_INFO(g_logger)<<"fiber run";
        xzmjx::Fiber::YieldToReady();
    }
}

int main(){
    XZMJX_LOG_INFO(g_logger)<<"main start";
    g_logger->setLevel(xzmjx::LogLevel::INFO);
    xzmjx::Scheduler sc;
    sc.submit(fun,-1);
    sc.submit(fun,-1);
    sc.submit(fun,-1);
    sc.start();
    sc.stop();
    return 0;
}

