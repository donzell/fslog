#ifndef _APPENDER_H_
#define _APPENDER_H_
#include <string>
#include <boost/noncopyable.hpp>

/**
 * @file   Appender.h
 * @author donzell <donzell.wu@gmail.com>
 * @date   Sun Oct 14 20:17:24 2012
 * 
 * @brief  接口类，所有的appender继承于此,同步文件Appender，异步文件Appender.
 * 
 * 
 */

class Appender:boost::noncopyable
{
public:
    Appender(const std::string& appenderName)
        :appenderName_(appenderName)
        {}
    
    virtual ~Appender()
        {}

    const std::string& getAppenderName()const
        {
            return appenderName_;
        }
    
    /** 
     * log初始化时对一个新的appender调用start.
     * 注意判断返回值。
     * 
     */
    virtual bool start()=0;

    /** 
     * log销毁时对所关联的appender调用stop.
     * 
     */
    virtual void stop()=0;

    /** 
     * 主要接口，由Logger前端调用此接口，把格式化好的日志交给appender去输出。
     * 为了减少copy次数，规定msg必须有调用者从堆上malloc，由appender负责free。
     * @param msg 
     * @param len 
     */
    virtual void output(char* msg,size_t len)=0;

private:
    std::string appenderName_;
};

    

#endif /* _APPENDER_H_ */
