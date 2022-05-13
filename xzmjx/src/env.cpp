#include "env.h"
#include "log.h"
#include "config.h"
#include <cstring>
#include <iostream>
#include <iomanip>

static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
namespace xzmjx{
bool Env::init(int argc,char** argv){
    char link[1024] = {0};
    char path[1024] = {0};
    sprintf(link,"/proc/%d/exe",getpid());
    readlink(link,path,sizeof(path));
    m_exe = path;
    auto pos = m_exe.find_last_of("/");
    m_cwd = m_exe.substr(0,pos)+"/";

    m_program = argv[0];
    const char* now_key = nullptr;
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            if(strlen(argv[i])>1){
                if(now_key){
                    addArg(now_key,"");
                }
                now_key = argv[i]+1;
            }else{
                XZMJX_LOG_ERROR(g_logger)<<"invalid arg idx="
                                         <<i<<" val="<<argv[i];
                return false;
            }
        }else{
            if(now_key){
                addArg(now_key,argv[i]);
                now_key = nullptr;
            }else{
                XZMJX_LOG_ERROR(g_logger)<<"invalid arg idx="
                                         <<i<<" val="<<argv[i];
                return false;
            }
        }
    }
    if(now_key){
        addArg(now_key,"");
    }
    return true;
}

void Env::addArg(const std::string& key,const std::string& val){
    RWMutexType::WriteLock lock(m_rwlock);
    m_args[key] = val;
}

bool Env::hasArg(const std::string& key){
    RWMutexType::ReadLock lock(m_rwlock);
    return m_args.find(key) != m_args.end();
}

void Env::delArg(const std::string& key){
    RWMutexType::WriteLock lock(m_rwlock);
    m_args.erase(key);
}

std::string Env::get(const std::string& key, const std::string& def){
    RWMutexType::ReadLock lock(m_rwlock);
    auto it = m_args.find(key);
    if(it == m_args.end()){
        return def;
    }else{
        return it->second;
    }
}

void Env::addArgHelp(const std::string& key,const std::string& help_desc){
    removeArgHrlp(key);
    RWMutexType::WriteLock lock(m_rwlock);
    m_args_helps.push_back(std::make_pair(key,help_desc));
}

void Env::removeArgHrlp(const std::string& key){
    RWMutexType::WriteLock lock(m_rwlock);
    for(size_t i = 0; i < m_args_helps.size(); ++i){
        if(m_args_helps[i].first == key){
            m_args_helps.erase(m_args_helps.begin()+i);
            i--;
        }
    }
}

void Env::printHelp(){
    RWMutexType::ReadLock lock(m_rwlock);
    std::cout<< "Usage: "<<m_program<<" [options]"<<std::endl;
    for(auto&& it:m_args_helps){
        std::cout<<std::setw(5)<<"-"<<it.first<<" : "<<it.second<<std::endl;
    }
}

void Env::printArg(){
    RWMutexType::ReadLock lock(m_rwlock);
    for(auto&& it:m_args){
        std::cout<<std::setw(5)<<"-"<<it.first<<" : "<<it.second<<std::endl;
    }
}

bool Env::setEnv(const std::string& key,const std::string& val){
    return !setenv(key.c_str(),val.c_str(),1);
}

std::string Env::getEnv(const std::string& key,const std::string& def ){
    const char* v = getenv(key.c_str());
    if(v == nullptr){
        return def;
    }else{
        return std::string(v);
    }
}

std::string Env::getAbsolutePath(const std::string& path) const{
    if(path.empty()){
        return "/";
    }
    if(path[0] == '/'){
        return path;
    }
    return m_cwd + path;
}

std::string Env::getAbsoluteWorkPath(const std::string& path) const{
    if(path.empty()){
        return "/";
    }
    if(path[0] == '/'){
        return path;
    }
    static xzmjx::ConfigVar<std::string>::ptr g_server_work_path = 
        xzmjx::Config::Lookup<std::string>("server.work_path");
    return g_server_work_path->getValue()+"/"+path;
}

std::string Env::getConfigPath(){
    return getAbsolutePath(get("c","conf"));
}
}