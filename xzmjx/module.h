#ifndef XZMJX_MODULE_H
#define XZMJX_MODULE_H
#include "stream.h"
#include "mutex.h"
#include "singleton.h"
#include <memory>
#include <functional>
#include <unordered_map>

namespace xzmjx {
class Module {
public:
    typedef std::shared_ptr<Module> ptr;
    Module(const std::string& name, const std::string& version, const std::string& filename);
    virtual ~Module();
    virtual void onBeforeArgsParse(int argc, char** argv);
    virtual void onAfterArgsParse(int argc, char** argv);

    virtual bool onLoad();
    virtual bool onUnload();

    virtual bool onConnect(xzmjx::Stream::ptr stream);
    virtual bool onDisConnect(xzmjx::Stream::ptr stream);

    virtual bool onServerReady();
    virtual bool onServerUp();

    virtual std::string statusString();
    const std::string& getName() const { return m_name; }
    const std::string& getVersion() const { return m_version; }
    const std::string& getFilename() const { return m_filename; }
    const std::string& getId() const { return m_id; }

    void setFilename(const std::string& filename) { m_filename = filename; }

private:
    std::string m_name;
    std::string m_version;
    std::string m_filename;
    std::string m_id;
};

class ModuleManager {
public:
    typedef std::shared_ptr<ModuleManager> ptr;
    typedef RWMutex RWMutexType;

    ModuleManager();
    void add(Module::ptr m);
    void del(const std::string& m);
    void delAll();
    void init();
    Module::ptr get(const std::string& name);
    void onConnect(Stream::ptr stream);
    void onDisConnect(Stream::ptr stream);
    void listAll(std::vector<Module::ptr>& m);

private:
    void initModule(const std::string& path);

private:
    RWMutexType m_rwlock;
    std::unordered_map<std::string, Module::ptr> m_modules;
};
typedef xzmjx::SingletonPtr<ModuleManager> ModuleMgr;
} // namespace xzmjx

#endif