#ifndef XZMJX_ENV_H
#define XZMJX_ENV_H
#include "mutex.h"
#include "singleton.h"
#include <map>
#include <string>
#include <vector>

namespace xzmjx{

/**
 * @brief 环境变量,启动参数
 * 
 */
class Env{
public:
    typedef RWMutex RWMutexType;
    typedef std::shared_ptr<Env> ptr;
    bool init(int argc,char** argv);

    void addArg(const std::string& key,const std::string& val);
    bool hasArg(const std::string& key);
    void delArg(const std::string& key);
    std::string get(const std::string& key, const std::string& def = "");
    void addArgHelp(const std::string& key,const std::string& help_desc);
    void removeArgHrlp(const std::string& key);
    void printHelp();
    void printArg();

    bool setEnv(const std::string& key,const std::string& val);
    std::string getEnv(const std::string& key,const std::string& def = "");

    const std::string& getExe() const { return m_exe; }
    const std::string& getCwd() const { return m_cwd; }

    std::string getAbsolutePath(const std::string& path) const;
    std::string getAbsoluteWorkPath(const std::string& path) const;
    std::string getConfigPath();

private:
    RWMutexType m_rwlock;
    std::map<std::string,std::string> m_args;
    std::vector<std::pair<std::string,std::string>> m_args_helps;
    std::string m_program;
    std::string m_exe; // 二进制文件的路径 
    std::string m_cwd; // 指二进制文件所在的目录，而不是真正的工作目录
};

typedef xzmjx::SingletonPtr<Env> EnvMgr;
}


#endif