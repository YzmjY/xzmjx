//
// Created by 20132 on 2022/3/10.
//
#include "log.h"
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <map>
#include <functional>

namespace xzmjx{
    std::string ToString(LogLevel level){
        std::string ans;
        switch (level) {
#define XX(name) \
    case LogLevel::name:\
        ans = #name;    \
        break;
            XX(DEBUG)
            XX(INFO)
            XX(WARN)
            XX(ERROR)
            XX(FATAL)
            XX(UNKONWN)
#undef XX
            default:
                ans = "UNKONWN";
        }
        return ans;
    }

    LogLevel FromString(const std::string& str){
        LogLevel ans = LogLevel::UNKONWN;
#define XX(level,v) \
    if(str == #v){  \
        ans = LogLevel::level;\
    }
        XX(DEBUG,DEBUG)
        XX(INFO,INFO)
        XX(WARN,WARN)
        XX(ERROR,ERROR)
        XX(FATAL,FATAL)
        XX(UNKONWN,UNKONWN)

        XX(DEBUG,debug)
        XX(INFO,info)
        XX(WARN,warn)
        XX(ERROR,error)
        XX(FATAL,fatal)
        XX(UNKONWN,unkonwn)
#undef XX
        return ans;
    }
    LogEvent::LogEvent(std::shared_ptr<Logger> logger,
                       LogLevel level,
                       const char* filename,
                       int32_t line,
                       uint32_t elapse,
                       uint32_t thread_id,
                       uint32_t fiber_id,
                       uint64_t time,
                       const std::string& thread_name):
                       m_filename(filename),
                       m_line(line),
                       m_elapse(elapse),
                       m_threadId(thread_id),
                       m_fiberId(fiber_id),
                       m_time(time),
                       m_threadName(thread_name),
                       m_logger(logger),
                       m_level(level){}

    void LogEvent::format(const char* fmt,...){
        va_list va;
        va_start(va,fmt);
        format(fmt,va);
        va_end(va);
    }
    void LogEvent::format(const char* fmt, va_list va){
        char* buf = nullptr;
        int len = vasprintf(&buf,fmt,va);
        if(len != -1){
            m_sstream<<std::string(buf);
            free(buf);
        }
    }


    LogEventWrap::LogEventWrap(LogEvent::ptr event):m_event(event){

    }
    LogEventWrap::~LogEventWrap(){
        m_event->getLogger()->log(m_event->getLogLevel(),m_event);
    }

    std::stringstream& LogEventWrap::getSS(){
        return m_event->getStringStream();
    }

    class MessageFormatItem : public LogFormatter::FormatterItem {
    public:
        MessageFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatterItem {
    public:
        LevelFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            os << ToString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatterItem {
    public:
        ElapseFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            os << event->getElapse();
        }
    };

    class LogNameFormatItem : public LogFormatter::FormatterItem {
    public:
        LogNameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            os << event->getLogger()->getName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatterItem {
    public:
        ThreadIdFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            os << event->getThreadId();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatterItem {
    public:
        FiberIdFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            os << event->getFiberId();
        }
    };

    class ThreadNameFormatItem : public LogFormatter::FormatterItem {
    public:
        ThreadNameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            os << event->getThreadName();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatterItem {
    public:
        DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
                :m_format(format) {
            if(m_format.empty()) {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }

        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }
    private:
        std::string m_format;
    };

    class FilenameFormatItem : public LogFormatter::FormatterItem {
    public:
        FilenameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatterItem {
    public:
        LineFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatterItem {
    public:
        NewLineFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            os << std::endl;
        }
    };


    class StringFormatItem : public LogFormatter::FormatterItem {
    public:
        StringFormatItem(const std::string& str)
                :m_string(str) {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            os << m_string;
        }
    private:
        std::string m_string;
    };

    class TabFormatItem : public LogFormatter::FormatterItem {
    public:
        TabFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel level, LogEvent::ptr event) override {
            os << "\t";
        }
    private:
        std::string m_string;
    };


