/**
 * @file   logger_c.cc
 * @author donzell <donzell.wu@gmail.com>
 * @date   Mon Jul  8 17:56:02 2013
 * 
 * @brief  
 * 
 * 
 */

#include "Log.h"
#include "logger_c.h"


/// logger loader
int InitLoggers_C(const char* conf)
{
    return CLogger::init(conf);
}


///根据logger的名字获取logger的实例指针；
logger_c_t GetLoggerInstance(const char* loggername)
{
    return &CLogger::GetInstance(loggername);
}

///设置该级别是否需要进行log
int CanLog(logger_c_t logger, int level)
{
    if (NULL == logger)
    {
        return 0;
    }
    if ( ((CLogger*)logger)->canLog(level) )
    {
        return 1;
    }
    return 0;
}

///设置logger级别
void SetLoggerLevel(logger_c_t logger, int level)
{
    if (NULL == logger)
    {
        return;
    }
    ((CLogger*)logger)->setLogLevel(level);
}

/// 辅助函数，使用logger_c_t打log
void Logger_Out(logger_c_t logger,int level,const char* file,const char* func,int line, const char* format, ...)
{
    va_list args; 
    va_start(args, format); 
    ((CLogger*)logger)->writeLog(file,line,func,level,format,args);
    va_end(args); 
}



