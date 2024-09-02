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
#include <bit>
#include <byteswap.h>
#include <cstdint>


namespace xzmjx{
class FSUtil{
public:
    static std::string Dirname(const std::string& filename);
    static bool OpenFileForRead(std::ifstream& stream,const char* filename,std::ios_base::openmode mode);
    static bool OpenFileForWrite(std::ofstream& stream,const char* filename,std::ios_base::openmode mode);
    static void ListAllFile(std::vector<std::string>& files
            ,const std::string& path
            ,const std::string& subfix);
    static bool Mkdir(const std::string& dirname);
    static bool IsRunningPidfile(const std::string& pidfile);
    static bool Rm(const std::string& path);
    static bool Mv(const std::string& from, const std::string& to);
    static bool Realpath(const std::string& path, std::string& rpath);
    static bool Symlink(const std::string& frm, const std::string& to);
    static bool Unlink(const std::string& filename, bool exist = false);
    static std::string Basename(const std::string& filename);
};

class StringUtil {
public:
    static std::string Format(const char* fmt, ...);
    static std::string Formatv(const char* fmt, va_list ap);

    static std::string UrlEncode(const std::string& str, bool space_as_plus = true);
    static std::string UrlDecode(const std::string& str, bool space_as_plus = true);

    static std::string Trim(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimLeft(const std::string& str, const std::string& delimit = " \t\r\n");
    static std::string TrimRight(const std::string& str, const std::string& delimit = " \t\r\n");


    static std::string WStringToString(const std::wstring& ws);
    static std::wstring StringToWString(const std::string& s);

};

std::string base64decode(const std::string &src);
std::string base64encode(const void *data, size_t len);
std::string sha1sum(const void *data, size_t len);


uint64_t GetThreadID();

uint64_t GetFiberID();

void Backtrace(std::vector<std::string>& symbols,int size,int skip = 1);

std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

uint64_t GetCurMS();
std::string Time2Str(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S") ;

template<typename T>
const char* TypeToName(){
    static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return s_name;
}

template<typename T>
T ByteSwap(T value){
    if(sizeof(T) == sizeof(uint8_t)){
        return value;
    }else if(sizeof(T) == sizeof(uint16_t)){
        return (T) bswap_16((uint16_t)value);
    }else if(sizeof(T) == sizeof(uint32_t)){
        return (T) bswap_32((uint32_t)value);
    }else if(sizeof(T) == sizeof(uint64_t)){
        return (T) bswap_64((uint64_t)value);
    }
    return -1;
}

enum EndianType{
            XZMJX_Little_Endian,
            XZMJX_Big_Endian
        };

template<typename T>
T EndianCastOnLittle(T value){
    if(std::endian::native == std::endian::little){
        return ByteSwap(value);
    }else{
        return value;
    }
}
template<typename T>
T EndianCastByType(T value,EndianType type){
    if(type == XZMJX_Little_Endian&&std::endian::native == std::endian::big){
        return ByteSwap(value);
    }else if(type == XZMJX_Big_Endian&&std::endian::native == std::endian::little){
        return ByteSwap(value);
    }else{
        return value;
    }
}






}///namespace xzmjx


#endif //XZMJX_UTIL_H
