#ifndef XZMJX_DS_BITMAP_H_
#define XZMJX_DS_BITMAP_H_

#include <memory>
#include <functional>

#include "bytearray.h"
namespace xzmjx {
namespace ds {

#define BITMAP_TYPE_UINT8 1
#define BITMAP_TYPE_UINT16 2
#define BITMAP_TYPE_UINT32 3
#define BITMAP_TYPE_UINT64 4

#ifndef BITMAP_TYPE
#define BITMAP_TYPE BITMAP_TYPE_UINT16
#endif

class Bitmap {
public:
    typedef std::shared_ptr<Bitmap> ptr;

#if BITMAP_TYPE == BITMAP_TYPE_UINT8
    typedef uint8_t base_type;
#elif BITMAP_TYPE == BITMAP_TYPE_UINT16
    typedef uint16_t base_type;
#elif BITMAP_TYPE == BITMAP_TYPE_UINT32
    typedef uint32_t base_type;
#elif BITMAP_TYPE == BITMAP_TYPE_UINT64
    typedef uint64_t base_type;
#endif

    explicit Bitmap(uint32_t size, uint8_t def = 0);
    Bitmap();
    Bitmap(const Bitmap& rhs);
    ~Bitmap();

    Bitmap& operator=(const Bitmap& rhs);
    std::string toString() const;
    bool get(uint32_t idx) const;
    void set(uint32_t idx, bool v);

    Bitmap& operator&=(const Bitmap& b);
    Bitmap& operator|=(const Bitmap& b);

    Bitmap operator&(const Bitmap& b);
    Bitmap operator|(const Bitmap& b);

    Bitmap& operator~();

    bool operator==(const Bitmap& b) const;
    bool operator!=(const Bitmap& b) const;

    bool any() const;

    uint32_t getSize() const { return m_size; }
    uint32_t getDataSize() const { return m_dataSize; }

    // void foreach(std::function<bool(uint32_t)> cb);
    // void rforeach(std::function<bool(uint32_t)> cb);

    // void writeTo(xzmjx::ByteArray::ptr ba) const;
    // bool readFrom(xzmjx::ByteArray::ptr ba);

    uint32_t getCount() const;

    static bool init();

private:
    uint32_t m_size;
    uint32_t m_dataSize;
    base_type* m_data;

private:
    static base_type POS[sizeof(base_type) * 8];
    static base_type NPOS[sizeof(base_type) * 8];
    static base_type MASK[sizeof(base_type) * 8];

    static const uint32_t VALUE_SIZE = sizeof(base_type) * 8;
    static const base_type U64_DIV_BASE = (sizeof(uint64_t) / sizeof(base_type));
    static const base_type U64_VALUE_SIZE = (VALUE_SIZE * U64_DIV_BASE);
};
} // namespace ds
} // namespace xzmjx

#endif