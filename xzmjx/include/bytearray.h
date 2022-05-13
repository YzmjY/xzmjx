//
// Created by 20132 on 2022/4/16.
//

#ifndef XZMJX_BYTEARRAY_H
#define XZMJX_BYTEARRAY_H
#include <memory>
#include <string>
#include <stdint.h>
#include <sys/socket.h>
#include "util.h"
namespace xzmjx{
class ByteArray{
public:
    typedef std::shared_ptr<ByteArray> ptr;

    struct Node{
        Node(size_t s);
        Node();
        ~Node();

        char* ptr;
        Node* next;
        size_t size;
    };

    ByteArray(size_t base_size = 4096);
    ~ByteArray();

    ///定长
    void writeFint8(const int8_t& value);
    void writeFuint8(const uint8_t& value);
    void writeFint16(const int16_t& value);
    void writeFuint16(const uint16_t& value);
    void writeFint32(const int32_t& value);
    void writeFuint32(const uint32_t& value);
    void writeFint64(const int64_t& value);
    void writeFuint64(const uint64_t& value);

    ///变长，压缩
    void writeInt32( int32_t value);
    void writeUint32(uint32_t value);
    void writeInt64(int64_t value);
    void writeUint64(uint64_t value);

    void writeFloat(const float & value);
    void writeDouble(const double & value);

    ///length(fixed/varint)+data
    void writeStringF16(const std::string& s);
    void writeStringF32(const std::string& s);
    void writeStringF64(const std::string& s);
    void writeStringVint(const std::string& s);

    ///read
    int8_t   readFint8();
    uint8_t  readFuint8();
    int16_t  readFint16();
    uint16_t readFuint16();
    int32_t  readFint32();
    uint32_t readFuint32();
    int64_t  readFint64();
    uint64_t readFuint64();

    int32_t  readInt32();
    uint32_t readUint32();
    int64_t  readInt64();
    uint64_t readUint64();

    float  readFloat();
    double readDouble();

    ///length(fixed/varint)+data
    std::string readStringF16();
    std::string readStringF32();
    std::string readStringF64();
    std::string readStringVint();

    void clear();
    void write(const void* buf,size_t size);
    void read(void* buf,size_t size);
    void read(void* buf,size_t size,size_t position) const;
    size_t getPosition() const {return m_position;};
    void setPosition(size_t pos);

    bool writeToFile(const std::string& name) const;
    bool readFromFile(const std::string& name);

    size_t getBaseSize() const{return m_base_size;}
    size_t getReadSize() const{return m_size - m_position;}

    bool isLittleEndain() const{return m_endian == XZMJX_Little_Endian;}
    void setLittleEndain(){m_endian = XZMJX_Little_Endian;}

    size_t getSize() const { return m_size;}

    std::string toString() const;

    uint64_t getReadBuffers(std::vector<iovec>& buffers,uint64_t len = ~0ull) const;
    uint64_t getReadBuffers(std::vector<iovec>& buffers,uint64_t len,uint64_t postion) const;
    uint64_t getWriteBuffers(std::vector<iovec>& buffers,uint64_t len);

private:
    void addCapacity(size_t size);
    size_t getCapacity() const{return m_capacity - m_position;}///@TODO: 不应该是总量减去已用的是当前剩余的capacity吗
private:
    size_t m_position;  ///当前操作的位置
    size_t m_base_size; ///每一个node的大小
    size_t m_capacity;  ///总容量
    size_t m_size;      ///目前的数据量
    Node* m_root;
    Node* m_cur;
    EndianType m_endian;///bytearray中数据的字节序，默认为网络字节序，即大端。
};
}

#endif //XZMJX_BYTEARRAY_H
