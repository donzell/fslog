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
#include "LogStream.h"
using std::string;
using std::vector;


void Formatter::_formatAccordingToFormats(LogStream& stream,const char* logInstance,const char* file,int line,const char* func,int level,vector<format_t>& formats)
{    
    for (vector<format_t>::iterator it=formats.begin(); it != formats.end(); ++it)
    {
        switch(it->type_){
            case format_t::RAW_STR:
                stream<<it->str_;
                break;
                
            case format_t::LOG_INSTANCE:
                stream<<it->str_;
                stream<<logInstance;
                break;
            
            case format_t::LOG_LEVEL:
                stream<<it->str_;
                stream<<LEVEL_STR[level];
                break;
            
            case format_t::FILE:
                stream<<it->str_;
                stream<<file;
                break;
            
            case format_t::FUNC:
                stream<<it->str_;
                stream<<func;
                break;
            
            case format_t::LINE:
                stream<<it->str_;
                stream<<line;
                break;
                
            case format_t::PID:
                stream<<it->str_;
                stream<<GetPidStr();
                break;
                
            case format_t::TID:
                stream<<it->str_;
                stream<<GetTidStr();
                break;
                
            case format_t::TIME:
                stream<<it->str_;
                do{
                    char *time_str=NULL;
                    size_t timeLen=0;
                    GetTimeString(&time_str,&timeLen);
                    stream<<time_str;
                }while(0);
                break;
                
            default:
                fprintf(stderr,"%s(%s:%d) fatal error,log met unknown format type!",__func__,__FILE__,__LINE__);
                abort();
                break;
        }
    }
}

enum {
    INTEGER_STR_MAXLEN=24,
};
    
