#include "AsyncFileAppender.h"
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
using std::string;
using std::queue;

pthread_once_t AsyncFileAppenderThread::pOnce = PTHREAD_ONCE_INIT;
AsyncFileAppenderThread* AsyncFileAppenderThread::pInstance = NULL;

void AsyncFileAppender::start()
{
    asyncThread_.start();
}

void AsyncFileAppender::stop()
{
    asyncThread_.stop();
}

void AsyncFileAppender::output(const char* msg,size_t len)
{
    asyncThread_.output(msg,len,this);
}

void AsyncFileAppender::output(const string& msg)
{
    asyncThread_.output(msg,this);
}

void AsyncFileAppender::inThreadOutput(const char* msg,size_t len)
{
    FileAppender::outputWithoutLock(msg,len);
}

void AsyncFileAppenderThread::start()
{
    boost::mutex::scoped_lock guard(runMutex_);
    if(asyncRunning_){
        return;
    }
    asyncRunning_ = true;    
    thread_.reset(new boost::thread(boost::bind(&AsyncFileAppenderThread::threadFunc,this)));
    if(!thread_){
        asyncRunning_ = false;
    }
}

void AsyncFileAppenderThread::stop()
{
    boost::mutex::scoped_lock guard(runMutex_);
    if(asyncRunning_){
        asyncRunning_ = false;
        thread_->join();
        thread_.reset();
        return;
    }
}

void AsyncFileAppenderThread::output(const char* msg,size_t len,AsyncFileAppender* pAppender)
{
    boost::mutex::scoped_lock guard(queueMutex_);
    logQueue_.push(new asyncWriteItem_t(pAppender,msg,len));
}

void AsyncFileAppenderThread::output(const string& msg,AsyncFileAppender* pAppender)
{
    boost::mutex::scoped_lock guard(queueMutex_);
    logQueue_.push(new asyncWriteItem_t(pAppender,msg));
}

void AsyncFileAppenderThread::threadFunc()
{
    asyncWriteItem_t* item = NULL;
    pthread_cleanup_push(AsyncFileAppenderThread::s_dumpQueue,this);
    
    while(asyncRunning_){
        item = NULL;
        
        {
            boost::mutex::scoped_lock guard(queueMutex_);
            if(!logQueue_.empty()){
                item = logQueue_.front();
                logQueue_.pop();
            }
        }
        if(item){
            item->pAppender_->inThreadOutput(item->buffer_.c_str(),item->buffer_.length());
            delete item;
            continue;
        }
        usleep(1);
    }

    pthread_cleanup_pop(1);
}

void AsyncFileAppenderThread::s_dumpQueue(void* arg)
{
    AsyncFileAppenderThread* ptr = (AsyncFileAppenderThread*)arg;
    ptr->dumpQueue();
}

void AsyncFileAppenderThread::dumpQueue()
{
    asyncWriteItem_t* item = NULL;
    
    // 退出时，尽量把所有日志写完
    while(true){
        item = NULL;
        {   
            boost::mutex::scoped_lock guard(queueMutex_);
            if(logQueue_.empty())
                break;
            
            item = logQueue_.front();
            logQueue_.pop();
        }
        if(item){
            item->pAppender_->inThreadOutput(item->buffer_.c_str(),item->buffer_.length());
            delete item;
        }
    }
}
