//
// Created by 20132 on 2022/4/17.
//
#include "stream.h"

namespace xzmjx{
int Stream::readFixedSize(void *buf, size_t length) {
    int left = length;
    while(left>0){
        int rt = read(buf,left);
        if(rt<=0){
            return rt;
        }
        left-=rt;
    }
    return length;
}

int Stream::readFixedSize(ByteArray::ptr ba, size_t length) {
    int left = length;
    while(left>0){
        int rt = read(ba,length);
        if(rt<=0){
            return rt;
        }
        left-=rt;
    }
    return length;
}

int Stream::writeFixedSize(const void *buf, size_t length) {
    int left = length;
    while(left>0){
        int rt = write(buf,length);
        if(rt<=0){
            return rt;
        }
        left-=rt;
    }
    return length;
}

int Stream::writeFixedSize(ByteArray::ptr ba, size_t length) {
    int left = length;
    while(left>0){
        int rt = write(ba,length);
        if(rt<=0){
            return rt;
        }
        left-=rt;
    }
    return length;
}

}///namespace xzmjx
