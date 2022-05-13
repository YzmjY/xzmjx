#include "daemon.h"
#include "log.h"
#include "config.h"

xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
int main_cb(int argc,char**argv){
    while(true){
        XZMJX_LOG_INFO(g_logger)<<"ping";
        sleep(1);
    }
    return 0;
}

int main(int argc,char**argv){
    YAML::Node n = YAML::LoadFile("/mnt/e/01-Code/xzmjx/xzmjx/bin/test/log.yml");
    xzmjx::Config::LoadFromYaml(n);
    return xzmjx::start_daemon(argc,argv,main_cb,true);
}