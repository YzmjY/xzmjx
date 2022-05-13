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


#endif