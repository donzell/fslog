/**
 * @file   Log.h
 * @author donzell <donzell.wu@gmail.com>
 * @date   Sat Sep 22 20:42:59 2012
 * 
 * @brief  小型简单的日志库，能够按大小切分日志，支持多线程、多进程写同一个日志文件，尽可能的高效，不要让Log成为性能瓶颈。
 * 可以动态设置某一个log实例的level，但是其他的配置就不能动态改变了；可以拥有多个log实例；日志格式可配置;支持常见的level;
 * 
 * 
 */
#ifndef _LOG_H_
#define _LOG_H_

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <string>
#include <map>
#include "Formatter.h"
#include "Appender.h"
#include "FileAppender.h"
#include "AsyncFileAppender.h"
#include "Atomic.h"

using std::map;
using std::string;
using std::make_pair;
using std::pair;


class CLogger;
typedef CLogger* LoggerPtr;
#define WFLOGINSTANCE "__LOG_WARN_FATLE__"

enum
{
    NONE=0,
    FATAL,
    WARN,
    ERROR,
    NOTICE,
    TRACE,
    LOG,
    INFO,
    DEBUG,
    LEVEL_ALL=100,
};
extern const char* LEVEL_STR[LEVEL_ALL];


class CLogger:boost::noncopyable
{
  public:
    enum{
        MAX_SIZE_PER_LOG=4096,
    };
    
    /** 
     * 只应该初始化一次
     * 
     * @param path 配置文件路径
     * 
     * @return 成功0，其他失败
     */
    static int32_t init(const string& path);
    /** 
     * 线程安全的获取一个Log实例
     * 
     * @param logname 
     * 
     * @return 
     */
    static CLogger&  GetInstance(const string& logname);

    static CLogger& getWfLogInstance()
    {
        return *pWfLogger_;
    }

    void setLogLevel(int level){level_ = level;}
    int getLogLevel()const{return level_;}
    inline bool canLog(int level)
    {return level <= level_;}
    
    void setLogName(const string& logname)
    {
        if(logname_){
            free(logname_);
        }
        
        logname_ = strdup(logname.c_str());
    }
    
    const char* getLogName() const {return logname_;}
    
    Appender* getAppender()const
    {
        return pAppender_;
    }
    
    Formatter& getFormatter(){return formatter_;}
    
    void writeLog(const char* file,int line,const char* func,int level,const char* fmt,va_list args);
    void writeLog(const char* file,int line,const char* func,int level,const char* fmt,...)__attribute__((format(printf,6,7)));
    void output(char* msg,size_t size);

  private:
    CLogger(const string& logname,Appender* pAppender,const string& fmt,int level);
    ~CLogger();
    static void setWfLogInstance(LoggerPtr wfLogger)
    {
        pWfLogger_ = wfLogger;
    }
    
    static void destroy();
    

    // 1.按分割符切割字段
    // 2.trim各个字段
    // 3.第一、二、三个字段不能为空，第四是切分大小，默认2G，第五是level，默认全部。    
    static std::vector<std::string> splitCheck(const std::string& instance);
    
    
    char *logname_;
    // CLogAppender* appender_;
    Formatter formatter_;
    volatile int level_;
    Appender* pAppender_;

    static CLogger* dummyLogger;
    
    static map<string,CLogger*> logMap_;
    typedef map<string,CLogger*>::iterator  logMapIter;
    static boost::mutex mutex_;
    static map<string,Appender*> appenderMap_;
    static CLogger* pWfLogger_;
};

