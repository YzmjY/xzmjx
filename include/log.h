//
// Created by 20132 on 2022/3/10.
//

#ifndef XZMJX_LOG_H
#define XZMJX_LOG_H

#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include <cstdarg>
#include <vector>
#include <unordered_map>

#include "singleton.h"
#include "util.h"
#include "thread.h"

/*
 * @brief 流式输出
 **/

#define XZMJX_LOG_LEVEL(logger,level) \
if(logger->getLevel()<=level)         \
xzmjx::LogEventWrap(xzmjx::LogEvent::ptr(new xzmjx::LogEvent(logger,level,__FILE__,__LINE__, \
                                        0,xzmjx::GetThreadID(),xzmjx::GetFiberID(),          \
                                        static_cast<uint64_t>(time(NULL)),xzmjx::Thread::GetName()))).getSS()


#define XZMJX_LOG_DEBUG(logger) XZMJX_LOG_LEVEL(logger,xzmjx::LogLevel::DEBUG)
#define XZMJX_LOG_INFO(logger) XZMJX_LOG_LEVEL(logger,xzmjx::LogLevel::INFO)
#define XZMJX_LOG_WARN(logger) XZMJX_LOG_LEVEL(logger,xzmjx::LogLevel::WARN)
#define XZMJX_LOG_ERROR(logger) XZMJX_LOG_LEVEL(logger,xzmjx::LogLevel::ERROR)
#define XZMJX_LOG_FATAL(logger) XZMJX_LOG_LEVEL(logger,xzmjx::LogLevel::FATAL)

#define XZMJX_LOG_FMT_LEVEL(logger,level,fmt,...) \
if(logger->getLevel()<=level)         \
xzmjx::LogEventWrap(xzmjx::LogEvent::ptr(new xzmjx::LogEvent(logger,level,__FILE__,__LINE__, \
                                        0,xzmjx::GetThreadID(),xzmjx::GetFiberID(),          \
                                        static_cast<uint64_t>(time(NULL)),xzmjx::Thread::GetName()))).getEvent()->format(fmt,__VA_ARGS__)

#define XZMJX_LOG_FMT_DEBUG(logger,fmt,...) XZMJX_LOG_FMT_LEVEL(logger,xzmjx::LogLevel::DEBUG,fmt,__VA_ARGS__)
#define XZMJX_LOG_FMT_INFO(logger,fmt,...) XZMJX_LOG_FMT_LEVEL(logger,xzmjx::LogLevel::INFO,fmt,__VA_ARGS__)
#define XZMJX_LOG_FMT_WARN(logger,fmt,...) XZMJX_LOG_FMT_LEVEL(logger,xzmjx::LogLevel::WARN,fmt,__VA_ARGS__)
#define XZMJX_LOG_FMT_ERROR(logger,fmt,...) XZMJX_LOG_FMT_LEVEL(logger,xzmjx::LogLevel::ERROR,fmt,__VA_ARGS__)
#define XZMJX_LOG_FMT_FATAL(logger,fmt,...) XZMJX_LOG_FMT_LEVEL(logger,xzmjx::LogLevel::FATAL,fmt,__VA_ARGS__)

#define XZMJX_LOG_ROOT() xzmjx::LogMgr::GetInstance()->getRoot()
#define XZMJX_LOG_NAME(name) xzmjx::LogMgr::GetInstance()->getLogger(name)

namespace xzmjx{

enum class LogLevel{
    UNKONWN = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

std::string ToString(LogLevel level);
LogLevel FromString(const std::string& str);

//日志现场信息
class Logger;
class LogEvent{
public:
    typedef std::shared_ptr<LogEvent> ptr;

    LogEvent(std::shared_ptr<Logger> logger,
             LogLevel level,const char* filename,
             int32_t line,
             uint32_t elapse,
             uint32_t thread_id,
             uint32_t fiber_id,
             uint64_t time,
             const std::string& thread_name);


    ~LogEvent() = default;

    const char* getFile() const{
        return m_filename;
    }

    int32_t getLine() const{
        return m_line;
    }

    uint32_t getElapse() const{
        return m_elapse;
    }

    uint32_t getThreadId() const{
        return m_threadId;
    }

    uint32_t getFiberId() const{
        return m_fiberId;
    }

    uint64_t getTime() const{
        return m_time;
    }

    const std::string& getThreadName() const{
        return m_threadName;
    }

    std::shared_ptr<Logger> getLogger() const{
        return m_logger;
    }

    LogLevel getLogLevel() const{
        return m_level;
    }

    std::stringstream& getStringStream(){
        return m_sstream;
    }

    void format(const char* fmt,...);
    void format(const char* fmt, va_list va);

