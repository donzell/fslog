#ifndef _FILEAPPENDER_H_
#define _FILEAPPENDER_H_

#include <string>
#include <stdint.h>
#include <vector>
#include <map>
#include "boost/thread.hpp"
#include "Appender.h"

class FileAppender:public Appender
{
public:
    enum
    {
        MIN_SPLITSIZE=20*1024*1024UL, // ��С20M
        MAX_SPLITSIZE=8*1024*1024*1024UL, // ���8G
        DEFAULT_SPLITSIZE=2*1024*1024*1024UL, // Ĭ��2G
    };
    typedef struct format
    {
        enum{
            RAW_STR,
            YEAR,
            MONTH,
            DAY,
            HOUR,
            MINUTE,
            SECOND,
            PID,
            SEQNUMBER,
        };
        int type_;
        std::string str_;
    }format_t;
    
    
    FileAppender(const std::string& path,uint64_t splitsize,const std::string& splitFormat);
    void setCheckInterval(int times){checkInterval_ = times;}
    void setCheckTimeInterval(time_t seconds){checkTimeInterval_=seconds;}
    
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

    void setSplitFormat(const std::string& splitFormat)
    {
        splitFormat_ = splitFormat;
        parseSplitFormat();     // Ҫ����splitFormat
    }
    std::string getSplitFormat()const{return splitFormat_;}
    
    void reopen();
    /* �������ļ��Ƿ���з��ˣ��Ƿ��Ѿ���mv���ˣ���Ҫʱ���߾��ļ����������ļ� */
    void checkFile();
    int getFd()const
        {return fd_;}
    
    virtual ~FileAppender();
    virtual bool start()
        {
            return true;
        }
    
    virtual void stop()
        {}
    
    virtual void output(const std::string& msg)
        {
            this->output(msg.c_str(),msg.length());
        }
    
    virtual void output(const char* msg,size_t len);
    void outputWithoutLock(const char* msg,size_t len);

    std::string getNewFilename(time_t now,int seqNumber)const;
private:
    void parseSplitFormat();
    void generateSplitFormatCharMap();

    
    std::string path_;
    uint64_t splitSize_;
    uint64_t inode_;
    int checkInterval_;         // ÿ���μ��һ���ļ�
    int loopCounter_;
    time_t checkTimeInterval_;  // ÿ������һ���ļ�
    time_t lastcheck_;          // �ϴμ��ʱ��
    int fd_;
    boost::mutex checkWriteMutex_;    
    std::string splitFormat_;
    std::vector<format_t> formats_;
    typedef std::map<char,int> charToFormatMap;
    charToFormatMap formatsCharMap_;

};

#endif /* _FILEAPPENDER_H_ */
