#include <Log.h>
#include <assert.h>
#include "StrUtil.h"
#include <iostream>
using namespace std;
#define padsize 200

void* routine(void* arg)
{
    char* instance = (char*)arg;
    CLogger& testlog = CLogger::GetInstance(instance);
    int i=0;
    char pad[padsize];
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
    cout<<pthread_self()<<" "<<time_before<<"==>"<<time_str2<<endl;
    
    return NULL;
}

int main(int argc, char *argv[])
{
    CLogger::init(argv[1]);
    pthread_t tid;
    pthread_create(&tid,NULL,routine,argv[2]);
    routine(argv[2]);
    pthread_join(tid,NULL);
    
    return 0;
}
