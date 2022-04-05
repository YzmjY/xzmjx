//
// Created by 20132 on 2022/4/2.
//
#include "log.h"
#include "hook.h"
#include "iomanager.h"
namespace {
    xzmjx::Logger::ptr g_logger = XZMJX_LOG_ROOT();
}
void test_hook(){
    xzmjx::IOManager iom(1);
    XZMJX_LOG_DEBUG(g_logger)<<"submit 1";
    iom.submit([](){
        sleep(2);
        XZMJX_LOG_ERROR(g_logger)<<"Sleep 2";
    });

    XZMJX_LOG_DEBUG(g_logger)<<"submit 2";
    iom.submit([](){
        sleep(3);
        XZMJX_LOG_ERROR(g_logger)<<"Sleep 3";
    });
}
int main(){
    g_logger->setLevel(xzmjx::LogLevel::ERROR);
    test_hook();

}