    std::string getContent() const{
        return m_sstream.str();
    }


private:
    const char* m_filename;             ///当前文件名
    int32_t m_line;                     ///当前行号
    uint32_t m_elapse;                  ///当前程序运行时间
    uint32_t m_threadId;                ///当前线程id
    uint32_t m_fiberId;                 ///当前协程id
    uint64_t m_time;                    ///当前时间戳
    std::string m_threadName;           ///当前线程名称
    std::stringstream m_sstream;        ///流式输出用户指定的日志内容
    std::shared_ptr<Logger> m_logger;   ///当前日志器，用来获取日志器名称
    LogLevel m_level;                   ///日志等级
};

class LogEventWrap{
public:
    explicit LogEventWrap(LogEvent::ptr event);
    ~LogEventWrap();

    LogEvent::ptr getEvent() const {return m_event;}
    std::stringstream& getSS();

private:
    LogEvent::ptr m_event;
};
///日志格式化
class LogFormatter{
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    explicit LogFormatter(const std::string& pattern="%d{%Y-%m-%d %H:%M:%S}%T[%p]%T[%c]%T%t%T%N%T%F%T%T%f:%l%T%m%n");
    std::string format(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event);
    std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event);
    bool error();


public:
    class FormatterItem{
    public:
        typedef std::shared_ptr<FormatterItem> ptr;
        virtual ~FormatterItem() = default;
        virtual void format(std::ostream& os,std::shared_ptr<Logger> logger,LogLevel level, LogEvent::ptr event) = 0;
    };

private:
    void init();
private:
    std::string m_pattern;
    std::vector<FormatterItem::ptr> m_items;
    bool m_error;
};

///日志输出地,与LogFormatter绑定实现项指定输出地输出格式化的日志
class Logger;
class LogAppender{
    friend class Logger;
public:
    typedef std::shared_ptr<LogAppender> ptr;
    typedef Spinlock MutexType;
    LogAppender();
    virtual ~LogAppender() = default;


    virtual void log(std::shared_ptr<Logger> logger,LogLevel level,LogEvent::ptr event) = 0;

    void setFormatter(LogFormatter::ptr format);
    LogFormatter::ptr getFormat();

    void setLevel(LogLevel level){
        m_level = level;
    }
    LogLevel getLevel(){
        return m_level;
    }

protected:
    LogFormatter::ptr m_formatter;
    LogLevel m_level;
    MutexType m_mutex;
    bool m_hasFormatter;
};

class StdOutLogAppender:public LogAppender{
public:
    typedef std::shared_ptr<StdOutLogAppender> ptr;
    void log(std::shared_ptr<Logger> logger,LogLevel level,LogEvent::ptr event) override;
};

class FileLogAppender:public LogAppender{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    explicit FileLogAppender(const std::string& filename);
    void log(std::shared_ptr<Logger> logger,LogLevel level,LogEvent::ptr event) override;

    bool reopen();

private:
    std::string m_filename;     ///日志文件名
    std::ofstream m_filestream; ///文件打开流
    uint64_t m_lastTime = 0;    ///上次打开时间
};

//日志器，拥有多个LogAppender，与用户直接相关，将日志事件落地到appender中
class Logger:public std::enable_shared_from_this<Logger>{
public:
    typedef std::shared_ptr<Logger> ptr;
    typedef Spinlock MutexType;

    explicit Logger(std::string name = "root");
    ~Logger() = default;

    void log(LogLevel level,LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppender();

    LogLevel getLevel(){
        return m_level;
    }
    void setLevel(LogLevel level){
        m_level = level;
    }

    std::string getName(){
        return m_name;
    }

    void setRoot(Logger::ptr root){
        m_root = root;
    }

    Logger::ptr getRoot(){
        return m_root;
    }

    void setFormatter(LogFormatter::ptr formtter) ;
    void setFormatter(std::string pattern) ;

    LogFormatter::ptr getFormatter();

private:
    std::string m_name;
    LogLevel m_level;
    Logger::ptr m_root;
    LogFormatter::ptr m_formatter;
    std::vector<LogAppender::ptr> m_appenders;
    MutexType m_mutex;
};

class LoggerManager{
public:
    typedef std::shared_ptr<LoggerManager> ptr;
    typedef Spinlock MutexType;

    LoggerManager();

    Logger::ptr getLogger(const std::string& name);

    void init();

    Logger::ptr getRoot() const{
        return m_root;
    }


private:
    std::unordered_map<std::string,Logger::ptr> m_loggers;
    Logger::ptr m_root;
    MutexType m_mutex;
};
typedef xzmjx::SingletonPtr<LoggerManager> LogMgr;
}///namespace xzmjx
#endif //XZMJX_LOG_H
