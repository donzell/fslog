#include <Conf.h>
#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
    int ret = 0;
    testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    return ret;
}

TEST(TextConfig,FileNotExist)
{
    TextConfig conf("notExist.conf",":","#");
    EXPECT_TRUE(conf.getString("log.logdir").empty());
}

TEST(TextConfig,CommentCut)
{
    const char* filecontent="log.logdir:./log#to be cut\n#log.a:value\n";
    ofstream conffile("test.conf");
    ASSERT_TRUE(conffile.is_open());
    conffile<<filecontent<<flush;
    conffile.close();

    
    TextConfig conf("test.conf",":","#");
    EXPECT_EQ(conf.getString("log.logdir"),"./log");
}


TEST(TextConfig,Trim)
{
    const char* filecontent=" log.logdir : ./log #to be cut\n#log.a:value\nlog. whilespace_conf: whilespace value ";
    ofstream conffile("test.conf");
    ASSERT_TRUE(conffile.is_open());
    conffile<<filecontent<<flush;
    conffile.close();
    
    TextConfig conf("test.conf",":","#");
    EXPECT_EQ(conf.getString("log.logdir"),"./log");
    EXPECT_EQ(conf.getString("log. whilespace_conf"),"whilespace value");
}


TEST(TextConfig,emptyLine)
{
    const char* filecontent="\n ## \n ";
    ofstream conffile("test.conf");
    ASSERT_TRUE(conffile.is_open());
    conffile<<filecontent<<flush;
    conffile.close();
    
    TextConfig conf("test.conf",":","#");
    EXPECT_EQ(conf.getConfCount(),0);
}

TEST(TextConfig,MultiConfValue)
{
    const char* filecontent="log.logdir:./log#to be cut\nlog.logdir:./log2\n#log.a:value\n";
    ofstream conffile("test.conf");
    ASSERT_TRUE(conffile.is_open());
    conffile<<filecontent<<flush;
    conffile.close();

    
    TextConfig conf("test.conf",":","#");
    ASSERT_TRUE(conf.getStrings("log.logdir").size()==2);
    EXPECT_EQ(conf.getStrings("log.logdir")[0],"./log");
    EXPECT_EQ(conf.getStrings("log.logdir")[1],"./log2");
}
