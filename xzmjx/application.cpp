#include "application.h"
#include "log.h"
#include "config.h"
#include "env.h"
#include "module.h"
#include "daemon.h"
#include "http/ws_server.h"
#include <signal.h>

static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");

static xzmjx::ConfigVar<std::string>::ptr g_server_work_path =
    xzmjx::Config::Lookup("server.work_path", std::string("/home/xzmjx/work/xzmjx"), "server work path");

static xzmjx::ConfigVar<std::string>::ptr g_server_pid_file =
    xzmjx::Config::Lookup("server.pid_file", std::string("xzmjx.pid"), "server pid file");

namespace xzmjx {
Application* Application::s_instance = nullptr;

Application::Application() { s_instance = this; }

bool Application::init(int argc, char** argv) {
    // 命令解析
    m_argc = argc;
    m_argv = argv;

    EnvMgr::GetInstance()->addArgHelp("s", "start with the terminal");
    EnvMgr::GetInstance()->addArgHelp("d", "run as daemon");
    EnvMgr::GetInstance()->addArgHelp("c", "conf path default: ./conf");
    EnvMgr::GetInstance()->addArgHelp("p", "print help");

    bool is_print_help = false;
    if (!EnvMgr::GetInstance()->init(argc, argv)) {
        is_print_help = true;
    }

    if (EnvMgr::GetInstance()->hasArg("p")) {
        is_print_help = true;
    }

    std::string conf_path = EnvMgr::GetInstance()->getConfigPath();
    XZMJX_LOG_INFO(g_logger) << "load conf path" << conf_path;
    Config::LoadFromConfDir(conf_path);

    // 加载.so模块
    ModuleMgr::GetInstance()->init();
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);
    for (auto&& i : modules) {
        i->onBeforeArgsParse(argc, argv);
    }

    if (is_print_help) {
        EnvMgr::GetInstance()->printHelp();
        return false;
    }

    for (auto&& i : modules) {
        i->onAfterArgsParse(argc, argv);
    }
    modules.clear();

    int run_type = 0;
    if (EnvMgr::GetInstance()->hasArg("s")) {
        run_type = 1;
    }
    if (EnvMgr::GetInstance()->hasArg("d")) {
        run_type = 2;
    }

    if (run_type == 0) {
        EnvMgr::GetInstance()->printHelp();
        return false;
    }

    std::string pid_file = g_server_work_path->getValue() + "/" + g_server_pid_file->getValue();
    if (FSUtil::IsRunningPidfile(pid_file)) {
        XZMJX_LOG_ERROR(g_logger) << "server is running:" << pid_file;
        return false;
    }

    if (!FSUtil::Mkdir(g_server_work_path->getValue())) {
        XZMJX_LOG_FATAL(g_logger) << "create work path [" << g_server_work_path->getValue() << " errno=" << errno
                                  << "errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Application::run() {
    bool is_daemon = EnvMgr::GetInstance()->hasArg("d");
    return start_daemon(m_argc, m_argv,
                        std::bind(&Application::main, this, std::placeholders::_1, std::placeholders::_2), is_daemon);
}

int Application::main(int argc, char** argv) {
    signal(SIGPIPE, SIG_IGN);
    XZMJX_LOG_INFO(g_logger) << "main";
    std::string conf_path = EnvMgr::GetInstance()->getConfigPath();
    {
        std::string pid_file = g_server_work_path->getValue() + "/" + g_server_pid_file->getValue();
        std::ofstream ofs(pid_file);
        if (!ofs) {
            XZMJX_LOG_ERROR(g_logger) << "open pidfile" << pid_file << " failed";
            return 0;
        }
        ofs << getpid();
    }
    m_mainIOManager.reset(new IOManager(4, "main"));
    m_mainIOManager->submit(std::bind(&Application::run_fiber, this));
    m_mainIOManager->addTimerEvent(
        2000,
        []() {
            // XZMJX_LOG_INFO(g_logger)<<"heart beats";
        },
        true);
    m_mainIOManager->stop();
    return 0;
}

int Application::run_fiber() {
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);
    bool error = false;
    // 加载模块
    for (auto&& i : modules) {
        if (!i->onLoad()) {
            XZMJX_LOG_ERROR(g_logger) << "module name=" << i->getName() << " version=" << i->getVersion()
                                      << " filename=" << i->getFilename();
            error = true;
        }
    }
    if (error) {
        _exit(0);
    }

    //@TODO: 启动服务
    std::vector<TcpServer::ptr> srvs;
    xzmjx::Address::ptr addr_1 = xzmjx::Address::LookupAnyIPAddress("0.0.0.0:8020");
    xzmjx::Address::ptr addr_2 = xzmjx::Address::LookupAnyIPAddress("0.0.0.0:8037");
    IOManager* accept_worker = IOManager::Self();
    IOManager* io_worker = IOManager::Self();
    IOManager* process_worker = IOManager::Self();
    TcpServer::ptr server_1;
    server_1.reset(new http::HttpServer(false, process_worker, io_worker, accept_worker));
    server_1->setName("xzmjx_http");
    server_1->bind(addr_1);
    srvs.push_back(server_1);
    m_servers["http"].push_back(server_1);
    TcpServer::ptr server_2;
    server_2.reset(new http::WSServer(process_worker, io_worker, accept_worker));
    server_2->setName("xzmjx_ws");
    server_2->bind(addr_2);
    srvs.push_back(server_2);
    m_servers["ws"].push_back(server_2);

    for (auto&& i : modules) {
        i->onServerReady();
    }
    for (auto&& i : srvs) {
        i->start();
    }
    for (auto&& i : modules) {
        i->onServerUp();
    }
    return 0;
}

bool Application::getServer(const std::string& type, std::vector<TcpServer::ptr>& srvs) {
    auto it = m_servers.find(type);
    if (it == m_servers.end()) {
        return false;
    }
    srvs = it->second;
    return true;
}

} // namespace xzmjx
