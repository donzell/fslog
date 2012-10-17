#include <stdio.h>
#include <boost/thread/xtime.hpp>
#include "AsyncLogging.h"


AsyncLogging::AsyncLogging()
    :running_(false),
//    startCond_(startMutex_),
//    cond_(mutex_),
     currentBuffer_(new Buffer),
     nextBuffer_(new Buffer),
     buffers_()
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

void AsyncLogging::output(const char* logline, size_t len)
{
    boost::mutex::scoped_lock guard(mutex_);
    if (currentBuffer_->avail() > len)
    {
        currentBuffer_->append(logline, len);
    }
    else
    {
        buffers_.push_back(currentBuffer_.release());

        if (nextBuffer_)
        {
            currentBuffer_ = boost::ptr_container::move(nextBuffer_);
        }
        else
        {
            currentBuffer_.reset(new Buffer); // Rarely happens
        }
        currentBuffer_->append(logline, len);
        cond_.notify_all();
    }
}

void* AsyncLogging::threadFunc(void* arg)
{
    AsyncLogging* pAsyncLog = static_cast<AsyncLogging*>(arg);
    assert(pAsyncLog!=NULL);
    pAsyncLog->threadStart();
    pthread_exit(NULL);
  
}
void AsyncLogging::threadStart()
{
  
    assert(running_ == true);
    startCond_.notify_all();
  
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    while (running_)
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            boost::mutex::scoped_lock guard(mutex_);
            if (buffers_.empty())  // unusual usage!
            {
                boost::xtime xt;
                boost::xtime_get(&xt,boost::TIME_UTC);
                xt.sec+=3;
                cond_.timed_wait(guard,xt);
            }
            buffers_.push_back(currentBuffer_.release());
            currentBuffer_ = boost::ptr_container::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_)
            {
                nextBuffer_ = boost::ptr_container::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        if (buffersToWrite.size() > 25)
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages , %zd larger buffers\n",buffersToWrite.size()-2);
            fputs(buf, stderr);
            // TODO write buf to logfile.
            
            buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
        }

        for (size_t i = 0; i < buffersToWrite.size(); ++i)
        {
            // FIXME: use unbuffered stdio FILE ? or use ::writev ?
            //output.append(buffersToWrite[i].data(), buffersToWrite[i].length());
            // TODO write.
        }

        if (buffersToWrite.size() > 2)
        {
            // drop non-bzero-ed buffers, avoid trashing
            buffersToWrite.resize(2);
        }

        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        // TODO flush.
    }
    // TODO flush.
}



