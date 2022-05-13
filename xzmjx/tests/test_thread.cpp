//
// Created by 20132 on 2022/3/11.
//
#include "thread.h"
#include "log.h"
#include <vector>
xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
xzmjx::LogAppender::ptr  g_appender(new xzmjx::FileLogAppender("./log_test.txt"));

void logThreadFun(char c){
    while(true){
        XZMJX_LOG_INFO(g_logger)<<std::string(100,c);
    }
}
int main(){
    g_logger->addAppender(g_appender);
    std::vector<xzmjx::Thread::ptr> thrs(4);
    auto cb1 = std::bind(logThreadFun,'+');
    auto cb2 = std::bind(logThreadFun,'-');
    for(int i = 0;i<2;i++){
        thrs[i*2] = std::make_shared<xzmjx::Thread>(cb1,"Thread"+std::to_string(i*2));
        thrs[i*2+1] = std::make_shared<xzmjx::Thread>(cb2,"Thread"+std::to_string(i*2+1));
    }
    for(size_t i = 0;i<thrs.size();i++){
        thrs[i]->join();
    }
    return 0;
}




