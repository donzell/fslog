#ifndef _FORMATTER_H_
#define _FORMATTER_H_

#include <string>
#include <vector>
#include <stdarg.h>
#include <sys/types.h>
#include <pthread.h>
#include "StrUtil.h"

typedef struct format
{
    enum{
        RAW_STR,
        LOG_INSTANCE,
        LOG_LEVEL,
        FILE,
        LINE,
        FUNC,
        PID,
        TID,
        TIME,
        MSG,
    };
    
    int type_;
    std::string str_;
}format_t;

class LogStream;

class Formatter
{
  public:
  Formatter(const std::string& logfmt)
        :logfmt_(logfmt),hasMsg_(false)
    {    
        parseFormat();
    }
    
    ~Formatter()
    {}
    

    void formatBeforeMsg(LogStream& stream,const char* logInstance,const char* file,int line,const char* func,int level)
    {
        _formatAccordingToFormats(stream,logInstance,file,line,func,level,beforeMsgFormats_);
    }
    
    void Formatter::formatAfterMsg(LogStream& stream,const char* logInstance,const char* file,int line,const char* func,int level)
    {
        if(likely(afterMsgFormats_.empty())){
            return;
        }
        _formatAccordingToFormats(stream,logInstance,file,line,func,level,afterMsgFormats_);
    }
    bool hasMsg()const{return hasMsg_;}
    /* return would format if size enough.if retval >= sizeof(outputstr),str is truncated. */
    size_t format(char* dst,size_t dst_size,const char* logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args);

  private:
    void parseFormat();
    void _formatAccordingToFormats(LogStream& stream,const char* logInstance,const char* file,int line,const char* func,int level,std::vector<format_t>& formats);
    
    std::string logfmt_;
    std::vector<format_t> formats_;
    std::vector<format_t> beforeMsgFormats_;
    std::vector<format_t> afterMsgFormats_;
    bool hasMsg_;
};
    

#endif /* _FORMATTER_H_ */
