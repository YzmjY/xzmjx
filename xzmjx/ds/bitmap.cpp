#include "ds/bitmap.h"
#include <cmath>
#include <stdlib.h>
#include <string.h>
#include <sstream>

namespace xzmjx{
namespace ds {

Bitmap::base_type Bitmap::POS[sizeof(base_type) * 8];
Bitmap::base_type Bitmap::NPOS[sizeof(base_type) * 8];
Bitmap::base_type Bitmap::MASK[sizeof(base_type) * 8];

bool Bitmap::init() {
    int n = 8*(sizeof(base_type));
    for(int i = 0; i < n; ++i) {
        POS[i] = ((base_type)1)<<i;
        NPOS[i] = ~(((base_type)1)<<i);
        MASK[i] = POS[i] - 1;
    }
    return true;
}

static bool s_init = Bitmap::init();

Bitmap::Bitmap(uint32_t size,uint8_t def)
    :m_size(size)
    ,m_dataSize(ceil(1.0*size/VALUE_SIZE))
    ,m_data(nullptr) {
        m_data = (base_type*)malloc(m_dataSize*sizeof(base_type));
        memset(m_data,def,m_dataSize*sizeof(base_type));
}

Bitmap::Bitmap()
    :m_size(0)
    ,m_dataSize(0)
    ,m_data(nullptr) {

}

Bitmap::Bitmap(const Bitmap& rhs) {
    m_size = rhs.m_size;
    m_dataSize = rhs.m_dataSize;
    m_data = (base_type*)malloc(m_dataSize * sizeof(base_type));
    memcpy(m_data,rhs.m_data,m_dataSize*sizeof(base_type));
}

Bitmap::~Bitmap() {
    if(m_data) {
        free(m_data);
    }
}

std::string Bitmap::toString() const {
    std::stringstream ss;
    ss << "[size=" << m_size
       << " data_size=" << m_dataSize
       << " data=";
    for(size_t i = 0; i < m_dataSize; ++i) {
        if(i) {
            ss << ",";
        }
        ss << m_data[i];
    }

    ss << "]";
    return ss.str();
}

bool Bitmap::get(uint32_t idx) const {
    uint32_t cur = idx / VALUE_SIZE;
    uint32_t pos = idx % VALUE_SIZE;
    if(cur >= m_dataSize) {
        throw std::out_of_range("idx out of range");
    }
    return m_data[cur]&POS[pos];
}

void Bitmap::set(uint32_t idx, bool v) {
    uint32_t cur = idx / VALUE_SIZE;
    uint32_t pos = idx % VALUE_SIZE;
    if(cur >= m_dataSize) {
        throw std::out_of_range("idx out of range");
    }
    if(v) {
        m_data[cur] |= POS[pos];
    } else {
        m_data[cur] &= NPOS[pos];
    }
}

bool Bitmap::any() const {
    if(m_data == nullptr) {
        return false;
    }

    for(uint32_t i = 0; i < m_dataSize; ++i) {
        if(m_data[i]) {
            return true;
        }
    }
    return false;
}

uint64_t countBits(const uint64_t& v) {
    return __builtin_popcount(v & 0xFFFFFFFF) 
        + __builtin_popcount(v >> 32);
}

uint32_t Bitmap::getCount() const {
    uint32_t len = m_dataSize / U64_DIV_BASE;
    uint32_t count = 0;
    uint32_t cur_pos = 0;
    for(uint32_t i = 0; i < len; ++i) {
        uint64_t tmp = ((uint64_t*)(m_data))[i];
        count += countBits(tmp);
        cur_pos += U64_VALUE_SIZE;
    }

    for(uint32_t i = len * U64_DIV_BASE; i < m_dataSize; ++i) {
        base_type tmp = m_data[i];
        if(tmp) {
            for(uint32_t n = 0; n < VALUE_SIZE && cur_pos < m_size; ++n,++cur_pos) {
                if(tmp & POS[n]) {
                    count++;
                }
            }
        } else {
            cur_pos += VALUE_SIZE;
        }
    }
    return count;
}

Bitmap& Bitmap::operator=(const Bitmap& rhs) {
    if(this == &rhs) {
        return *this;
    }

    m_dataSize = rhs.m_dataSize;
    m_size = rhs.m_size;
    if(m_data) {
        free(m_data);
        m_data = nullptr;
    }
    m_data = (base_type*)malloc(m_dataSize * sizeof(base_type));
    memcpy(m_data,rhs.m_data,m_dataSize * sizeof(base_type));
    return *this;
}

Bitmap& Bitmap::operator&=(const Bitmap& rhs) {
    if(m_size != rhs.m_size) {
        throw std::logic_error("size must be equal");
    }

    uint32_t maxSize = m_size / U64_VALUE_SIZE;
    for(uint32_t i = 0; i < maxSize; ++i) {
        ((uint64_t*)m_data)[i] &= ((uint64_t*)rhs.m_data)[i];
    }

    for(uint32_t i = maxSize * U64_DIV_BASE; i < m_dataSize; ++i) {
        m_data[i] &= rhs.m_data[i];
    }

    return *this;
}

Bitmap& Bitmap::operator|=(const Bitmap& rhs) {
    if(m_size != rhs.m_size) {
        throw std::logic_error("size must be equal");
    }

    uint32_t maxSize = m_size / U64_VALUE_SIZE;
    for(uint32_t i = 0; i < maxSize; ++i) {
        ((uint64_t*)m_data)[i] |= ((uint64_t*)rhs.m_data)[i];
    }

    for(uint32_t i = maxSize * U64_DIV_BASE; i < m_dataSize; ++i) {
        m_data[i] |= rhs.m_data[i];
    }

    return *this; 
}

Bitmap Bitmap::operator& (const Bitmap& rhs) {
    Bitmap ans(*this);
    return ans &= rhs;
}

Bitmap Bitmap::operator| (const Bitmap& rhs) {
    Bitmap ans(*this);
    return ans |= rhs;
}

Bitmap& Bitmap::operator~() {
    uint32_t maxSize = m_size / U64_VALUE_SIZE;
    for(uint32_t i = 0; i < maxSize; ++i) {
        ((uint64_t*)m_data)[i] = ~(((uint64_t*)m_data)[i]);
    }

    for(uint32_t i = maxSize * U64_DIV_BASE; i < m_dataSize; ++i) {
        m_data[i] = ~(m_data[i]);
    }

    return *this;
}

bool Bitmap::operator== (const Bitmap& rhs) const {
    if(this == &rhs) {
        return true;
    }

    if(m_size != rhs.m_size || m_dataSize != rhs.m_dataSize) {
        return false;
    }

    if(memcmp(m_data,rhs.m_data, m_dataSize * sizeof(base_type))) {
        return false;
    }

    return true;
}

bool Bitmap::operator!= (const Bitmap& rhs) const {
    return !(*this == rhs);
}

} // namespace ds
} // namespace xzmjx
