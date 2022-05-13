#ifndef XZMJX_CHAT_PROTOCOL_H
#define XZMJX_CHAT_PROTOCOL_H
#include <memory>
#include <map>

//1. login协议。
//Client: {type: "login_request", "name": "昵称"}
//Server: {type: "login_response", result: 200, msg: "ok"}
//
//2. send协议。
//Client：{type: "send_request", "msg" : "消息"}
//Server: {type: "send_response", result: 200, msg: "ok"}
//
//3. user_enter 通知
//        {type: "user_enter", msg: "xxx 加入聊天室", time:xxx}
//
//4. user_leave 通知
//        {type: "user_leave", msg: "xxx 离开聊天室", time:xxx}
//
//5. msg 通知
//        {type: "msg", msg: "具体聊天信息", user: "xxx", time: xxxx}
namespace chat{
class ChatMessage{
public:
    typedef std::shared_ptr<ChatMessage> ptr;
    ChatMessage();
    static ChatMessage::ptr Create(const std::string& msg);
    std::string get(const std::string& id) const;
    void set(const std::string& id,const std::string& val);
private:
    std::map<std::string,std::string> m_data;
};
}

#endif