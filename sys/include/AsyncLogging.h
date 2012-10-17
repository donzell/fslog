#ifndef _ASYNCLOGGING_H_
#define _ASYNCLOGGING_H_

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>

class AsyncLogging : boost::noncopyable
{
public:

    AsyncLogging();

    ~AsyncLogging()
        {
            if (running_)
            {
                stop();
            }
        }

    void output(const char* logline, size_t len);

    void start()
        {
            boost::mutex::scoped_lock guard(startMutex_);
            if(running_){
                return;
            }
            running_ = true;
            pthread_create(&tid_,NULL,&AsyncLogging::threadFunc,this);
            startCond_.wait(guard);
        }

    void stop()
        {
            boost::mutex::scoped_lock guard(startMutex_);
            if(false == running_){
                return;
            }
            running_ = false;
            cond_.notify_all();
            pthread_join(tid_,NULL);
        }

private:

    // declare but not define, prevent compiler-synthesized functions
    AsyncLogging(const AsyncLogging&);  // ptr_container
    void operator=(const AsyncLogging&);  // ptr_container

    static   void* threadFunc(void*);
    void threadStart();
    
    typedef detail::FixedBuffer<detail::kLargeBuffer> Buffer;
    typedef boost::ptr_vector<Buffer> BufferVector;
    typedef BufferVector::auto_type BufferPtr;

    bool running_;
    boost::mutex startMutex_;
    boost::condition startCond_;
    boost::mutex mutex_;
    boost::condition cond_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
    pthread_t tid_;
    
};

#endif
