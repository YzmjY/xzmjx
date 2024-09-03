#include "ds/bloomfilter.h"

namespace xzmjx {
namespace ds {

BloomFilter::BloomFilter(int bitsPerKey, int n) : m_bitsPerKey(bitsPerKey) {
    // bitsPerKey的含义：布隆过滤器共m
    // bits，需要存储n个数据，则每个key可以占bitsPerKey = m/n
    // 数学证明0.69*m/n个hash函数可以获得最小的碰撞
    m_hashCnt = static_cast<size_t>(m_bitsPerKey * 0.69);
    if (m_hashCnt < 1) {
        m_hashCnt = 1;
    }
    if (m_hashCnt > 30) {
        m_hashCnt = 30;
    }
    m_size = n * m_bitsPerKey;
    m_data = std::make_shared<Bitmap>(m_size);
}

void BloomFilter::add(const std::string& key) {
    uint32_t h = hash(key);
    const uint32_t delta = (h >> 17) | (h << 15); // Rotate right 17 bits
    for (size_t j = 0; j < m_hashCnt; j++) {
        const uint32_t bitpos = h % m_size;
        m_data->set(bitpos, true);
        h += delta;
    }
}

bool BloomFilter::mayMatch(const std::string& key) {
    uint32_t h = hash(key);
    const uint32_t delta = (h >> 17) | (h << 15); // Rotate right 17 bits
    for (size_t j = 0; j < m_hashCnt; j++) {
        const uint32_t bitpos = h % m_size;
        if (m_data->get(bitpos)) {
            return false;
        }
        h += delta;
    }
    return true;
}

} // namespace ds
} // namespace xzmjx