#ifndef XZMJX_LIBRARY_H
#define XZMJX_LIBRARY_H
#include "module.h"
namespace xzmjx {
class Library {
public:
    static Module::ptr GetModule(const std::string& path);
};
} // namespace xzmjx

#endif