#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <sstream>
#include "FileAppender.h"
#include "StrUtil.h"
using namespace std;

FileAppender::FileAppender(const string& path,uint64_t splitsize,const string& splitFormat)
    :Appender(path),path_(path),splitSize_(splitsize),inode_(0),checkInterval_(10),loopCounter_(0),checkTimeInterval_(1),lastcheck_(0),fd_(-1),splitFormat_(splitFormat)
{
    reopen();
    if(fd_ < 0){
        // 只会尝试创建缺少的单级目录.比如/home/work/log/test.log，只会尝试创建log目录
        struct stat st;
        if(stat(path_.c_str(),&st) < 0){
            size_t pos = path_.rfind("/");
            string dirpath;
            if(pos != string::npos){
                dirpath = path_.substr(0,pos);
            }
            if(!dirpath.empty() && stat(dirpath.c_str(),&st) < 0){
                mkdir(dirpath.c_str(),0777);
            }
        }
        reopen();
    }
    
    struct stat buf;
    fstat(fd_,&buf);
    inode_ = buf.st_ino;
    if(splitSize_ <= 0){
        splitSize_ = DEFAULT_SPLITSIZE;
    }
    else if(splitSize_ < MIN_SPLITSIZE){
        splitSize_ = MIN_SPLITSIZE;
    }
    else if(splitSize_ > MAX_SPLITSIZE){
        splitSize_ = MAX_SPLITSIZE;
    }

    generateSplitFormatCharMap();
    parseSplitFormat();
}

void FileAppender::generateSplitFormatCharMap()
{
    formatsCharMap_['Y']=format_t::YEAR;
    formatsCharMap_['m']=format_t::MONTH;
    formatsCharMap_['d']=format_t::DAY;
    formatsCharMap_['H']=format_t::HOUR;
    formatsCharMap_['M']=format_t::MINUTE;
    formatsCharMap_['S']=format_t::SECOND;
    formatsCharMap_['P']=format_t::PID;
    formatsCharMap_['n']=format_t::SEQNUMBER;
}

void FileAppender::parseSplitFormat()
{
    
    string::size_type pos = 0;
    string::size_type find_pos = 0;
    string::size_type use;
    string tmp;
    
    use = splitFormat_.find('%');
    while(use != string::npos){
        assert(pos <= use);
        format_t fmt;
        if(use+1 != string::npos){
            charToFormatMap::iterator it=formatsCharMap_.find(splitFormat_[use+1]);
            if(it!=formatsCharMap_.end()){
                tmp = splitFormat_.substr(pos,use-pos);
                if(!tmp.empty()){
                    fmt.str_ = tmp;
                }
                fmt.type_ = formatsCharMap_[splitFormat_[use+1]];
                formats_.push_back(fmt);
                pos = use+2;
                find_pos = pos;
            }
            else{
                find_pos = use+1;
            }
        }
        else{
            //%H%m%d_xxx% 最后一个字符就是%
            fmt.type_ = format_t::RAW_STR;
            fmt.str_ = splitFormat_.substr(pos,splitFormat_.length()-pos);
            formats_.push_back(fmt);
            break;
        }
        
        use = splitFormat_.find('%',find_pos);
    }
    format_t fmt;
    // %H%m%dxx 最后是RAW_STR
    if(pos != string::npos){
        fmt.type_ = format_t::RAW_STR;
        fmt.str_ = splitFormat_.substr(pos,splitFormat_.length()-pos);
        formats_.push_back(fmt);
    }
}

FileAppender::~FileAppender()
{
    if(fd_ >= 0)
        ::close(fd_);
}

void FileAppender::reopen()
{
    // int fd = ::open(path_.c_str(),O_CREAT|O_WRONLY|O_APPEND,0755);
    // int oldFd = __sync_lock_test_and_set(&fd_,fd);
    // if(oldFd >= 0)
    //     ::close(oldFd);
    // }
    if(fd_ >= 0){
        ::close(fd_);
    }
    fd_ = ::open(path_.c_str(),O_CREAT|O_WRONLY|O_APPEND,0755);
}

