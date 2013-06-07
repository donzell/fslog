#include  "Log.h"

#include <iostream>
using namespace std;

void func()
{
    CLogger& logger = CLogger::GetInstance("dblog");
    LOGGER_DEBUG(logger,"one dblog.ip=%s port=%u","127.0.0.1",3306);
    
}

// 要说这example这没啥写的，接口就是这么的简单纯洁。
int main(int argc, char *argv[])
{

    // 一个程序里，只需要init一次即可。最好判断一下成功与否，日志很重要，如果失败，最好退出。
    if(CLogger::init("example.conf") != 0){
        cerr<<"check you conf file example.conf!"<<endl;
        return -1;
    }

    // 使用前，请获取CLogger引用，给一个instance名。该日志库支持多个日志实例，极为方便。
    // 可能我想把跟用户请求相关的放到serverlog里，把数据库操作方法到dblog里，把底层网络放到libeventlog里，分类放置。
    CLogger& logger = CLogger::GetInstance("serverlog");

    // 应该使用且仅使用这一组宏打日志。日志中需要的time,func,那一坨信息，他们要不要打，都是在配置里指定的！
    // 不是定死的，你可以定义自己的日志格式。看example.conf配置说明吧。
    
    LOGGER_NOTICE(logger,"%s %d","hello",1);
    

    func();
    
    return 0;
}
