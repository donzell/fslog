#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include "Formatter.h"
#include "Log.h"
#include "xmemcpy.h"
using std::string;
using std::vector;


enum {
    INTEGER_STR_MAXLEN=24,
};
enum{
    FMT_RAW_STR=0,
    FMT_LOG_INSTANCE,
    FMT_LOG_LEVEL,
    FMT_FILE,
    FMT_LINE,
    FMT_FUNC,
    FMT_PID,
    FMT_TID,
    FMT_TIME,
    FMT_MSG,
    FMT_LAST,              /* always last one. */
};
enum{
    DETAIL_FMT=1,
    VERBOSE_FMT,
    MIDDLE_FMT,
    SIMPLE_FMT,
};

typedef size_t (*formatter_func)(char* dst,size_t dst_size,const std::string& logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args);

static size_t raw_str_func(char* dst,size_t dst_size,const std::string& logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    return 0;
}
static inline size_t memcpy_minsize(void* dst,const void* src,size_t dst_size,size_t src_size)
{
    size_t copy_size = (dst_size >= src_size ? src_size:dst_size);
    xmemcpy(dst,src,copy_size);
    return copy_size;
}
static size_t log_instance_func(char* dst,size_t dst_size,const std::string& logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    return memcpy_minsize(dst,logInstance.c_str(),dst_size,logInstance.length());
}
static size_t log_level_func(char* dst,size_t dst_size,const std::string& logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    return memcpy_minsize(dst,LEVEL_STR[level],dst_size,strlen(LEVEL_STR[level]));
}
static size_t file_func(char* dst,size_t dst_size,const std::string& logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    return memcpy_minsize(dst,file,dst_size,strlen(file));
}
static size_t func_func(char* dst,size_t dst_size,const std::string& logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    return memcpy_minsize(dst,func,dst_size,strlen(func));
}
static size_t line_func(char* dst,size_t dst_size,const std::string& logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    int lineLen = snprintf(dst,dst_size,"%d",line);
    if(lineLen < 0){
        // error.no graceful way to handle this,i think.
        return 0;
    }
    if(static_cast<size_t>(lineLen) >= dst_size){
        return dst_size;
    }
    return static_cast<size_t>(lineLen);
}
static size_t pid_func(char* dst,size_t dst_size,const std::string& logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    const char* pid_str = GetPidStr();
    size_t pidLen = strlen(pid_str);
    return memcpy_minsize(dst,pid_str,dst_size,pidLen);
}
static size_t tid_func(char* dst,size_t dst_size,const std::string& logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    const char* tid_str = GetTidStr();
    size_t tidLen = strlen(tid_str);
    return memcpy_minsize(dst,tid_str,dst_size,tidLen);
}
static size_t time_func(char* dst,size_t dst_size,const std::string& logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    char *time_str=NULL;
    size_t timeLen=0;
    GetTimeString(&time_str,&timeLen);
    
    return memcpy_minsize(dst,time_str,dst_size,timeLen);
}
static size_t msg_func(char* dst,size_t dst_size,const std::string& logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    int ret = vsnprintf(dst,dst_size,fmt,args);
    if(ret < 0){
        return 0;
    }
    
    if(static_cast<size_t>(ret) >= dst_size){
        return dst_size;
    }
    return static_cast<size_t>(ret);
}

static formatter_func formatter_handler[FMT_LAST]={raw_str_func,
                                                         log_instance_func,
                                                         log_level_func,
                                                         file_func,
                                                         line_func,
                                                         func_func,
                                                         pid_func,
                                                         tid_func,
                                                         time_func,
                                                         msg_func
};

typedef size_t (*special_format_func)(char* dst,size_t dst_size,const char* logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args);
static std::map<string,special_format_func > special_format_func_map;
static inline size_t verbose_format(char* dst,size_t dst_size,const char* logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    //%L %N %T %P:%t %f(%F:%l) %M
    char *time_str=NULL;
    size_t timeLen=0;
    GetTimeString(&time_str,&timeLen);
    int ret = snprintf(dst,dst_size,"%s %s %s %s:%s %s(%s:%d) ",LEVEL_STR[level],logInstance,time_str,GetPidStr(),GetTidStr(),func,file,line);
    if(ret <= 0){
        return 0;
    }
    if(static_cast<size_t>(ret) >= dst_size){
        return dst_size;
    }
    int msg_len = vsnprintf(dst+ret,dst_size-ret,fmt,args);
    if(msg_len < 0){
        return ret;
    }
    if(static_cast<size_t>(msg_len +ret) >= dst_size){
        return dst_size;
    }
    return static_cast<size_t>(ret+msg_len);
}

static inline size_t detail_format(char* dst,size_t dst_size,const char* logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    //%L %T %P:%t %f(%F:%l) %M
    char *time_str=NULL;
    size_t timeLen=0;
    GetTimeString(&time_str,&timeLen);
    int ret = snprintf(dst,dst_size,"%s %s %s:%s %s(%s:%d) ",LEVEL_STR[level],time_str,GetPidStr(),GetTidStr(),func,file,line);
    if(ret <= 0){
        return 0;
    }
    if(static_cast<size_t>(ret) >= dst_size){
        return dst_size;
    }
    int msg_len = vsnprintf(dst+ret,dst_size-ret,fmt,args);
    if(msg_len < 0){
        return ret;
    }
    if(static_cast<size_t>(msg_len +ret) >= dst_size){
        return dst_size;
    }
    return static_cast<size_t>(ret+msg_len);
}

