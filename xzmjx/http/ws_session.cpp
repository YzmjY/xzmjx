#include "http/ws_session.h"
#include "log.h"
#include "util.h"

static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
xzmjx::ConfigVar<uint32_t>::ptr xzmjx::http::g_websocket_msg_max_size =
    xzmjx::Config::Lookup("websockt.message.max_size", (uint32_t)1024 * 1024 * 32, "websocket message max size");

namespace xzmjx {
namespace http {
std::string WSFrameHead::toString() const {
    std::stringstream ss;
    ss << "[WSFrameHead fin=" << fin << " rsv1=" << rsv1 << " rsv2=" << rsv2 << " rsv3=" << rsv3 << " opcode=" << opcode
       << " mask=" << mask << " payload=" << payload << "]";
    return ss.str();
}

WSFrameMessage::WSFrameMessage(int opcode, const std::string& data) : m_opcode(opcode), m_data(data) {}

WSSession::WSSession(Socket::ptr sock, bool owner) : HttpSession(sock, owner) {}

HttpRequest::ptr WSSession::handleShake() {
    HttpRequest::ptr req;
    do {
        req = recvRequest();
        if (!req) {
            XZMJX_LOG_INFO(g_logger) << "invaild http request";
            break;
        }
        if (strcasecmp(req->getHeader("Upgrade").c_str(), "websocket")) {
            XZMJX_LOG_INFO(g_logger) << "http header Upgrade != websocket";
            break;
        }
        if (strcasecmp(req->getHeader("Connection").c_str(), "Upgrade")) {
            XZMJX_LOG_INFO(g_logger) << "http header Connection != Upgrade";
            break;
        }
        if (req->getHeaderAs<int>("Sec-webSocket-Version") != 13) {
            XZMJX_LOG_INFO(g_logger) << "http header Sec-webSocket-Version != 13";
            break;
        }
        std::string key = req->getHeader("Sec-WebSocket-Key");
        if (key.empty()) {
            XZMJX_LOG_INFO(g_logger) << "http header Sec-WebSocket-Key = null";
            break;
        }
        std::string v = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        std::string tmp = sha1sum(v.c_str(), v.size());
        v = base64encode(tmp.c_str(), tmp.size());
        req->setWebsocket(true);

        auto rsp = req->createResponse();
        rsp->setStatus(HttpStatus::SWITCHING_PROTOCOLS);
        rsp->setWebsocket(true);
        rsp->setReason("Web Socket Protocol Handshake");
        rsp->setHeader("Upgrade", "websocket");
        rsp->setHeader("Connection", "Upgrade");
        rsp->setHeader("Sec-WebSocket-Accept", v);
        // auto rsp = req->createResponse();
        // rsp->setStatus(xzmjx::http::HttpStatus::NOT_FOUND);
        // rsp->setHeader("Server","xzmjx/1.0.0");
        // rsp->setHeader("Content-Type","text/html");
        // rsp->setBody("<html><head><title>404 Not Found"
        //     "</title></head><body><center><h1>404 Not Found</h1></center>"
        //     "<hr><center>xzmjx-websocket</center></body></html>");
        sendResponse(rsp);
        XZMJX_LOG_DEBUG(g_logger) << *req;
        XZMJX_LOG_DEBUG(g_logger) << *rsp;
        return req;
    } while (0);
    if (req) {
        XZMJX_LOG_INFO(g_logger) << *req;
    }
    return nullptr;
}

WSFrameMessage::ptr WSSession::recvMessage() { return WSRecvMessage(this, false); }

int32_t WSSession::sendMessage(WSFrameMessage::ptr msg, bool fin) { return WSSendMessage(this, msg, false, fin); }

int32_t WSSession::sendMessage(const std::string& msg, int32_t opcode, bool fin) {
    return WSSendMessage(this, std::make_shared<WSFrameMessage>(opcode, msg), false, fin);
}

int32_t WSSession::ping() { return WSPing(this); }

int32_t WSSession::pong() { return WSPong(this); }

WSFrameMessage::ptr WSRecvMessage(Stream* stream, bool client) {
    // 接收Websocket协议数据
    int opcode = 0;
    std::string data;
    int cur_len = 0;
    do {
        // 协议头
        WSFrameHead ws_head;
        if (stream->readFixedSize(&ws_head, sizeof(ws_head)) <= 0) {
            break;
        }
        XZMJX_LOG_DEBUG(g_logger) << "WSFrameHead " << ws_head.toString();

        // 依据协议头，不同处理
        if (ws_head.opcode == WSFrameHead::kPing) {
            XZMJX_LOG_INFO(g_logger) << "PING";
            if (WSPong(stream) <= 0) {
                break;
            }
        } else if (ws_head.opcode == WSFrameHead::kPong) {
        } else if (ws_head.opcode == WSFrameHead::kBin_Frame || ws_head.opcode == WSFrameHead::kText_Frame ||
                   ws_head.opcode == WSFrameHead::kContinue) {
            if (!client && !ws_head.mask) {
                // 客户端发往服务端的报文必须mask置一
                XZMJX_LOG_INFO(g_logger) << "WSFrameHead Mask != 1";
                break;
            }
            // 计算payload长度
            uint64_t length = 0;
            if (ws_head.payload == 126) {
                uint16_t len = 0;
                if (stream->readFixedSize(&len, sizeof(len)) <= 0) {
                    break;
                }
                length = EndianCastOnLittle(len);
            } else if (ws_head.payload == 127) {
                uint64_t len = 0;
                if (stream->readFixedSize(&len, sizeof(len)) <= 0) {
                    break;
                }
                length = EndianCastOnLittle(len);
            } else {
                length = ws_head.payload;
            }
            if ((cur_len + length) >= g_websocket_msg_max_size->getValue()) {
                XZMJX_LOG_WARN(g_logger) << "WSFrameMessage length > " << g_websocket_msg_max_size->getValue();
                break;
            }
            char mask[4];
            if (ws_head.mask) {
                if (stream->readFixedSize(mask, sizeof(mask)) <= 0) {
                    break;
                }
            }

            data.resize(cur_len + length);
            if (stream->readFixedSize(&data[0] + cur_len, length) <= 0) {
                break;
            }
            if (ws_head.mask) {
                for (int i = 0; i < (int)length; i++) {
                    data[cur_len + i] ^= mask[i % 4];
                }
            }
            cur_len += length;
            if (!opcode && ws_head.opcode != WSFrameHead::kContinue) {
                opcode = ws_head.opcode;
            }
            if (ws_head.fin) {
                XZMJX_LOG_DEBUG(g_logger) << data;
                return WSFrameMessage::ptr(new WSFrameMessage(opcode, std::move(data)));
            }
        } else {
        }
    } while (true);
    stream->close();
    return nullptr;
}

int32_t WSSendMessage(Stream* stream, WSFrameMessage::ptr msg, bool client, bool fin) {
    do {
        WSFrameHead head;
        memset(&head, 0, sizeof(head));
        head.fin = fin;
        head.opcode = msg->getOpcode();
        head.mask = client;
        uint64_t size = msg->getData().size();
        if (size < 126) {
            head.payload = size;
        } else if (size < 0xff) {
            head.payload = 126;
        } else {
            head.payload = 127;
        }

        if (stream->writeFixedSize(&head, sizeof(head)) <= 0) {
            break;
        }
        if (head.payload == 126) {
            uint16_t len = size;
            len = EndianCastOnLittle(len);
            if (stream->writeFixedSize(&len, sizeof(len)) <= 0) {
                break;
            }
        } else if (head.payload == 127) {
            uint64_t len = size;
            len = EndianCastOnLittle(len);
            if (stream->writeFixedSize(&len, sizeof(len)) <= 0) {
                break;
            }
        }
        if (client) {
            char mask[4];
            uint32_t rand_value = rand();
            memcpy(mask, &rand_value, sizeof(mask));
            std::string data = msg->getData();
            for (size_t i = 0; i < data.size(); ++i) {
                data[i] ^= mask[i % 4];
            }

            if (stream->writeFixedSize(mask, sizeof(mask)) <= 0) {
                break;
            }
        }
        if (stream->writeFixedSize(msg->getData().c_str(), size) <= 0) {
            break;
        }
        return size + sizeof(head);
    } while (0);
    stream->close();
    return -1;
}

int32_t WSPing(Stream* stream) {
    WSFrameHead ws_head;
    memset(&ws_head, 0, sizeof(ws_head));
    ws_head.fin = 1;
    ws_head.opcode = WSFrameHead::kPing;
    int32_t rt = stream->writeFixedSize(&ws_head, sizeof(ws_head));
    if (rt <= 0) {
        stream->close();
    }
    return rt;
}

int32_t WSPong(Stream* stream) {
    WSFrameHead ws_head;
    memset(&ws_head, 0, sizeof(ws_head));
    ws_head.fin = 1;
    ws_head.opcode = WSFrameHead::kPong;
    int32_t rt = stream->writeFixedSize(&ws_head, sizeof(ws_head));
    if (rt <= 0) {
        stream->close();
    }
    return rt;
}
} // namespace http
} // namespace xzmjx