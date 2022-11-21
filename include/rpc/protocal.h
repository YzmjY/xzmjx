#ifndef XZMJX_RPC_PROTOCAL
#define XZMJX_RPC_PROTOCAL
#include <memory>

namespace xzmjx {
namespace rpc {
class Protocol {
public:
    enum class Type{

    };
    //|id|version|type|length|payload|checksum
public:
    using ptr = std::shared_ptr<Protocol>;
    Protocol() = default;
    ~Protocol() = default;
    static Protocol::ptr Create() {
        Protocol::ptr p = std::make_shared<Protocol>();

        return p;
    }

    void setID(uint64_t id);
    void setVersion(uint8_t v);
    void setType(Type type);
    void setLength(uint64_t len);
    void setContent(const std::string& data);

    uint64_t getID() const;
    uint8_t getVersion() const;
    Type getType() const;
    uint64_t getLength() const;
    std::string getContent() const;

    std::string toString() const;

private:
    uint64_t m_id = 0;
    uint8_t m_version = 0x10;
    Type m_type;
    uint64_t m_length = 0;
    uint32_t m_crc = 0;
    std::string m_data;
};
}
}


#endif