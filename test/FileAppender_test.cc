#include <string>
#include <time.h>
#include <sys/time.h>
#include <FileAppender.h>
#include <gtest/gtest.h>

using namespace std;


static time_t now = 1351829102;
static string testyear="2012";
static string testmonth="11";
static string testday="02";
static string testhour="12";
static string testminute="05";
static string testsecond="02";

int main(int argc, char *argv[])
{
    int ret=0;
    testing::InitGoogleTest(&argc,argv);
    ret = RUN_ALL_TESTS();
    return ret;
}

TEST(FileAppender,getNewFilenameEmptyFormat)
{
    
    FileAppender appender("./testFileappender.log",10000000,"");
    for(int i=0;i<10;i++){
        string newFileName = appender.getNewFilename(now,i);
        
        ASSERT_EQ(newFileName , "./testFileappender.log");
    }
    
}

TEST(FileAppender,getNewFilenameFullFormat)
{    
    FileAppender appender("./testFileappender.log",10000000,"%_%Y%m%d_%H%M%S_%P_%n%str");
    char pidstr[16];
    snprintf(pidstr,sizeof(pidstr),"%d",getpid());
    
    for(int i=0;i<10;i++){
        string newFileName = appender.getNewFilename(now,i);
        char str[16];
        snprintf(str,sizeof(str),"%03d",i);
        string name="./testFileappender.log";
        name+="%_";
        name = name+testyear+testmonth+testday+"_"+testhour+testminute+testsecond+"_"+pidstr+"_"+str+"%str";
        
        ASSERT_EQ(newFileName,name);
    }    
}

TEST(FileAppender,getNewFilenameSuffixFormat)
{    
    FileAppender appender("./testFileappender.log",10000000,"%n");
    string name="./testFileappender.log";
    for(int i=0;i<10;i++){
        string newFileName = appender.getNewFilename(now,i);
        char str[16];
        snprintf(str,sizeof(str),"%03d",i);
        string rightname=name+str;

        ASSERT_EQ(newFileName,rightname);
    }    
}

TEST(FileAppender,getNewFilenameStrFormat)
{    
    FileAppender appender("./testFileappender.log",10000000,"str");
    string rightname="./testFileappender.logstr";
    for(int i=0;i<10;i++){
        string newFileName = appender.getNewFilename(now,i);

        ASSERT_EQ(newFileName,rightname);
    }    
}

