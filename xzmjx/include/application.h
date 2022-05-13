#ifndef XZMJX_APPLICATION_H
#define XZMJX_APPLICATION_H
#include "http/http_server.h"
namespace xzmjx{
class Application{
public:
    Application();
    static Application* GetInstance() { return s_instance; }
    bool init(int argc,char** argv);
    bool run();
    bool getServer(const std::string& type,std::vector<TcpServer::ptr>& srvs);
    void listAllServer(std::map<std::string,std::vector<TcpServer::ptr>>& srvs);

private:
    int main(int argc,char** argv);
    int run_fiber();

private:
    int m_argc;
    char** m_argv;    
    IOManager::ptr m_mainIOManager;
    std::map<std::string,std::vector<TcpServer::ptr>> m_servers;
    static Application* s_instance;
};
}



#endif