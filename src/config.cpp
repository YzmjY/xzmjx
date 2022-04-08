//
// Created by 20132 on 2022/3/11.
//
#include "config.h"


namespace xzmjx{
namespace {
    xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
}

void ListAllNodes(){

}

void Config::LoadFromYaml(const YAML::Node& rootNode){

}
void Config::LoadFromConfDir(const std::string& path,bool force){

}

ConfigBase::ptr Config::LookupBase(const std::string&name){
    RWMutexType::ReadLock lock(GetRwMutex());
    auto iter = GetConfigMap().find(name);
    return iter == GetConfigMap().end()? nullptr:iter->second;
}
}
