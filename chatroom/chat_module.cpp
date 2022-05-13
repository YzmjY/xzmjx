#include "chat_module.h"

namespace chat{
ChatModule::ChatModule()
    :xzmjx::Module("chat_room","1.0",""){

}

bool ChatModule::onLoad(){
    XZMJX_LOG_INFO(g_logger)<<"Chatroom Module Load";
    return true;
}

bool ChatModule::onUnload(){
    XZMJX_LOG_INFO(g_logger)<<"Chatroom Module Unload";
    return true;
}

bool ChatModule::onServerReady(){
    XZMJX_LOG_INFO(g_logger)<<"onServerReady";
    return true;
}

bool ChatModule::onServerUp() {
    XZMJX_LOG_INFO(g_logger)<<"onServerUp";
    return true;
}
}


extern "C" {
xzmjx::Module *CreateModule() {
    xzmjx::Module *module = new chat::ChatModule();
    XZMJX_LOG_INFO(g_logger) << "CreateModule" << module->statusString();
    return module;
}


void DestoryModule(xzmjx::Module *v) {
    XZMJX_LOG_INFO(g_logger) << "DestoryModule" << v->statusString();
    delete v;
}
}