#define LOGGER_FATAL(logger,fmt,...)                                    \
    do{                                                                 \
        LoggerPtr wfLogger = CLogger::getWfLogInstance();               \
        if(wfLogger && wfLogger->canLog(level)){                        \
            wfLogger->writeLog(__FILE__,__LINE__,__func__,FATAL,fmt,##__VA_ARGS__); \
        }                                                               \
        if(logger.canLog(FATAL)){                                       \
            logger.writeLog(__FILE__,__LINE__,__func__,FATAL,fmt,##__VA_ARGS__); \
        }                                                               \
    }while(0)


#define LOGGER_WARN(logger,fmt,...)                                     \
    do{                                                                 \
        LoggerPtr wfLogger = CLogger::getWfLogInstance();               \
        if(wfLogger && wfLogger->canLog(level)){                        \
            wfLogger->writeLog(,__FILE__,__LINE__,__func__,FATAL,fmt,##__VA_ARGS__); \
        }                                                               \
        if(logger.canLog(WARN)){                                        \
            logger.writeLog(,__FILE__,__LINE__,__func__,WARN,fmt,##__VA_ARGS__); \
        }                                                               \
    }while(0)

#define LOGGER_ERROR(logger,fmt,...)                                    \
    do{                                                                 \
        if(logger.canLog(ERROR))                                        \
            logger.writeLog(__FILE__,__LINE__,__func__,ERROR,fmt,##__VA_ARGS__); \
    }while(0)


#define LOGGER_TRACE(logger,fmt,...)                                    \
    do{                                                                 \
        if(logger.canLog(TRACE))                                        \
            logger.writeLog(__FILE__,__LINE__,__func__,TRACE,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#define LOGGER_NOTICE(logger,fmt,...)                                   \
    do{                                                                 \
        if(logger.canLog(NOTICE))                                       \
            logger.writeLog(__FILE__,__LINE__,__func__,NOTICE,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#define LOGGER_LOG(logger,fmt,...)                                      \
    do{                                                                 \
        if(logger.canLog(LOG))                                          \
            logger.writeLog(__FILE__,__LINE__,__func__,LOG,fmt,##__VA_ARGS__); \
    }while(0)

#define LOGGER_INFO(logger,fmt,...)                                     \
    do{                                                                 \
        if(logger.canLog(INFO))                                         \
            logger.writeLog(__FILE__,__LINE__,__func__,INFO,fmt,##__VA_ARGS__); \
    }while(0)


#define LOGGER_DEBUG(logger,fmt,...)                                    \
    do{                                                                 \
        if(logger.canLog(DEBUG))                                        \
            logger.writeLog(__FILE__,__LINE__,__func__,DEBUG,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#define LOGGER_X_IF(logger, condition, level, fmt, ...)                 \
    do {                                                                \
        if (condition)                                                  \
            if (logger.canLog(level))                                   \
                logger.writeLog(__FILE__, __LINE__, __func__, level, fmt, ##__VA_ARGS__); \
    }                                                                   \
    while (0)

#define LOGGER_X_EVERY_N(logger, n, level, fmt, ...)                    \
    do {                                                                \
        if (logger.canLog(level)){                                      \
            static AtomicInt64 counter;                                 \
            if (counter.increment() % static_cast<int64_t>(n) == 0)     \
                logger.writeLog(__FILE__, __LINE__, __func__, level, fmt, ##__VA_ARGS__); \
        }                                                               \
    }                                                                   \
    while (0)


#define LOGGER_FATAL_EVERY_N(logger, n, fmt, ...) LOGGER_X_EVERY_N(logger, n, FATAL, fmt, ##__VA_ARGS__)
#define LOGGER_WARN_EVERY_N(logger, n, fmt, ...) LOGGER_X_EVERY_N(logger, n, WARN, fmt, ##__VA_ARGS__)
#define LOGGER_ERROR_EVERY_N(logger, n, fmt, ...) LOGGER_X_EVERY_N(logger, n, ERROR, fmt, ##__VA_ARGS__)
#define LOGGER_NOTICE_EVERY_N(logger, n, fmt, ...) LOGGER_X_EVERY_N(logger, n, NOTICE, fmt, ##__VA_ARGS__)
#define LOGGER_TRACE_EVERY_N(logger, n, fmt, ...) LOGGER_X_EVERY_N(logger, n, TRACE, fmt, ##__VA_ARGS__)
#define LOGGER_LOG__EVERY_N(logger, n, fmt, ...) LOGGER_X_EVERY_N(logger, n, LOG, fmt, ##__VA_ARGS__)
#define LOGGER_INFO_EVERY_N(logger, n, fmt, ...) LOGGER_X_EVERY_N(logger, n, INFO, fmt, ##__VA_ARGS__)
#define LOGGER_DEBUG_EVERY_N(logger, n, fmt, ...) LOGGER_X_EVERY_N(logger, n, DEBUG, fmt, ##__VA_ARGS__)

#define LOGGER_FATAL_IF(logger, condition, fmt, ...) LOGGER_X_IF(logger, condition, FATAL, fmt, ##__VA_ARGS__)
#define LOGGER_WARN_IF(logger, condition, fmt, ...) LOGGER_X_IF(logger, condition, WARN, fmt, ##__VA_ARGS__)
#define LOGGER_ERROR_IF(logger, condition, fmt, ...) LOGGER_X_IF(logger, condition, ERROR, fmt, ##__VA_ARGS__)
#define LOGGER_NOTICE_IF(logger, condition, fmt, ...) LOGGER_X_IF(logger, condition, NOTICE, fmt, ##__VA_ARGS__)
#define LOGGER_TRACE_IF(logger, condition, fmt, ...) LOGGER_X_IF(logger, condition, TRACE, fmt, ##__VA_ARGS__)
#define LOGGER_LOG_IF(logger, condition, fmt, ...) LOGGER_X_IF(logger, condition, LOG, fmt, ##__VA_ARGS__)
#define LOGGER_INFO_IF(logger, condition, fmt, ...) LOGGER_X_IF(logger, condition, INFO, fmt, ##__VA_ARGS__)
#define LOGGER_DEBUG_IF(logger, condition, fmt, ...) LOGGER_X_IF(logger, condition, DEBUG, fmt, ##__VA_ARGS__)


#endif
