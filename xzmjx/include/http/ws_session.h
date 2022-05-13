#ifndef XZMJX_WS_SESSION_H
#define XZMJX_WS_SESSION_H
#include "config.h"
#include "http/http_session.h"

namespace xzmjx{
namespace http{

// Websocket协议封装
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-------+-+-------------+-------------------------------+
// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
// | |1|2|3|       |K|             |                               |
// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
// |     Extended payload length continued, if payload len == 127  |
// + - - - - - - - - - - - - - - - +-------------------------------+
// |                               |Masking-key, if MASK set to 1  |
// +-------------------------------+-------------------------------+
// | Masking-key (continued)       |          Payload Data         |
// +-------------------------------- - - - - - - - - - - - - - - - +
// :                     Payload Data continued ...                :
// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
// |                     Payload Data continued ...                |
// +---------------------------------------------------------------+
#pragma pack(1)
struct WSFrameHead{
    enum OPCODE{
        kContinue = 0,
        kText_Frame = 1,
        kBin_Frame = 2,
        kClose = 8,
        kPing = 0x9,
        kPong = 0xA
    };
    uint32_t opcode:4;
    bool rsv3:1;
    bool rsv2:1;
    bool rsv1:1;
    bool fin:1;
    uint32_t payload:7;
    bool mask:1;

    std::string toString() const;
};
#pragma pack()

/**
 * @brief Websocket协议帧封装
 * 
 */
class WSFrameMessage{
public:
    typedef std::shared_ptr<WSFrameMessage> ptr;
    WSFrameMessage(int opcode = 0,const std::string& data = "");

    int getOpcode() const { return m_opcode; }
    void setOpcode(int v) { m_opcode = v; }

    const std::string& getData() const { return m_data; }
    std::string getData() { return m_data; }
    void setData(const std::string& d) { m_data = d; }
 
private:
    int m_opcode;
    std::string m_data;
};

class WSSession : public HttpSession {
public: 
    typedef std::shared_ptr<WSSession> ptr;
    WSSession(Socket::ptr sock,bool owner = true);

    // Websocket使用http进行握手
    HttpRequest::ptr handleShake();

    WSFrameMessage::ptr recvMessage();
    int32_t sendMessage(WSFrameMessage::ptr msg,bool fin = true);
    int32_t sendMessage(const std::string& msg,int32_t opcode = WSFrameHead::kText_Frame,bool fin = true);
    int32_t ping();
    int32_t pong();
private:
    bool handleServerShake();
    bool handleClientShake();
};

extern xzmjx::ConfigVar<uint32_t>::ptr g_websocket_msg_max_size;
WSFrameMessage::ptr WSRecvMessage(Stream* stream,bool client);
int32_t WSSendMessage(Stream* stream,WSFrameMessage::ptr msg,bool client,bool fin);
int32_t WSPing(Stream* stream);
int32_t WSPong(Stream* stream);
}
}


#endif