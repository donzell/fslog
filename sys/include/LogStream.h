#ifndef _LOGSTREAM_H
#define _LOGSTREAM_H

#include <assert.h>
#include <string.h> // memcpy
#include <string>

#include <boost/noncopyable.hpp>

namespace detail
{

    const size_t kSmallBuffer = 4096;
    const size_t kLargeBuffer = 4096*1024;

    template<int SIZE>
    class FixedBuffer : boost::noncopyable
    {
    public:
        FixedBuffer()
            : cur_(data_)
            {
                setCookie(cookieStart);
            }

        ~FixedBuffer()
            {
                setCookie(cookieEnd);
            }

        void append(const char* /*restrict*/ buf, size_t len)
            {
                size_t availLen = avail();
                if (availLen <= len)
                {
                    len = availLen;
                }
    
                memcpy(cur_, buf, len);
                cur_ += len;
            }
        void appendForce(const char* buf,size_t len)
            {
                if(avail() >= len){
                    memcpy(cur_, buf, len);
                    cur_ += len;
                }
                else if(sizeof(data_) >= len){
                    cur_ = data_;
                    cur_ += (sizeof(data_)-len);
                    memcpy(cur_,buf,len);
                    cur_ += len;
                }
                else{
                    memcpy(data_,buf,sizeof(data_));
                    cur_+=sizeof(data_);
                }
            }
        
        const char* data()const{ return data_; }
        size_t length() const { return cur_ - data_; }

        // write to data_ directly
        char* current() { return cur_; }
        size_t avail() const { return end() - cur_; }
        void add(size_t len) { cur_ += len; }

        void reset() { cur_ = data_; }
        void bzero() { ::bzero(data_, sizeof data_); }

        void setCookie(void (*cookie)()) { cookie_ = cookie; }
        // for used by unit test
        std::string asString() const { return std::string(data_, length()); }

    private:
        const char* end() const { return data_ + sizeof data_; }
        // Must be outline function for cookies.
        static void cookieStart();
        static void cookieEnd();

        void (*cookie_)();
        char data_[SIZE];
        char* cur_;
    };

}

class LogStream : boost::noncopyable
{
    typedef LogStream self;
public:
    typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

    self& operator<<(bool v)
        {
            buffer_.append(v ? "1" : "0", 1);
            return *this;
        }

    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);

    self& operator<<(const void*);

    self& operator<<(float v)
        {
            *this << static_cast<double>(v);
            return *this;
        }
    self& operator<<(double);
    // self& operator<<(long double);

    self& operator<<(char v)
        {
            buffer_.append(&v, 1);
            return *this;
        }

    // self& operator<<(signed char);
    // self& operator<<(unsigned char);

    self& operator<<(const char* v)
        {
            buffer_.append(v, strlen(v));
            return *this;
        }

    self& operator<<(const std::string& v)
        {
            buffer_.append(v.c_str(), v.size());
            return *this;
        }

    void append(const char* data, int len) { buffer_.append(data, len); }
    const Buffer& buffer() const { return buffer_; }
    void resetBuffer() { buffer_.reset(); }

    /* 强制添加换行符，空间不足可截断字符串 */
    void forceAddEndLine()
        {
            buffer_.appendForce("\n",1);
        }

private:
    void staticCheck();

    template<typename T>
    void formatInteger(T);

    Buffer buffer_;

    static const size_t kMaxNumericSize = 32;
};

class Fmt // : boost::noncopyable
{
public:
    template<typename T>
    Fmt(const char* fmt, T val);

    const char* data() const { return buf_; }
    int length() const { return length_; }

private:
    char buf_[32];
    int length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
    s.append(fmt.data(), fmt.length());
    return s;
}


#endif  // _LOGSTREAM_H

