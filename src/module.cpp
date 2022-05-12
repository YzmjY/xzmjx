#include "module.h"
#include "log.h"
#include "config.h"
#include "library.h"
static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
static xzmjx::ConfigVar<std::string>::ptr g_module_path
        = xzmjx::Config::Lookup("module.path",std::string("module"),"module path");

namespace xzmjx{
Module::Module(const std::string& name
        ,const std::string& version
        ,const std::string& filename)
        :m_name(name)
        ,m_version(version)
        ,m_filename(filename)
        ,m_id(name+"/"+version){

}
Module::~Module(){

}
void Module::onBeforeArgsParse(int argc,char** argv){

}
void Module::onAfterArgsParse(int argc,char** argv){

}
bool Module::onLoad(){
    return true; 
}
bool Module::onUnload(){
    return true;
}
bool Module::onConnect(xzmjx::Stream::ptr stream){
    return true;
}
bool Module::onDisConnect(xzmjx::Stream::ptr stream){
    return true;
}
bool Module::onServerReady(){
    return true;
}
bool Module::onServerUp(){
    return true;
}
std::string Module::statusString(){
    std::stringstream ss;
    ss<<"Module name="<<getName()
      <<" version="<<getVersion()
      <<" filename="<<getFilename()
      <<std::endl;
    return ss.str();
}

ModuleManager::ModuleManager(){

}
void ModuleManager::add(Module::ptr m){
    RWMutexType::WriteLock lock(m_rwlock);
    m_modules[m->getId()] = m;                       
}
void ModuleManager::del(const std::string& m){
    RWMutexType::WriteLock lock(m_rwlock);
    auto it = m_modules.find(m);
    if(it == m_modules.end()){
        return;
    }
    Module::ptr module = it->second;
    m_modules.erase(it);
    lock.unlock();
    module->onUnload();
}
void ModuleManager::delAll(){
    RWMutexType::ReadLock lock(m_rwlock);
    auto tmp = m_modules;
    lock.unlock();
    for(auto&& it:tmp){
        del(it.first);
    }
}
void ModuleManager::init(){

}
Module::ptr ModuleManager::get(const std::string& name){

}
void ModuleManager::onConnect(Stream::ptr stream){

}
void ModuleManager::onDisConnect(Stream::ptr stream){

}
void ModuleManager::listAll(std::vector<Module::ptr>& m){

}
void ModuleManager::listByType(uint32_t type,std::vector<Module::ptr>& m){

}
void ModuleManager::foreach(uint32_t type,std::function<void(Module::ptr)> cb){

}
void ModuleManager::initModule(const std::string& path){
    Module::ptr m = Library::GetModule(path);
    if(m){
        add(m);
    }
}
}