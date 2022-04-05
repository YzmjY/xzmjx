//
// Created by 20132 on 2022/3/30.
//
#include "timer.h"
#include "log.h"
#include "iomanager.h"

static xzmjx::Logger::ptr g_logger = XZMJX_LOG_ROOT();

int main(){
    /**
    xzmjx::TimerManager manager;
    manager.addTimer(5*1000,[](){ XZMJX_LOG_INFO(g_logger)<<"Timer：5s";},true);
    manager.addTimer(1*1000,[](){ XZMJX_LOG_INFO(g_logger)<<"Timer：1s";}, true);

    while(true){
        sleep(1);
        std::vector<std::function<void()>> cbs;
        manager.listExpiredCb(cbs);
        for(size_t i = 0;i<cbs.size();i++){
            cbs[i]();
        }
    }
     **/

    xzmjx::IOManager::ptr iom(new xzmjx::IOManager(1));
    iom->addTimerEvent(3*1000,[](){
        XZMJX_LOG_INFO(g_logger)<<"Timer 3s";});
    iom->addTimerEvent(5*1000,[](){
        XZMJX_LOG_INFO(g_logger)<<"Timer 5s";
    });

    return 0;
}

