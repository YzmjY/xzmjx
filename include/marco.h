//
// Created by 20132 on 2022/3/16.
//

#ifndef XZMJX_MARCO_H
#define XZMJX_MARCO_H
#include "util.h"
#include <assert.h>
#include "log.h"

#define XZMJX_ASSERT(x) \
    if(!(x)){             \
        XZMJX_LOG_ERROR(XZMJX_LOG_ROOT())<<"ASSERTION : "#x \
        <<"\nbacktrace\n"\
        <<xzmjx::BacktraceToString(100,2,"\t");             \
        assert(x);\
    }

#define XZMJX_ASSERT_2(x,m) \
    if(!(x)){             \
        XZMJX_LOG_ERROR(XZMJX_LOG_ROOT())<<"ASSERTION : "#x \
        <<"\n"<<m                    \
        <<"\nbacktrace\n"\
        <<xzmjx::BacktraceToString(100,2,"\t");             \
        assert(x);\
    }

#endif //XZMJX_MARCO_H
