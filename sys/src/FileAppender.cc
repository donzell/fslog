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
        // ֻ�᳢�Դ���ȱ�ٵĵ���Ŀ¼.����/home/work/log/test.log��ֻ�᳢�Դ���logĿ¼
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
    // 1.��鵱ǰfd�Ƿ�ָ��log�ļ���������˵����mv����rm�ˣ�����open(path,O_CREAT|O_WRONLY|O_APPEND,0755)
    // 2.���log�ļ��Ĵ�С������Ѿ������зִ�С��mv��open
    struct stat buf;
    memset(&buf,0,sizeof(buf));
    
    if(::stat(path_.c_str(),&buf) < 0){
        // ��Ӧ��û��Ȩ�ޣ������û��������ļ���Ŀ¼chmod��
        // �����ļ��ձ�mv����û���ü�create�����Ӧ�ú��ټ�
        reopen();
        return;
    }
    
    
    if(fd_ < 0 || inode_ != buf.st_ino){
        uint64_t saved_ino = buf.st_ino;
        
        // fd < 0������inode�Ѿ����ˣ����´�
        reopen();
        if(fd_ >= 0){
            struct stat dbuf;
            if(fstat(fd_,&dbuf) < 0){
                // ��ô����?
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
                break;          // �ҵ�һ�������ڵ��ļ���
            }
            // ע������ļ���ļ��Ƿ���ں����rename����ԭ�Ӳ��������������ʱ�䴰���ڱ���Ϊ�Ĵ�������һ���������ļ������ǳ������ᣬ��ΪnewFileName�Ǽ���pid�ġ�����ʵ�ڷǳ��ǳ�С�ˣ����Ժ��ԡ�
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
    // ��һ���������һ���Ƿ���Ҫ���´򿪣�ÿ�ζ����Ч��̫��.�ļ��зִ�С����������ⲻ��
    if(++loopCounter_ >= checkInterval_){
        loopCounter_ = 0;
        checkFile();
    }
    ::write(fd_,msg,len);
}
