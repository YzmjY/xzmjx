//
// Created by 20132 on 2022/3/10.
//

#include "util.h"
#include <sys/stat.h>
#include "fiber.h"
#include <execinfo.h>
#include "log.h"
#include <sys/time.h>


namespace xzmjx{
    namespace {
        xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
    }
    bool FSUtil::OpenFileForRead(std::ifstream& stream,const char* filename,std::ios_base::openmode mode){
        stream.open(filename,mode);
        return stream.is_open();
    }
    bool FSUtil::OpenFileForWrite(std::ofstream& stream,const char* filename,std::ios_base::openmode mode){
        stream.open(filename,mode);
        return stream.is_open();
    }
    std::string FSUtil::Dirname(const std::string& filename){
        if(filename.empty()){
            return ".";
        }
        auto pos = filename.rfind('/');
        if(pos == 0){
            return "/";
        }else if(pos == std::string::npos){
            return ".";
        }else{
            /// xx/xx/xx
            ///      ^pos-> dir = xx/xx-> [0->pos-1]
            return filename.substr(0,pos);
        }
    }
    uint64_t GetThreadID(){
        return syscall(SYS_gettid);
    }

    uint64_t GetFiberID(){
        return Fiber::GetFiberId();
        //return 0;
    }

    void Backtrace(std::vector<std::string>& symbols,int size,int skip){
        void** buf = (void**)malloc(size*sizeof(void*));
        backtrace(buf,size);
        char** strings = backtrace_symbols(buf,size);
        if(strings == nullptr){
            XZMJX_LOG_ERROR(g_logger)<<"backtrace_symbols error";
            free(buf);
            return;
        }
        symbols.clear();
        symbols.reserve(size);
        for(int i = skip;i < size;i++){
            symbols.push_back(strings[i]);
        }

        free(buf);
        free(strings);
    }

    std::string BacktraceToString(int size, int skip ,const std::string& prefix){
        std::vector<std::string> symbols;
        Backtrace(symbols,size,skip);
        std::stringstream ss;
        for(auto&& symbol:symbols){
            ss<<prefix<<symbol;
        }
        return ss.str();
    }

    uint64_t GetCurMS(){
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return tv.tv_sec*1000+tv.tv_usec/1000;
    }

}