    LogFormatter::LogFormatter(const std::string& pattern):m_pattern(pattern),m_error(false){
        init();
    }
    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event){
        std::stringstream ss;
        for(auto&& item:m_items){
            item->format(ss,logger,level,event);
        }
        return ss.str();
    }
    std::ostream& LogFormatter::format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel level, LogEvent::ptr event){
        for(auto&& item:m_items){
            item->format(ofs,logger,level,event);
        }
        return ofs;
    }
    bool LogFormatter::error(){
        return m_error;
    }
    /**
     * @brief 格式解析
     * @details
     *  %m 消息
     *  %p 日志级别
     *  %r 累计毫秒数
     *  %c 日志名称
     *  %t 线程id
     *  %n 换行
     *  %d 时间
     *  %f 文件名
     *  %l 行号
     *  %T 制表符
     *  %F 协程id
     *  %N 线程名称
     *
     *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
     */
    void LogFormatter::init(){
        static std::map<char ,std::function<FormatterItem::ptr(const std::string&)>> itemMap{
                {'m', [] (const std::string& str) { return MessageFormatItem::ptr(new MessageFormatItem(str));}},
                {'p', [] (const std::string& str) { return LevelFormatItem::ptr(new LevelFormatItem(str));}},
                {'r', [] (const std::string& str) { return ElapseFormatItem::ptr(new ElapseFormatItem(str));}},
                {'c', [] (const std::string& str) { return LogNameFormatItem::ptr(new LogNameFormatItem(str));}},
                {'t', [] (const std::string& str) { return ThreadIdFormatItem::ptr(new ThreadIdFormatItem(str));}},
                {'n', [] (const std::string& str) { return NewLineFormatItem::ptr(new NewLineFormatItem(str));}},
                {'d', [] (const std::string& str) { return DateTimeFormatItem::ptr(new DateTimeFormatItem(str));}},
                {'f', [] (const std::string& str) { return FilenameFormatItem::ptr(new FilenameFormatItem(str));}},
                {'l', [] (const std::string& str) { return LineFormatItem::ptr(new LineFormatItem(str));}},
                {'T', [] (const std::string& str) { return TabFormatItem::ptr(new TabFormatItem(str));}},
                {'F', [] (const std::string& str) { return FiberIdFormatItem::ptr(new FiberIdFormatItem(str));}},
                {'N', [] (const std::string& str) { return ThreadNameFormatItem::ptr(new ThreadNameFormatItem(str));}},
        };
        ///只支持对时间的格式化
        std::string str;
        int n = m_pattern.size();
        for(int i = 0;i<n;i++){
            if(m_pattern[i] != '%'){
                str.append(1,m_pattern[i]);
                continue;
            }
            i++;
            if(i>=n){
                m_error = true;
                m_items.push_back(StringFormatItem::ptr(new StringFormatItem("<error format %>")));
                continue;
            }
            if(m_pattern[i] == '%'){
                str.append(1,'%');
                continue;
            }
            if(!str.empty()){
                ///%前有一个字符串，可以输出
                m_items.push_back(StringFormatItem::ptr(new StringFormatItem(str)));
                str.clear();
            }
            if(m_pattern[i] != 'd'){
                auto iter = itemMap.find(m_pattern[i]);
                if(iter == itemMap.end()){
                    m_error = true;
                    m_items.push_back(StringFormatItem::ptr(new StringFormatItem("<error format %>")));
                    continue;
                }else{
                    m_items.push_back(iter->second(""));
                }
            }else{
                if(i+1>=n||m_pattern[i+1] != '{'){
                    m_items.push_back(itemMap['d'](""));
                    continue;
                }
                i++;
                auto index = m_pattern.find_first_of('}',i);
                if(index == std::string::npos){
                    m_items.push_back(StringFormatItem::ptr(new StringFormatItem("<error format %d"+m_pattern.substr(i)+">")));
                    m_error = true;
                    continue;
                }else{
                    std::string fmt;
                    ///"{xxxxx}"
                    /// ^     ^
                    /// i     index
                    fmt = m_pattern.substr(i+1,index-i-1);
                    m_items.push_back(DateTimeFormatItem::ptr(new DateTimeFormatItem(fmt)));
                    i = index;
                }
            }
        }
    }

    LogAppender::LogAppender(): m_level(LogLevel::DEBUG), m_has_formatter(false){

    }

    void LogAppender::setFormatter(LogFormatter::ptr format){
        MutexType::Lock lock(m_mutex);
        m_formatter = format;
        if(m_formatter) {
            m_has_formatter = true;
        }else{
            m_has_formatter = false;
        }
    }

    LogFormatter::ptr LogAppender::getFormat(){
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }

