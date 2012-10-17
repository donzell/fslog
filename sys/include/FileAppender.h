#ifndef _FILEAPPENDER_H_
#define _FILEAPPENDER_H_

#include <string>
#include <stdint.h>
#include <boost/thread.hpp>
#include "Appender.h"

class FileAppender:public Appender
{
public:
    enum
    {
        MIN_SPLITSIZE=20*1024*1024UL, // 最小20M
        MAX_SPLITSIZE=8*1024*1024*1024UL, // 最大8G
        DEFAULT_SPLITSIZE=2*1024*1024*1024UL, // 默认2G
    };
    
    FileAppender(const std::string& path,uint64_t splitsize,int checkInterval=1000);

    void setSplitSize(uint64_t splitsize){
        if(splitsize <= 0)
            splitSize_ = DEFAULT_SPLITSIZE;
        else if(splitsize < MIN_SPLITSIZE)
            splitSize_ = MIN_SPLITSIZE;
        else if(splitsize > MAX_SPLITSIZE)
            splitSize_ = MAX_SPLITSIZE;
        else
            splitSize_ = splitsize;
    }

    uint64_t getSplitSize()const{return splitSize_;}

    void reopen();
    /* 负责检查文件是否该切分了，是否已经被mv走了，必要时移走旧文件，生成新文件，最终要提供一个可用的文件fd。 */
    void checkFile();
    int getFd()const
        {return fd_;}
    
    virtual ~FileAppender();
    virtual void start()
        {}
    
    virtual void stop()
        {}
    
    virtual void output(const std::string& msg)
        {
            this->output(msg.c_str(),msg.length());
        }
    
    virtual void output(const char* msg,size_t len);
    void outputWithoutLock(const char* msg,size_t len);
private:
    std::string path_;
    uint64_t splitSize_;
    uint64_t inode_;
    int checkInterval_;         // 每几次检查一次文件
    int loopCounter_;
    int fd_;
    boost::mutex checkWriteMutex_;    
    
};

#endif /* _FILEAPPENDER_H_ */
