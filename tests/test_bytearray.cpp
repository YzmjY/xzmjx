//
// Created by 20132 on 2022/4/17.
//
#include "bytearray.h"
#include "log.h"
#include <vector>
xzmjx::Logger::ptr g_logger = XZMJX_LOG_ROOT();

void test(){
    xzmjx::ByteArray::ptr ba(new xzmjx::ByteArray(1));
    std::vector<uint64_t> vec;
    for(uint64_t i = 0; i< 100;++i){
        vec.push_back(i);
    }
    for(auto&& it:vec){
        ba->writeUint64(it);
    }
    XZMJX_LOG_INFO(g_logger)<<"ByteArray Position:"<<ba->getPosition();
    ba->setPosition(0);
    for(size_t i = 0;i<vec.size();++i){
        uint64_t v = ba->readUint64();
        if(v!=vec[i]){
            XZMJX_LOG_ERROR(g_logger)<<"ERROR";
        }else{
            XZMJX_LOG_INFO(g_logger)<<"GO:"<<v<<"=="<<vec[i];
        }
    }
}

int main(){
    test();
    return 0;
}

