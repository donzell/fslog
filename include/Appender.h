#ifndef _APPENDER_H_
#define _APPENDER_H_
#include <string>
#include <boost/noncopyable.hpp>

/**
 * @file   Appender.h
 * @author donzell <donzell.wu@gmail.com>
 * @date   Sun Oct 14 20:17:24 2012
 * 
 * @brief  �ӿ��࣬���е�appender�̳��ڴ�,ͬ���ļ�Appender���첽�ļ�Appender.
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
     * log��ʼ��ʱ��һ���µ�appender����start.
     * ע���жϷ���ֵ��
     * 
     */
    virtual bool start()=0;

    /** 
     * log����ʱ����������appender����stop.
     * 
     */
    virtual void stop()=0;

    /** 
     * ��Ҫ�ӿڣ���Loggerǰ�˵��ô˽ӿڣ��Ѹ�ʽ���õ���־����appenderȥ�����
     * Ϊ�˼���copy�������涨msg�����е����ߴӶ���malloc����appender����free��
     * @param msg 
     * @param len 
     */
    virtual void output(char* msg,size_t len)=0;

private:
    std::string appenderName_;
};

    

#endif /* _APPENDER_H_ */