size_t Formatter::format(char* dst,size_t dst_size,const char* logInstance,const char* file,int line,const char* func,int level,const char* fmt,va_list args)
{
    size_t ret=0;
    for (vector<format_t>::iterator it=formats_.begin(); it != formats_.end(); ++it)
    {
        assert(ret <= dst_size);

        #define MEMNCPY(dst,src,dstSize,srcSize) do{size_t minLen = (srcSize)<(dstSize)?(srcSize):(dstSize);if(minLen > 0)memcpy(dst,src,minLen);}while(0);

        switch(it->type_){
            case format_t::RAW_STR:
                MEMNCPY(dst+ret,it->str_.c_str(),dst_size-ret,it->str_.length());
                ret += it->str_.length();
                break;
                
            case format_t::LOG_INSTANCE:
            {
                MEMNCPY(dst+ret,it->str_.c_str(),dst_size-ret,it->str_.length());
                ret += it->str_.length();
                size_t logInstanceLen = strlen(logInstance);
                MEMNCPY(dst+ret,logInstance,dst_size-ret,logInstanceLen);
                ret += logInstanceLen;
                break;
            }
            
            case format_t::LOG_LEVEL:
            {
                MEMNCPY(dst+ret,it->str_.c_str(),dst_size-ret,it->str_.length());
                ret += it->str_.length();
                size_t levelLen = strlen(LEVEL_STR[level]);
                MEMNCPY(dst+ret,LEVEL_STR[level],dst_size-ret,levelLen);
                ret += levelLen;
                break;
            }
            
            case format_t::FILE:
            {
                MEMNCPY(dst+ret,it->str_.c_str(),dst_size-ret,it->str_.length());
                ret += it->str_.length();
                size_t fileLen = strlen(file);
                MEMNCPY(dst+ret,file,dst_size-ret,fileLen);
                ret += fileLen;
                break;
            }
            
            case format_t::FUNC:
            {
                MEMNCPY(dst+ret,it->str_.c_str(),dst_size-ret,it->str_.length());
                ret += it->str_.length();
                size_t funcLen = strlen(func);
                MEMNCPY(dst+ret,func,dst_size-ret,funcLen);
                ret += funcLen;
                
                break;
            }
            
            case format_t::LINE:
                MEMNCPY(dst+ret,it->str_.c_str(),dst_size-ret,it->str_.length());
                ret += it->str_.length();
                do{
                    char line_str[INTEGER_STR_MAXLEN]={0};
                    size_t lineLen = static_cast<size_t>(snprintf(line_str,sizeof(line_str),"%d",line));
                    MEMNCPY(dst+ret,line_str,dst_size-ret,lineLen);
                    ret += lineLen;
                }while(0);
                break;
                
            case format_t::PID:
                MEMNCPY(dst+ret,it->str_.c_str(),dst_size-ret,it->str_.length());
                ret += it->str_.length();
                do{
                    const char* pid_str = GetPidStr();
                    size_t pidLen = strlen(pid_str);
                    MEMNCPY(dst+ret,pid_str,dst_size-ret,pidLen);
                    ret += pidLen;
                }while(0);
                break;
                
            case format_t::TID:
                MEMNCPY(dst+ret,it->str_.c_str(),dst_size-ret,it->str_.length());
                ret += it->str_.length();
                do{
                    const char *tid_str=GetTidStr();
                    size_t tidLen = strlen(tid_str);
                    MEMNCPY(dst+ret,tid_str,dst_size-ret,tidLen);
                    ret += tidLen;
                }while(0);
                break;
                
            case format_t::TIME:
                MEMNCPY(dst+ret,it->str_.c_str(),dst_size-ret,it->str_.length());
                ret += it->str_.length();
                do{
                    char *time_str=NULL;
                    size_t timeLen=0;
                    GetTimeString(&time_str,&timeLen);
                    MEMNCPY(dst+ret,time_str,dst_size-ret,timeLen);
                    ret += timeLen;
                }while(0);
                break;
                
            case format_t::MSG:
                MEMNCPY(dst+ret,it->str_.c_str(),dst_size-ret,it->str_.length());
                ret += it->str_.length();
                do{
                    int s_ret = vsnprintf(dst+ret,dst_size-ret,fmt,args);
                    ret += s_ret;
                }while(0);
                break;
                
            default:
                fprintf(stderr,"fatal error,log met unknown format type!");
                abort();
                break;
        }
        
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
                    fmt.type_ = format_t::LOG_LEVEL;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 'N':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = format_t::LOG_INSTANCE;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 'P':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = format_t::PID;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 'T':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = format_t::TIME;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 't':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = format_t::TID;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 'l':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = format_t::LINE;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 'F':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = format_t::FILE;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;                    
                    break;
                    
                case 'f':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }
                    
                    fmt.type_ = format_t::FUNC;
                    formats_.push_back(fmt);
                    pos = use+2;
                    find_pos = pos;
                    break;
                    
                case 'M':
                    tmp = logfmt_.substr(pos,use-pos);
                    if(!tmp.empty()){
                        fmt.str_ = tmp;
                    }

                    fmt.type_ = format_t::MSG;
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
            fmt.type_ = format_t::RAW_STR;
            fmt.str_ = logfmt_.substr(pos,logfmt_.length()-pos);
            formats_.push_back(fmt);
            break;
        }
        
        use = logfmt_.find('%',find_pos);
    }
    format_t fmt;
    // %L %P %M xx 最后是RAW_STR
    if(pos != string::npos){
        fmt.type_ = format_t::RAW_STR;
        fmt.str_ = logfmt_.substr(pos,logfmt_.length()-pos);
        formats_.push_back(fmt);
    }
    // 复制到两个格式化列表中，分别是%M之前和之后的
    bool metMsg = false;
    for (vector<format_t>::iterator it=formats_.begin(); it != formats_.end(); ++it)
    {
        if(it->type_ != format_t::MSG){
            if(!metMsg){
                beforeMsgFormats_.push_back(*it);
            }
            else{
                    afterMsgFormats_.push_back(*it);
            }
        }
        else{
            hasMsg_ = true;
        }
    }
}
