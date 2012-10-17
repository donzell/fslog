#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "FileAppender.h"
#include "StrUtil.h"
using std::string;

FileAppender::FileAppender(const string& path,uint64_t splitsize,int checkInterval)
    :Appender(path),path_(path),splitSize_(splitsize),inode_(0),checkInterval_(checkInterval),loopCounter_(0),fd_(-1)
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

void FileAppender::checkFile()
{
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
        for (int surfix = 0; surfix < 1000; ++surfix)
        {    
            // a.log.20121010090048_3164_000
            pid_t pid = getpid();
            char pid_str[24]={0};
            snprintf(pid_str,sizeof(pid_str),"%d",pid);
            char time_str[128]={0};
            GetTimeStringForFileName(time_str,sizeof(time_str));
            char surfix_str[10];
            snprintf(surfix_str,sizeof(surfix_str),"%03d",surfix);
            newFileName = path_+"."+time_str+"_"+pid_str+"_"+surfix_str;
            struct stat checkBuf;
            if(stat(newFileName.c_str(),&checkBuf) < 0){
                break;          // 找到一个不存在的文件名
            }
            // 注意上面的检查文件是否存在和这个rename不是原子操作，可能在这个时间窗口内被人为的创建出来一个重名的文件，但是程序本身不会，因为newFileName是加了pid的。概率实在非常非常小了，可以忽略。
        }
        // rename,reopen
        rename(path_.c_str(),newFileName.c_str());
        reopen();
    }
}

void FileAppender::output(const char* msg,size_t len)
{
    boost::mutex::scoped_lock guard(checkWriteMutex_);
    outputWithoutLock(msg,len);
}

void FileAppender::outputWithoutLock(const char* msg,size_t len)
{
    // 隔一定次数检查一次是否需要重新打开，每次都检查效率太低.文件切分大小有误差，这个问题不大
    if(++loopCounter_ >= checkInterval_){
        loopCounter_ = 0;
        checkFile();
    }
    ::write(fd_,msg,len);
}
