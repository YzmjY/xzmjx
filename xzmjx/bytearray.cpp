//
// Created by 20132 on 2022/4/16.
//
#include "bytearray.h"
#include "log.h"
#include <cstring>
#include <cmath>
#include <exception>

namespace xzmjx {
namespace {
xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
}
ByteArray::Node::Node(size_t s) : ptr(new char[s]), next(nullptr), size(s) {}

ByteArray::Node::Node() : ptr(nullptr), next(nullptr), size(0) {}

ByteArray::Node::~Node() {
    if (ptr) {
        delete[] ptr;
        size = 0;
        next = nullptr;
    }
}
ByteArray::ByteArray(size_t base_size)
    : m_position(0),
      m_base_size(base_size),
      m_capacity(base_size),
      m_size(0),
      m_root(new Node(base_size)),
      m_cur(m_root),
      m_endian(XZMJX_Big_Endian) {}

ByteArray::~ByteArray() {
    Node* temp = m_root;
    while (temp) {
        m_cur = temp;
        temp = temp->next;
        delete m_cur;
    }
}

void ByteArray::writeFint8(const int8_t& value) { write(&value, sizeof(value)); }

void ByteArray::writeFuint8(const uint8_t& value) { write(&value, sizeof(value)); }

void ByteArray::writeFint16(const int16_t& value) {
    int16_t v = EndianCastByType(value, m_endian);
    write(&v, sizeof(v));
}

void ByteArray::writeFuint16(const uint16_t& value) {
    uint16_t v = EndianCastByType(value, m_endian);
    write(&v, sizeof(v));
}

void ByteArray::writeFint32(const int32_t& value) {
    int32_t v = EndianCastByType(value, m_endian);
    write(&v, sizeof(v));
}

void ByteArray::writeFuint32(const uint32_t& value) {
    uint32_t v = EndianCastByType(value, m_endian);
    write(&v, sizeof(v));
}

void ByteArray::writeFint64(const int64_t& value) {
    int64_t v = EndianCastByType(value, m_endian);
    write(&v, sizeof(v));
}

void ByteArray::writeFuint64(const uint64_t& value) {
    uint64_t v = EndianCastByType(value, m_endian);
    write(&v, sizeof(v));
}

static uint32_t EncodeZigzag32(const int32_t& v) {
    if (v < 0) {
        return ((uint32_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

static uint64_t EncodeZigzag64(const int64_t& v) {
    if (v < 0) {
        return ((uint64_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

static int32_t DecodeZigzag32(const uint32_t& v) { return (v >> 1) ^ (-(v & 1)); }

static int64_t DecodeZigzag64(const uint64_t& v) { return (v >> 1) ^ (-(v & 1)); }
void ByteArray::writeInt32(int32_t value) { writeUint32(EncodeZigzag32(value)); }

void ByteArray::writeUint32(uint32_t value) {
    uint8_t tmp[5]; /// 至多被编码为五个字节
    uint8_t i = 0;
    while (value >= 0x80) {
        tmp[i] = (value & 0x7F) | 0x80; /// 低七位+标识自身不是最后一个字节的1
        value >>= 7;
        i++;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeInt64(int64_t value) { writeUint64(EncodeZigzag64(value)); }

void ByteArray::writeUint64(uint64_t value) {
    uint8_t tmp[10]; /// 至多被编码为五个字节
    uint8_t i = 0;
    while (value >= 0x80) {
        tmp[i] = (value & 0x7F) | 0x80; /// 低七位+标识自身不是最后一个字节的1
        value >>= 7;
        i++;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeFloat(const float& value) {
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint32(v);
}

void ByteArray::writeDouble(const double& value) {
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint64(v);
}

void ByteArray::writeStringF16(const std::string& s) {
    writeFuint16(s.size());
    write(s.c_str(), s.size());
}

void ByteArray::writeStringF32(const std::string& s) {
    writeFuint32(s.size());
    write(s.c_str(), s.size());
}

void ByteArray::writeStringF64(const std::string& s) {
    writeFuint64(s.size());
    write(s.c_str(), s.size());
}
void ByteArray::writeStringVint(const std::string& s) {
    writeUint64(s.size()); /// zigzag压缩长度
    write(s.c_str(), s.size());
}
int8_t ByteArray::readFint8() {
    int8_t ans;
    read(&ans, sizeof(ans));
    return ans;
}

uint8_t ByteArray::readFuint8() {
    uint8_t ans;
    read(&ans, sizeof(ans));
    return ans;
}

int16_t ByteArray::readFint16() {
    int16_t ans;
    read(&ans, sizeof(ans));
    return EndianCastByType(ans, m_endian);
}

uint16_t ByteArray::readFuint16() {
    uint16_t ans;
    read(&ans, sizeof(ans));
    return EndianCastByType(ans, m_endian);
}

int32_t ByteArray::readFint32() {
    int32_t ans;
    read(&ans, sizeof(ans));
    return EndianCastByType(ans, m_endian);
}

uint32_t ByteArray::readFuint32() {
    uint32_t ans;
    read(&ans, sizeof(ans));
    return EndianCastByType(ans, m_endian);
}

int64_t ByteArray::readFint64() {
    int64_t ans;
    read(&ans, sizeof(ans));
    return EndianCastByType(ans, m_endian);
}

uint64_t ByteArray::readFuint64() {
    uint64_t ans;
    read(&ans, sizeof(ans));
    return EndianCastByType(ans, m_endian);
}

int32_t ByteArray::readInt32() { return DecodeZigzag32(readUint32()); }

uint32_t ByteArray::readUint32() {
    /// 每个字节取后七位，判断字节最高位是否为1
    uint32_t ans = 0;
    for (int i = 0; i < 32; i += 7) {
        uint8_t b = readFuint8();
        if (b & 0x80) {
            ans |= ((uint32_t)(b & 0x7F)) << i;
            break; /// 当前为此次编码的最后一组
        } else {
            ans |= ((uint32_t)b) << i;
        }
    }
    return ans;
}

int64_t ByteArray::readInt64() { return DecodeZigzag64(readUint64()); }

uint64_t ByteArray::readUint64() {
    /// 每个字节取后七位，判断字节最高位是否为1
    uint64_t ans = 0;
    for (int i = 0; i < 64; i += 7) {
        uint8_t b = readFuint8();
        if (b & 0x80) {
            ans |= (((uint64_t)(b & 0x7F)) << i);
        } else {
            ans |= ((uint64_t)b) << i;
            break; /// 当前为此次编码的最后一组
        }
    }
    return ans;
}

float ByteArray::readFloat() {
    uint32_t v = readFuint32();
    float ans;
    memcpy(&ans, &v, sizeof(ans));
    return ans;
}

double ByteArray::readDouble() {
    uint64_t v = readFuint64();
    double ans;
    memcpy(&ans, &v, sizeof(ans));
    return ans;
}

std::string ByteArray::readStringF16() {
    uint16_t len = readFint16();
    std::string s;
    s.resize(len);
    read(&s[0], len);
    return s;
}

std::string ByteArray::readStringF32() {
    uint32_t len = readFint32();
    std::string s;
    s.resize(len);
    read(&s[0], len);
    return s;
}

std::string ByteArray::readStringF64() {
    uint64_t len = readFint16();
    std::string s;
    s.resize(len);
    read(&s[0], len);
    return s;
}

std::string ByteArray::readStringVint() {
    uint64_t len = readUint64();
    std::string s;
    s.resize(len);
    read(&s[0], len);
    return s;
}

void ByteArray::clear() {
    m_position = m_size = 0;
    m_capacity = m_base_size;
    Node* next = m_root->next;
    while (next) {
        Node* temp = next->next;
        delete next;
        next = temp;
    }
    m_cur = m_root;
    m_root->next = nullptr;
}

void ByteArray::write(const void* buf, size_t size) {
    if (size == 0) {
        return;
    }
    addCapacity(size);

    size_t npos = m_position % m_base_size; /// 操作位置在每一个node里的偏移
    size_t ncap = m_cur->size - npos;       /// 当前node剩余的容量
    size_t bpos = 0;                        /// 已写容量

    while (size > 0) {
        if (ncap >= size) {
            /// 于当前节点结束
            memcpy(m_cur->ptr + npos, (const char*)buf + bpos, size);
            if (m_cur->size == (npos + size)) {
                /// 当前节点用完，下一个操作位置是下一个节点
                m_cur = m_cur->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy(m_cur->ptr + npos, (const char*)buf + bpos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            ncap = m_cur->size;
            npos = 0;
        }
    }
    if (m_position > m_size) {
        m_size = m_position;
    }
}

void ByteArray::read(void* buf, size_t size) {
    if (size > getReadSize()) {
        throw std::out_of_range("not enough data to read");
    }

    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;

    while (size > 0) {
        if (ncap >= size) {
            memcpy((char*)buf + bpos, m_cur->ptr + npos, size);
            if (m_cur->size == size) {
                m_cur = m_cur->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bpos, m_cur->ptr + npos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            npos = 0;
            m_cur = m_cur->next;
            ncap = m_cur->size;
        }
    }
}

void ByteArray::read(void* buf, size_t size, size_t position) const {
    if (size > m_size - position) {
        throw std::out_of_range("not enough data to read");
    }

    size_t npos = position % m_base_size;

    uint64_t node_count = position / m_base_size;
    Node* cur = m_root;
    while (node_count > 0) {
        cur = cur->next;
        --node_count;
    }
    size_t ncap = cur->size - npos;
    size_t bpos = 0;

    while (size > 0) {
        if (ncap >= size) {
            memcpy((char*)buf + bpos, cur->ptr + npos, size);
            bpos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bpos, cur->ptr + npos, ncap);
            bpos += ncap;
            size -= ncap;
            npos = 0;
            cur = cur->next;
            ncap = cur->size;
        }
    }
}

void ByteArray::setPosition(size_t pos) {
    if (pos > m_capacity) {
        throw std::out_of_range("setPostion out of range");
    }
    m_position = pos;
    if (m_position > m_size) {
        m_size = m_position;
    }
    m_cur = m_root;
    while (pos > m_cur->size) {
        m_cur = m_cur->next;
        pos -= m_cur->size;
    }
    /// size_t 为无符号整型，上面不能>=
    if (pos == m_cur->size) {
        m_cur = m_cur->next;
    }
}

bool ByteArray::writeToFile(const std::string& name) const {
    ///[m_position,m_size)dump到文件中
    std::ofstream ofs;
    ofs.open(name, std::ios::trunc | std::ios::binary);
    if (!ofs) {
        XZMJX_LOG_ERROR(g_logger) << "writeToFile name=" << name << " error, errno=" << errno
                                  << " errstr=" << strerror(errno);
        return false;
    }

    uint64_t read_size = getReadSize();
    size_t pos = m_position;
    Node* cur = m_cur;

    while (read_size > 0) {
        size_t diff = pos % m_base_size; /// 当前操作位置在node中的下标
        size_t len = (m_base_size - diff >= read_size) ? read_size : m_base_size - diff;
        ofs.write(cur->ptr + diff, (std::streamsize)len);
        cur = cur->next;
        pos += len;
        read_size -= len;
    }
    return true;
}

bool ByteArray::readFromFile(const std::string& name) {
    std::ifstream ifs;
    ifs.open(name, std::ios::binary);
    if (!ifs) {
        XZMJX_LOG_ERROR(g_logger) << "readFromFile name=" << name << " error, errno=" << errno
                                  << " errstr=" << strerror(errno);
        return false;
    }

    std::shared_ptr<char> buff(new char[m_base_size], [](char* ptr) { delete[] ptr; });
    while (!ifs.eof()) {
        ifs.read(buff.get(), (std::streamsize)m_base_size);
        write(buff.get(), ifs.gcount());
    }
    return true;
}

void ByteArray::addCapacity(size_t size) {
    if (size == 0) {
        return;
    }
    size_t old_cap = getCapacity(); /// 当前剩余可写
    if (old_cap >= size) {
        return;
    }
    size = size - old_cap; /// 待增长的大小
    size_t node_count = ceil((1.0 * size) / m_base_size);
    Node* temp = m_root;
    while (temp->next) {
        temp = temp->next;
    }
    Node* first = nullptr;
    for (size_t i = 0; i < node_count; i++) {
        temp->next = new Node(m_base_size);
        if (first == nullptr) {
            first = temp->next;
        }
        temp = temp->next;
        m_capacity += m_base_size;
    }
    if (old_cap == 0) {
        m_cur = first;
    }
}

std::string ByteArray::toString() const {
    std::string str;
    str.resize(getReadSize());
    read(&str[0], str.size(), m_position);
    return str;
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const {
    len = len > getReadSize() ? getReadSize() : len;
    if (len == 0) {
        return 0;
    }
    buffers.clear();
    uint64_t size = len;
    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    Node* cur = m_cur;
    while (len > 0) {
        if (ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const {
    len = len > (m_size - position) ? (m_size - position) : len;
    if (len == 0) {
        return 0;
    }
    buffers.clear();
    uint64_t size = len;
    size_t npos = position % m_base_size;
    size_t node_count = position / m_base_size;
    Node* cur = m_root;
    while (node_count > 0) {
        cur = cur->next;
        --node_count;
    }

    size_t ncap = cur->size - npos;
    struct iovec iov;
    while (len > 0) {
        if (ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len) {
    if (len == 0) {
        return 0;
    }
    addCapacity(len);
    uint64_t size = len;
    buffers.clear();
    size_t npos = m_position % m_base_size;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    Node* cur = m_cur;
    while (len > 0) {
        if (ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}
} // namespace xzmjx