string FileAppender::getNewFilename(time_t now,int seqNumber)const
{
    struct tm newTimeObj;
    localtime_r(&now,&newTimeObj);
    char tmpStr[16]={0};
    
    // 如同date命令的格式%Y%m%dH%M%S
    // %P进程号，%n序列号，从000开始，如果同名文件存在，自动累加直到找到一个可用名字，上限999. example: a.log_%Y%m%d%H%M%S_%P_%n,如果a.log.20121010090048_3164_000被占用，会去看a.log.20121010090048_3164_001是否可用
    stringstream stream;
    stream<<path_;
    
    for (vector<format_t>::const_iterator it=formats_.begin(); it != formats_.end(); ++it)
    {
        switch(it->type_){
            case format_t::RAW_STR:
                stream<<it->str_;
                break;
                
            case format_t::YEAR:
                snprintf(tmpStr,sizeof(tmpStr),"%4d",newTimeObj.tm_year+1900);
                stream<<it->str_;
                stream<<tmpStr;
                break;
            
            case format_t::MONTH:
                snprintf(tmpStr,sizeof(tmpStr),"%02d",newTimeObj.tm_mon+1);
                stream<<it->str_;
                stream<<tmpStr;
                break;

            case format_t::DAY:
                snprintf(tmpStr,sizeof(tmpStr),"%02d",newTimeObj.tm_mday);
                stream<<it->str_;
                stream<<tmpStr;
                break;

            case format_t::HOUR:
                snprintf(tmpStr,sizeof(tmpStr),"%02d",newTimeObj.tm_hour);
                stream<<it->str_;
                stream<<tmpStr;
                break;

            case format_t::MINUTE:
                snprintf(tmpStr,sizeof(tmpStr),"%02d",newTimeObj.tm_min);
                stream<<it->str_;
                stream<<tmpStr;
                break;

            case format_t::SECOND:
                snprintf(tmpStr,sizeof(tmpStr),"%02d",newTimeObj.tm_sec);
                stream<<it->str_;
                stream<<tmpStr;
                break;

            case format_t::PID:
                snprintf(tmpStr,sizeof(tmpStr),"%d",getpid());
                stream<<it->str_;
                stream<<tmpStr;
                break;

            case format_t::SEQNUMBER:
                snprintf(tmpStr,sizeof(tmpStr),"%03d",seqNumber);
                stream<<it->str_;
                stream<<tmpStr;
                break;

            default:
                fprintf(stderr,"%s(%s:%d) fatal error,fileAppender met unknown format type!",__func__,__FILE__,__LINE__);
                abort();
                break;
        }
    }
    return stream.str();
}

void FileAppender::checkFile()
{
    // 隔一定次数检查一次是否需要重新打开，每次都检查效率太低.文件切分大小有误差，这个问题不大
    time_t now = time(NULL);
    if((++loopCounter_ < checkInterval_) && (lastcheck_ + checkTimeInterval_ < now)){
        return;
    }
    
    lastcheck_ = now;
    loopCounter_ = 0;
        
// 1.检查当前fd是否还指向log文件，不是则说明被mv或者rm了，重新open(path,O_CREAT|O_WRONLY|O_APPEND,0755)
// 2.检查log文件的大小，如果已经超过切分大小，mv，open
    struct stat buf;
    memset(&buf,0,sizeof(buf));
    
    if(::stat(path_.c_str(),&buf) < 0){
        // 不应该没有权限，除非用户主动对文件或目录chmod了
        // 可能文件刚被mv，还没来得及create，这个应该很少见
        reopen();
        return;
    }
    
    
    if(fd_ < 0 || inode_ != buf.st_ino){
        uint64_t saved_ino = buf.st_ino;
        
        // fd < 0，或者inode已经变了，重新打开
        reopen();
        if(fd_ >= 0){
            struct stat dbuf;
            if(fstat(fd_,&dbuf) < 0){
                // 怎么会呢?
                inode_ = saved_ino;
            }
            else{
                inode_ = dbuf.st_ino;
            }
        }
        else{
            reopen();
        }
        return;
    }

    if(static_cast<uint64_t>(buf.st_size) >= splitSize_){
        // mv reopen.
        string newFileName;
        bool foundFileName=false;//是否找到一个可用的文件名
        time_t now=time(NULL);
        for (int surfix = 0; surfix < 1000; ++surfix)
        {
            newFileName=getNewFilename(now,surfix);
            struct stat checkBuf;
            if(stat(newFileName.c_str(),&checkBuf) < 0){
                foundFileName=true;
                break;          // 找到一个不存在的文件名
            }
            // 注意上面的检查文件是否存在和这个rename不是原子操作，可能在这个时间窗口内被人为的创建出来一个重名的文件，但是程序本身不会，因为newFileName是加了pid的。概率实在非常非常小了，可以忽略。
        }
        // rename,reopen
        if(foundFileName){
            rename(path_.c_str(),newFileName.c_str());
            reopen();
        }
        else{
            // shit,试了1000次居然都被占用了，只好继续打到原来的文件，过一会再尝试切分，那时时间变了，或许有可用文件名了
            // do nothing
        }        
    }
}

void FileAppender::output(char* msg,size_t len)
{
    {
        boost::mutex::scoped_lock guard(checkWriteMutex_);
        checkFile();
    }
    ::write(fd_,msg,len);
}

void FileAppender::outputWithoutLock(char* msg,size_t len)
{
    checkFile();
    
    ssize_t left=len;
    do{
        ssize_t nwrite =  ::write(fd_,msg+(len-left),left);
        if(nwrite >= 0){
            left -= nwrite;
        }
        else{
            // error.
            fprintf(stderr,"error=write_fail logger=%s filepath=%s fd=%d errno=%d log=%s\n",this->getAppenderName().c_str(),path_.c_str(),fd_,errno,msg);
            break;
        }
    }while(left>0);
}
