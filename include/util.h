//
// Created by 20132 on 2022/3/10.
//

#ifndef XZMJX_UTIL_H
#define XZMJX_UTIL_H
#include <fstream>
#include <unistd.h>
#include <sys/syscall.h>
#include <string>
#include <vector>
#include <cxxabi.h>

namespace xzmjx{
class FSUtil{
public:
    static std::string Dirname(const std::string& filename);
    static bool OpenFileForRead(std::ifstream& stream,const char* filename,std::ios_base::openmode mode);
    static bool OpenFileForWrite(std::ofstream& stream,const char* filename,std::ios_base::openmode mode);
};

uint64_t GetThreadID();

uint64_t GetFiberID();

void Backtrace(std::vector<std::string>& symbols,int size,int skip = 1);

std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

uint64_t GetCurMS();

template<typename T>
const char* TypeToName(){
    static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return s_name;
}

}///namespace xzmjx


#endif //XZMJX_UTIL_H