    void StdOutLogAppender::log(std::shared_ptr<Logger> logger,LogLevel level,LogEvent::ptr event){
        if(level>=m_level){
            MutexType::Lock lock(m_mutex);
            m_formatter->format(std::cout,logger,level,event);
        }
    }

    FileLogAppender::FileLogAppender(const std::string& filename):m_filename(move(filename)){
        reopen();
    }
    void FileLogAppender::log(std::shared_ptr<Logger> logger,LogLevel level,LogEvent::ptr event){
        time_t now = time(NULL);
        if(m_last_time < static_cast<uint64_t>(now) - 3){
            reopen();
        }
        if(level>=m_level){
            MutexType::Lock lock(m_mutex);
            m_formatter->format(m_filestream,logger,level,event);
        }
    }
    bool FileLogAppender::reopen(){
        MutexType::Lock lock(m_mutex);
        if(m_filestream){
            m_filestream.close();
        }
        FSUtil::OpenFileForWrite(m_filestream,m_filename.c_str(),std::ios_base::app);
        return m_filestream.is_open();
    }

    Logger::Logger(std::string name):m_name(name),m_level(LogLevel::DEBUG){
        m_formatter.reset(new LogFormatter);
    }
    void Logger::log(LogLevel level,LogEvent::ptr event){
        if(level>=m_level){
            Logger::ptr self = shared_from_this();
            MutexType::Lock lock(m_mutex);
            if(m_appenders.empty()){
                m_root->log(level,event);
            }
            else{
                for(auto&& appender:m_appenders){
                    appender->log(self,level,event);
                }
            }
        }
    }

    void Logger::debug(LogEvent::ptr event){
        log(LogLevel::DEBUG,event);
    }
    void Logger::info(LogEvent::ptr event){
        log(LogLevel::INFO,event);
    }
    void Logger::warn(LogEvent::ptr event){
        log(LogLevel::WARN,event);
    }
    void Logger::error(LogEvent::ptr event){
        log(LogLevel::ERROR,event);
    }
    void Logger::fatal(LogEvent::ptr event){
        log(LogLevel::FATAL,event);
    }

    void Logger::addAppender(LogAppender::ptr appender){
        if(!appender){
            return;
        }
        MutexType::Lock lock(m_mutex);
        if(appender->getFormat() == nullptr){
            LogAppender::MutexType::Lock l(appender->m_mutex);
            if(!appender->m_has_formatter){
                appender->m_formatter = m_formatter;
            }
        }
        m_appenders.push_back(appender);
    }
    void Logger::delAppender(LogAppender::ptr appender){
        MutexType::Lock lock(m_mutex);
        auto end = m_appenders.end();
        for(auto iter = m_appenders.begin();iter !=end;iter++){
            if(*iter == appender){
                m_appenders.erase(iter);
                break;
            }
        }
    }
    void Logger::clearAppender(){
        MutexType::Lock lock(m_mutex);
        m_appenders.clear();
    }

    void Logger::setFormatter(LogFormatter::ptr formtter){
        MutexType::Lock lock(m_mutex);
        m_formatter = formtter;

        ///@TODO:没搞清楚m_hasFormatter的作用
        for(auto&& i:m_appenders){
            LogAppender::MutexType::Lock l(i->m_mutex);
            if(!i->m_has_formatter){
                i->m_formatter = formtter;
            }
        }
    }

    void Logger::setFormatter(std::string pattern) {
        LogFormatter::ptr val(new LogFormatter(pattern));
        if(val->error()){
            throw std::logic_error("pattern error");
        }
        setFormatter(val);
    }

    LogFormatter::ptr Logger::getFormatter(){
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }


    LoggerManager::LoggerManager(){
        m_root.reset(new Logger);
        m_root->addAppender(LogAppender::ptr(new StdOutLogAppender));

        m_loggers[m_root->getName()] = m_root;
        init();
    }

    Logger::ptr LoggerManager::getLogger(const std::string& name){
        MutexType::Lock lock(m_mutex);
        auto iter = m_loggers.find(name);
        if(iter != m_loggers.end()){
            return iter->second;
        }
        Logger::ptr logger(new Logger(name));
        logger->setRoot(m_root);
        m_loggers[logger->getName()] = logger;
        return logger;
    }

    void LoggerManager::init(){

    }





}

