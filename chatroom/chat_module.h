#ifndef XZMJX_CHAT_MODULE_H
#define XZMJX_CHAT_MODULE_H

#include "module.h"
#include "log.h"
static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
namespace chat {
class ChatModule : public xzmjx::Module {
public:
    typedef std::shared_ptr<ChatModule> ptr;

    ChatModule();

    bool onLoad() override;

    bool onUnload() override;

    bool onServerReady() override;

    bool onServerUp() override;
};
}

extern "C" {
xzmjx::Module *CreateModule() {
    xzmjx::Module *module = new chat::ChatModule();
    XZMJX_LOG_INFO(g_logger)<<"CreateModule"<<module->statusString();
    return module;
}


void DestoryModule(xzmjx::Module* v){
    XZMJX_LOG_INFO(g_logger)<<"DestoryModule"<<v->statusString();
    delete v;
}
}

#endif