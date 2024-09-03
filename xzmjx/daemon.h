#ifndef XZMJX_DAEMON_H
#define XZMJX_DAEMON_H

#include <functional>
#include "singleton.h"
namespace xzmjx {
struct ProcessInfo {
    typedef std::shared_ptr<ProcessInfo> ptr;
    pid_t parent_pid = 0;
    pid_t main_pid = 0;
    uint64_t parent_start_ts = 0;
    uint64_t main_start_ts = 0;
    uint32_t restart_cnt = 0;

    std::string toString() const;
};
typedef xzmjx::SingletonPtr<ProcessInfo> ProcessInfoMgr;
namespace {
typedef std::function<int(int argc, char** argv)> MainCallback;
}
int start_daemon(int agrc, char** argv, MainCallback main_cb, bool is_daemon);
} // namespace xzmjx

#endif