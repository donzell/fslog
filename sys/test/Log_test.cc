#include <Log.h>
#include <assert.h>
#include "StrUtil.h"
#include <iostream>
using namespace std;
#define padsize 100

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
    if(argc != 4){
        cout<<argv[0]<<" conf logInstance threadNum.\nExample:"<<argv[0]<<" testlog.conf test 2"<<endl;
        return -1;
    }
    
    int threadNum = atoi(argv[3]);
    
    cout<<"init="<<CLogger::init(argv[1])<<endl;
    vector<pthread_t> tids;
    pthread_t tid;
    
    for(int i=0;i<threadNum;i++){
        pthread_create(&tid,NULL,routine,argv[2]);
        tids.push_back(tid);
    }
    
    routine(argv[2]);
    for(int i=0;i<threadNum;i++)
        pthread_join(tids[i],NULL);
    
    return 0;
}