static inline size_t middle_format(char* dst,size_t dst_size,const char* logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    //%L %T %P %f(%F:%l) %M
    char *time_str=NULL;
    size_t timeLen=0;
    GetTimeString(&time_str,&timeLen);
    int ret = snprintf(dst,dst_size,"%s %s %s %s(%s:%d) ",LEVEL_STR[level],time_str,GetPidStr(),func,file,line);
    if(ret <= 0){
        return 0;
    }
    if(static_cast<size_t>(ret) >= dst_size){
        return dst_size;
    }
    int msg_len = vsnprintf(dst+ret,dst_size-ret,fmt,args);
    if(msg_len < 0){
        return ret;
    }
    if(static_cast<size_t>(msg_len +ret) >= dst_size){
        return dst_size;
    }
    return static_cast<size_t>(ret+msg_len);
}

static inline size_t simple_format(char* dst,size_t dst_size,const char* logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    //%L %T %P %F:%l %M
    char *time_str=NULL;
    size_t timeLen=0;
    GetTimeString(&time_str,&timeLen);
    int ret = snprintf(dst,dst_size,"%s %s %s %s:%d ",LEVEL_STR[level],time_str,GetPidStr(),file,line);
    if(ret <= 0){
        return 0;
    }
    if(static_cast<size_t>(ret) >= dst_size){
        return dst_size;
    }
    int msg_len = vsnprintf(dst+ret,dst_size-ret,fmt,args);
    if(msg_len < 0){
        return ret;
    }
    if(static_cast<size_t>(msg_len +ret) >= dst_size){
        return dst_size;
    }
    return static_cast<size_t>(ret+msg_len);
}

// 返回dst_size大小说明最后一个字节是0
size_t Formatter::format(char* dst,size_t dst_size,const char* logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    if(likely(special_format_ == DETAIL_FMT)){
        return detail_format(dst,dst_size,logInstance,file,line,func,level,fmt,args);
    }
    if(likely(special_format_ == VERBOSE_FMT)){
        return verbose_format(dst,dst_size,logInstance,file,line,func,level,fmt,args);
    }
    if(likely(special_format_ == MIDDLE_FMT)){
        return middle_format(dst,dst_size,logInstance,file,line,func,level,fmt,args);
    }
    if(likely(special_format_ == SIMPLE_FMT)){
        return simple_format(dst,dst_size,logInstance,file,line,func,level,fmt,args);
    }
    
    size_t ret=0;
    for (vector<format_t>::iterator it=formats_.begin(); it != formats_.end(); ++it)
    {
        assert(it->type_ >= 0 && it->type_ < FMT_LAST);
        assert(ret <= dst_size);
        ret += memcpy_minsize(dst+ret,it->str_.c_str(),dst_size-ret,it->str_.length());
        if(ret >= dst_size){
            break;
        }
        
        ret += formatter_handler[it->type_](dst+ret,dst_size-ret,logInstance,file,line,func,level,fmt,args);
        if(ret >= dst_size){
            ret = dst_size;
            break;
        }
    }
    return ret;
}

void Formatter::parseFormat()
{
    string::size_type pos = 0;
    string::size_type find_pos = 0;
    string::size_type use;
    string tmp;
    
    use = logfmt_.find('%');
    while(use != string::npos){

        assert(pos <= use);
        format_t fmt;
        if(use+1 != string::npos){
            switch(logfmt_[use+1]){
                case 'L':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    fmt.type_ = FMT_LOG_LEVEL;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 'N':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = FMT_LOG_INSTANCE;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 'P':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = FMT_PID;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 'T':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = FMT_TIME;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 't':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = FMT_TID;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 'l':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = FMT_LINE;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 'F':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = FMT_FILE;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;                    
                    break;
                    
                case 'f':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = FMT_FUNC;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 'M':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }

                    fmt.type_ = FMT_MSG;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                default:
                    find_pos = use+1;
                    break;
            }
            
        }
        else{
            //%L %P %M xxx% 最后一个字符就是%
            fmt.type_ = FMT_RAW_STR;
            fmt.str_ = logfmt_.substr(pos,logfmt_.length()-pos);
            formats_.push_back(fmt);
            break;
        }
        // TODO 还没有处理%%的情况，这个不重要，先不处理。
        use = logfmt_.find('%',find_pos);
    }
    format_t fmt;
    // %L %P %M xx 最后是RAW_STR
    if(pos != string::npos){
        fmt.type_ = FMT_RAW_STR;
        fmt.str_ = logfmt_.substr(pos,logfmt_.length()-pos);
        formats_.push_back(fmt);
    }

    // 最推荐使用detail格式.如果你需要多个instance打到一个文件，推荐第一个verbose。
    //verbose==>%L %N %T %P:%t %f(%F:%l) %M
    //detail==>%L %T %P:%t %f(%F:%l) %M
    //middle==>%L %T %P %f(%F:%l) %M
    //simple==>%L %T %P %F:%l %M
    if(logfmt_ == "%L %N %T %P:%t %f(%F:%l) %M"){
        special_format_ = VERBOSE_FMT;
    }
    else if(logfmt_ == "%L %N %T %P:%t %f(%F:%l) %M"){
        special_format_ = DETAIL_FMT;
    }
    else if(logfmt_ == "%L %T %P %f(%F:%l) %M"){
        special_format_ = MIDDLE_FMT;
    }
    else if(logfmt_ == "%L %T %P %F:%l %M"){
        special_format_ = SIMPLE_FMT;
    }
    else{
        special_format_ = -1;
    }
    
}
