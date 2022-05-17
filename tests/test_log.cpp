#include "log.h"
#include "config.h"
#include "env.h"
#include "thread.h"
#include <iostream>
#include <vector>
#include <algorithm>

void cb1(xzmjx::Logger::ptr logger){
    while(true){
        XZMJX_LOG_ERROR(logger)<<"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
        sleep(1);
    }
}

void cb2(xzmjx::Logger::ptr logger){
    while(true){
        XZMJX_LOG_ERROR(logger)<<"----------------------------------------------------------------------------------------------------------------------------";
        sleep(1);
    }
}
int main(int argc,char**argv){
    xzmjx::EnvMgr::GetInstance()->init(argc,argv);
    std::string path = xzmjx::EnvMgr::GetInstance()->getConfigPath();
    xzmjx::Config::LoadFromConfDir(path);
    xzmjx::Logger::ptr logger_1 = XZMJX_LOG_NAME("thr1");
    xzmjx::Logger::ptr logger_2 = XZMJX_LOG_NAME("thr2");

    // sleep放大竞争，可以看到其不安全
    xzmjx::Thread::ptr t1(new xzmjx::Thread(std::bind(cb1,logger_1),"Thread1"));
    sleep(1);
    xzmjx::Thread::ptr t2(new xzmjx::Thread(std::bind(cb2,logger_2),"Thread2"));
    t1->join();
    t2->join();
    return 0;
}