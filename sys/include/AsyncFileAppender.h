#ifndef _ASYNCFILEAPPENDER_H_
#define _ASYNCFILEAPPENDER_H_

#include <string>
#include <queue>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include "FileAppender.h"

class AsyncFileAppender;

typedef struct asyncWriteItem
{
    AsyncFileAppender* pAppender_;
    std::string buffer_;
    asyncWriteItem(AsyncFileAppender* pAppender,const std::string& buffer)
        :pAppender_(pAppender),buffer_(buffer)
        {}
    asyncWriteItem(AsyncFileAppender* pAppender,const char* msg,size_t len)
        :pAppender_(pAppender),buffer_(msg,len)
        {}
    
}asyncWriteItem_t;

class AsyncFileAppenderThread:boost::noncopyable
{
public:
    void output(const std::string& msg,AsyncFileAppender* pAppender);
    void output(const char* msg,size_t len,AsyncFileAppender* pAppender);
    void start();
    void stop();
    static AsyncFileAppenderThread& getInstance()
        {
            pthread_once(&pOnce,init);
            return *pInstance;
        }
    
  private:
    static AsyncFileAppenderThread* pInstance;
    static pthread_once_t pOnce;
    static void init()
        {
            pInstance = new AsyncFileAppenderThread();
        }
    
    AsyncFileAppenderThread()
        :asyncRunning_(false)
        {}
    
    ~AsyncFileAppenderThread();
    
    boost::scoped_ptr<boost::thread> thread_;

    enum{
        flushInterval_=3,
    };
    
    volatile bool asyncRunning_;
    boost::mutex queueMutex_;
    boost::mutex runMutex_;
    
    std::queue<asyncWriteItem_t*> logQueue_;
    pthread_cond_t queueCond_;
    void threadFunc();
    static void s_dumpQueue(void* arg);
    void dumpQueue();
    
    
};


class AsyncFileAppender:public FileAppender
{
public:
  AsyncFileAppender(const std::string& path,size_t splitSize,std::string& splitFormat)
      :FileAppender(path,splitSize,splitFormat),asyncThread_(AsyncFileAppenderThread::getInstance())
        {}
    virtual ~AsyncFileAppender()
        {}
    virtual void start();
    virtual void stop();

    virtual void output(const std::string& msg);
    virtual void output(const char* msg,size_t len);

    void inThreadOutput(const char* msg,size_t len);
    
  private:
    AsyncFileAppenderThread& asyncThread_;
};


#endif /* _ASYNCFILEAPPENDER_H_ */
