#ifndef XZMJX_RPC_SERIALIZER_H
#define XZMJX_RPC_SERIALIZER_H
#include <memory>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "bytearray.h"

namespace xzmjx{
namespace rpc{
class Serializer {
public:
    using ptr = std::shared_ptr<Serializer>;
    Serializer() {
        m_byte_array = std::make_shared<ByteArray>();
    }
    
    Serializer(const std::string& data) {
        m_byte_array = std::make_shared<ByteArray>();
        m_byte_array->write(data.c_str(),data.size());
    }

    Serializer(const ByteArray::ptr& ba) {
        m_byte_array = ba;
    }
    
    ~Serializer() = default;


    void clear() {
        m_byte_array->clear();
    }

    void skip(size_t count) {
        uint64_t offset = m_byte_array->getPosition();
        m_byte_array->setPosition(offset+count);
    }

    std::string toString() const {
        return m_byte_array->toString();
    }

    template<typename U>
    void serialize(U& data);

    template<typename U>
    U deserialize();


    template<>
    void serialize(bool& data) {
        m_byte_array->writeFint8(data);
    }

    template<>
    bool deserialize() {
        uint8_t res = m_byte_array->readFint8();
        return static_cast<bool>(res);
    }

    template<>
    void serialize(double& data) {
        m_byte_array->writeDouble(data);
    }

    template<>
    double deserialize() {
        return m_byte_array->readDouble();
    }

    template<>
    void serialize(float& data) {
        m_byte_array->writeFloat(data);
    }

    template<>
    float deserialize() {
        return m_byte_array->readFloat();
    }

    template<>
    void serialize(std::string& data) {
        m_byte_array->writeStringVint(data);
    }

    template<>
    std::string deserialize() {
        return m_byte_array->readStringVint();
    }

    template<>
    void serialize(uint8_t& data) {
        m_byte_array->writeFint8(data);
    }

    template<>
    uint8_t deserialize() {
        return m_byte_array->readFint8();
    }

    template<>
    void serialize(uint16_t& data) {

    }

    template<>
    void serialize(uint32_t& data) {

    }

    template<>
    void serialize(uint64_t& data) {

    }

    template<>
    void serialize(int8_t& data) {

    }

    template<>
    void serialize(int16_t& data) {

    }

    template<>
    void serialize(int32_t& data) {

    }

    template<>
    void serialize(int64_t& data) {

    }

    template<class T>
    void serialize(std::vector<T>& data) {

    }

    template<class T>
    void serialize(std::list<T>& data) {

    }

    template<class T>
    void serialize(std::set<T>& data) {

    }

    template<class T>
    void serialize(std::unordered_set<T>& data) {

    }

    template<class K,class V>
    void serialize(std::map<K,V>& data) {

    }

    template<class K,class V>
    void serialize(std::unordered_map<K,V>& data) {

    }

    template<class T, size_t N>
    void serialize(std::array<T,N>& data) {

    }

    template<class T, size_t N>
    void serialize(T(&data)[N]) {

    }

    template<class T>
    void serialize(T(&data)[]) {

    }
private:
    ByteArray::ptr m_byte_array;
};
}
}


#endif