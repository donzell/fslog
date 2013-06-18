/**
 * @file   Log.h
 * @author wudongzheng01 <donzell.wu@gmail.com>
 * @date   Sat Sep 22 20:42:59 2012
 * 
 * @brief  С�ͼ򵥵���־�⣬�ܹ�����С�з���־��֧�ֶ��̡߳������дͬһ����־�ļ��������ܵĸ�Ч����Ҫ��Log��Ϊ����ƿ����
 * ���Զ�̬����ĳһ��logʵ����level���������������þͲ��ܶ�̬�ı��ˣ�����ӵ�ж��logʵ������־��ʽ������;֧�ֳ�����level;
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
     * ֻӦ�ó�ʼ��һ��
     * 
     * @param path �����ļ�·��
     * 
     * @return �ɹ�0������ʧ��
     */
    static int32_t init(const string& path);
    /** 
     * �̰߳�ȫ�Ļ�ȡһ��Logʵ��
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
    

    // 1.���ָ���и��ֶ�
    // 2.trim�����ֶ�
    // 3.��һ�����������ֶβ���Ϊ�գ��������зִ�С��Ĭ��2G��������level��Ĭ��ȫ����    
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
        if(logger.canLog(FATAL)){                            \
            logger.writeLog(__FILE__,__LINE__,__func__,FATAL,fmt,##__VA_ARGS__); \
        }                                                               \
    }while(0)


#define LOGGER_WARN(logger,fmt,...)                                     \
    do{                                                                 \
        LoggerPtr wfLogger = CLogger::getWfLogInstance();               \
        if(wfLogger && wfLogger->canLog(level)){                        \
            wfLogger->writeLog(,__FILE__,__LINE__,__func__,FATAL,fmt,##__VA_ARGS__); \
        }                                                               \
        if(logger.canLog(WARN)){                             \
            logger.writeLog(,__FILE__,__LINE__,__func__,WARN,fmt,##__VA_ARGS__); \
        }                                                               \
    }while(0)

#define LOGGER_ERROR(logger,fmt,...)                                    \
    do{                                                                 \
        if(logger.canLog(ERROR))                             \
            logger.writeLog(__FILE__,__LINE__,__func__,ERROR,fmt,##__VA_ARGS__); \
    }while(0)


#define LOGGER_TRACE(logger,fmt,...)                                    \
    do{                                                                 \
        if(logger.canLog(TRACE))                             \
            logger.writeLog(__FILE__,__LINE__,__func__,TRACE,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#define LOGGER_NOTICE(logger,fmt,...)                                   \
    do{                                                                 \
        if(logger.canLog(NOTICE))                            \
            logger.writeLog(__FILE__,__LINE__,__func__,NOTICE,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#define LOGGER_LOG(logger,fmt,...)                                      \
    do{                                                                 \
        if(logger.canLog(LOG))                               \
            logger.writeLog(__FILE__,__LINE__,__func__,LOG,fmt,##__VA_ARGS__); \
    }while(0)

#define LOGGER_INFO(logger,fmt,...)                                     \
    do{                                                                 \
        if(logger.canLog(INFO))                              \
            logger.writeLog(__FILE__,__LINE__,__func__,INFO,fmt,##__VA_ARGS__); \
    }while(0)


#define LOGGER_DEBUG(logger,fmt,...)                                    \
    do{                                                                 \
        if(logger.canLog(DEBUG))                             \
            logger.writeLog(__FILE__,__LINE__,__func__,DEBUG,fmt,##__VA_ARGS__); \
    }                                                                   \
    while(0)

#endif