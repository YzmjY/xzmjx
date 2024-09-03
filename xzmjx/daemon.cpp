#include "daemon.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <cstring>

#include "config.h"
#include "log.h"

static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
static xzmjx::ConfigVar<uint32_t>::ptr g_daemon_restart_interval =
    xzmjx::Config::Lookup("daemon.restart_interval", (uint32_t)5, "daemon restart interval");

namespace xzmjx {
std::string ProcessInfo::toString() const {
    std::stringstream ss;
    ss << "[ProcessInfo parent_pid=" << parent_pid << " main_pid=" << main_pid << " parent_start_ts=" << parent_start_ts
       << " main_start_ts=" << main_start_ts << " restart_cnt=" << restart_cnt << "]";
    return ss.str();
}

static int real_start(int argc, char** argv, MainCallback main_cb) {
    ProcessInfoMgr::GetInstance()->main_pid = getpid();
    ProcessInfoMgr::GetInstance()->main_start_ts = time(NULL);
    return main_cb(argc, argv);
}

static int real_daemon(int argc, char** argv, MainCallback main_cb) {
    daemon(1, 0);
    ProcessInfoMgr::GetInstance()->parent_pid = getpid();
    ProcessInfoMgr::GetInstance()->parent_start_ts = time(0);
    while (true) {
        pid_t pid = fork();
        if (pid == 0) {
            // child
            ProcessInfoMgr::GetInstance()->main_pid = getpid();
            ProcessInfoMgr::GetInstance()->main_start_ts = time(NULL);
            XZMJX_LOG_INFO(g_logger) << "process start pid=" << getpid();
            return real_start(argc, argv, main_cb);
        } else if (pid < 0) {
            XZMJX_LOG_ERROR(g_logger) << "fork fail return=" << pid << " errno=" << errno
                                      << " errstr=" << strerror(errno);
        } else {
            int status = 0;
            waitpid(pid, &status, 0);
            if (status) {
                if (status == 9) {
                    XZMJX_LOG_INFO(g_logger) << "killed";
                    break;
                } else {
                    XZMJX_LOG_ERROR(g_logger) << "child crash pit" << pid << " status=" << status;
                }
            } else {
                XZMJX_LOG_INFO(g_logger) << "child finished pit=" << pid;
                break;
            }
        }
        ProcessInfoMgr::GetInstance()->restart_cnt += 1;
        sleep(g_daemon_restart_interval->getValue());
    }
    return 0;
}

int start_daemon(int agrc, char** argv, MainCallback main_cb, bool is_daemon) {
    if (is_daemon) {
        return real_daemon(agrc, argv, main_cb);
    } else {
        ProcessInfoMgr::GetInstance()->parent_pid = getpid();
        ProcessInfoMgr::GetInstance()->parent_start_ts = time(0);
        return real_start(agrc, argv, main_cb);
    }
}

} // namespace xzmjx