/**
 * @file   Log.h
 * @author wudongzheng01 <donzell.wu@gmail.com>
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

using std::map;
using std::string;
using std::make_pair;
using std::pair;


class CLogger;
typedef CLogger* LoggerPtr;
#define WFLOGINSTANCE "__LOG_WARN_FATLE__"

enum
{
    FATAL=0,
    WARN,
    ERROR,
    NOTICE,
    TRACE,
    LOG,
    INFO,
    DEBUG,
    LEVEL_ALL,
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
    static LoggerPtr getLogInstance(const string& logname);

    static LoggerPtr getWfLogInstance()
    {
        return pWfLogger_;
    }

    void setLogLevel(int level){level_ = level;}
    int getLogLevel()const{return level_;}
    inline bool canLog(int level)
    {return level <= level_;}
    
    void setLogName(const string& logname)
    {logname_ = logname;}
    
    const string& getLogName() const {return logname_;}
    
    Appender* getAppender()const
    {
        return pAppender_;
    }
    
    Formatter& getFormatter(){return formatter_;}
    
    
    void writeLog(const string& logName,const char* file,int line,const char* func,int level,const char* fmt,...)__attribute__((format(printf,7,8)));
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
    
    
    string logname_;
    // CLogAppender* appender_;
    Formatter formatter_;
    volatile int level_;
    Appender* pAppender_;
    
    static map<string,CLogger*> logMap_;
    typedef map<string,CLogger*>::iterator  logMapIter;
    static boost::mutex mutex_;
    static map<string,Appender*> appenderMap_;
    static CLogger* pWfLogger_;
};

#define LOG_FATAL(logger,fmt,...)                                       \
    do{                                                                 \
        if(logger && logger->canLog(FATAL))                             \
            logger->writeLog(logger->getLogName(),__FILE__,__LINE__,__func__,FATAL,fmt,##__VA_ARGS__); \
    }while(0)


#define LOG_WARN(logger,fmt,...)                                        \
    do{                                                                 \
        if(logger && logger->canLog(WARN))                              \
            logger->writeLog(logger->getLogName(),__FILE__,__LINE__,__func__,WARN,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#define LOG_ERROR(logger,fmt,...)                                       \
    do{                                                                 \
        if(logger && logger->canLog(ERROR))                             \
            logger->writeLog(logger->getLogName(),__FILE__,__LINE__,__func__,ERROR,fmt,##__VA_ARGS__); \
    }while(0)


#define LOG_TRACE(logger,fmt,...)                                       \
    do{                                                                 \
        if(logger && logger->canLog(TRACE))                             \
            logger->writeLog(logger->getLogName(),__FILE__,__LINE__,__func__,TRACE,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#define LOG_NOTICE(logger,fmt,...)                                      \
    do{                                                                 \
        if(logger && logger->canLog(NOTICE))                            \
            logger->writeLog(logger->getLogName(),__FILE__,__LINE__,__func__,NOTICE,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#define LOG_LOG(logger,fmt,...)                                         \
    do{                                                                 \
        if(logger && logger->canLog(LOG))                               \
            logger->writeLog(logger->getLogName(),__FILE__,__LINE__,__func__,LOG,fmt,##__VA_ARGS__); \
    }while(0)

#define LOG_INFO(logger,fmt,...)                                        \
    do{                                                                 \
        if(logger && logger->canLog(INFO))                              \
            logger->writeLog(logger->getLogName(),__FILE__,__LINE__,__func__,INFO,fmt,##__VA_ARGS__); \
    }while(0)


#define LOG_DEBUG(logger,fmt,...)                                       \
    do{                                                                 \
        if(logger && logger->canLog(DEBUG))                             \
            logger->writeLog(logger->getLogName(),__FILE__,__LINE__,__func__,DEBUG,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#endif
