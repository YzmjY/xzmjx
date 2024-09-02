#ifndef XZMJX_DS_BLOOMFILTER_H_
#define XZMJX_DS_BLOOMFILTER_H_

#include <memory>
#include "ds/bitmap.h"
namespace xzmjx{
namespace ds{

class BloomFilter {
public:
    typedef std::shared_ptr<BloomFilter> ptr;

private:
    uint32_t hash(const std::string& s) {
        return (uint32_t)std::hash<std::string>()(s);
    }

public:
    explicit BloomFilter(int bitsPerKey, int n);
    ~BloomFilter() = default;

public:
    void add(const std::string& key);
    bool mayMatch(const std::string& key); 


private:
    uint32_t m_bitsPerKey;
    size_t m_hashCnt;
    uint32_t m_size;
    Bitmap::ptr m_data;
};
} // namespace ds
} // namespace xzmjx





#endif