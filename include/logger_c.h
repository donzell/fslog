/**
 * @file   logger_c.h
 * @author donzell <donzell.wu@gmail.com>
 * @date   Mon Jul  8 17:32:28 2013
 * 
 * @brief  
 * 
 * 
 */

#ifndef _LOGGER_C_H_
#define _LOGGER_C_H_

#include <stdlib.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef void* logger_c_t;
/* 要和Log.h保持一致 */
enum
{
    LEVEL_NONE=0,
    LEVEL_FATAL,
    LEVEL_WARN,
    LEVEL_ERROR,
    LEVEL_NOTICE,
    LEVEL_TRACE,
    LEVEL_LOG,
    LEVEL_INFO,
    LEVEL_DEBUG,
    LEVEL_LEVEL_ALL=100,
};

/** logger loader */
int InitLoggers_C(const char* conf);


/** 根据logger的名字获取logger的实例指针 */
logger_c_t GetLoggerInstance(const char* loggername);

/** 设置log级别 */
void SetLoggerLevel(logger_c_t logger, int level);

/** 设置该级别是否需要进行log */
int CanLog(logger_c_t logger, int level);

void Logger_Out(logger_c_t logger,int level,const char* file,const char* fun,int line, const char* format, ...)__attribute__((format(printf,6,7)));

/** ------------------------------------------------------------------------------------------ */

#define Logger_Info(logger, fmt, ...) \
	do \
	{ \
	if(logger!=NULL && CanLog(logger,LEVEL_INFO)) \
		Logger_Out(logger,LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
	} while(0)

#define Logger_Debug(logger, fmt, ...) \
	do \
	{ \
	if(logger!=NULL && CanLog(logger,LEVEL_DEBUG)) \
		Logger_Out(logger,LEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
	} while(0)

#define Logger_Log(logger, fmt, ...) \
	do \
	{ \
	if(logger!=NULL && CanLog(logger,LEVEL_LOG)) \
		Logger_Out(logger,LEVEL_LOG, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
	} while(0)

#define Logger_Notice(logger, fmt, ...) \
	do \
	{ \
	if(logger!=NULL && CanLog(logger,LEVEL_NOTICE)) \
		Logger_Out(logger,LEVEL_NOTICE, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
	} while(0)

#define Logger_Error(logger, fmt, ...) \
	do \
	{ \
	if(logger!=NULL && CanLog(logger,LEVEL_ERROR)) \
		Logger_Out(logger,LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
	} while(0)

#define Logger_Warn(logger, fmt, ...) \
	do \
	{ \
	if(logger!=NULL && CanLog(logger,LEVEL_WARN)) \
		Logger_Out(logger,LEVEL_WARN, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
	} while(0)

#define Logger_Fatal(logger, fmt, ...) \
	do \
	{ \
	if(logger!=NULL && CanLog(logger,LEVEL_FATAL)) \
		Logger_Out(logger,LEVEL_FATAL, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__); \
	} while(0)

#ifdef  __cplusplus
}
#endif


#endif /* _LOGGER_C_H_ */
