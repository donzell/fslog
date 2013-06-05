#include <Log.h>
#include <assert.h>
#include "StrUtil.h"
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    CLogger::init(argv[1]);
    LoggerPtr testlog = CLogger::getLogInstance(argv[2]);
    int i=0;
    #define padsize 200
    char *pad=(char*)malloc(padsize);
    assert(pad);                                                                                                                                                                          
    memset(pad,'a',padsize);
    pad[padsize-1]=0;
    char* time_str1,*time_str2;
    size_t len1,len2;
    GetTimeString(&time_str1,&len1);
    char time_before[32]={0};
    memcpy(time_before,time_str1,len1);
    
    while ( i++ < 10000000 )
    {
        LOGGER_NOTICE(testlog,"%d, pad=%s",i,pad);
    }
    GetTimeString(&time_str2,&len2);
    cout<<time_before<<"==>"<<time_str2<<endl;
    
    return 0;
}
