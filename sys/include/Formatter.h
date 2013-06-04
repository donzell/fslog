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
    int type_;
    std::string str_;
}format_t;
    

class Formatter
{
  public:
  Formatter(const std::string& logfmt)
      :logfmt_(logfmt)
    {    
        parseFormat();
    }
    
    ~Formatter()
    {}

    /* return would format if size enough.if retval >= sizeof(outputstr),str is truncated. */
    size_t format(char* dst,size_t dst_size,const std::string& logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args);

  private:
    void parseFormat();
    
    std::string logfmt_;
    std::vector<format_t> formats_;
};
    

#endif /* _FORMATTER_H_ */
