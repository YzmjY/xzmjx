#include "env.h"
#include "log.h"

int main(int argc,char**argv){
    xzmjx::EnvMgr::GetInstance()->init(argc,argv);
    xzmjx::EnvMgr::GetInstance()->printArg();

    std::string s = xzmjx::EnvMgr::GetInstance()->getConfigPath();
    XZMJX_LOG_INFO(XZMJX_LOG_ROOT())<<s;
    return 0;
}