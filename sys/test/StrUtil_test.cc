#include "StrUtil.h"
#include "gtest/gtest.h"
#include<vector>
#include<string>
using namespace std;
int main(int argc, char **argv)
{
    int ret = 0;
    testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    return ret;
}

static void checkResult(vector<string> result,vector<string> rightAnswer)
{
    ASSERT_EQ(result.size(),rightAnswer.size());
    for (size_t i = 0; i < result.size(); ++i)
    {
        EXPECT_EQ(result[i],rightAnswer[i]);
    }
}

TEST(StrUtil,trim)
{
    string str("  space");
    EXPECT_EQ(trim(str),"space");
    str="space  ";
    EXPECT_EQ(trim(str),"space");
    str=" space ";
    EXPECT_EQ(trim(str),"space");
    str=" space ;";
    EXPECT_EQ(trim(str),"space ;");
}

TEST(StrUtil,split)
{
    string str;
    vector<string> answer;

    vector<string> result;

    str="a,b i k e,c,d,e,f,";
    result= split(str,',',3);
    answer.clear();
    answer.push_back("a");
    answer.push_back("b i k e");
    answer.push_back("c");
    checkResult(result,answer);

    str="a,b i,";
    result=split(str,',',3);
    answer.clear();
    answer.push_back("a");
    answer.push_back("b i");
    answer.push_back("");
    checkResult(result,answer);
    
    str="a,b,c";
    result=split(str,',',3);
    answer.clear();
    answer.push_back("a");
    answer.push_back("b");
    answer.push_back("c");
    checkResult(result,answer);

    str="a,b i";
    result=split(str,',',3);
    answer.clear();
    answer.push_back("a");
    answer.push_back("b i");
    checkResult(result,answer);
    
}

class toRealPathTest:public testing::TestWithParam<vector<string> >
{
public:
    toRealPathTest()
        {}
    
    virtual ~toRealPathTest()
        {}
    
};

TEST_P(toRealPathTest,test)
{
    vector<string> param = GetParam();

    ASSERT_EQ(param.size(),4);
    EXPECT_EQ(toRealPath(param[0],param[1],param[2]),param[3])<<"toRealPath("<<param[0]<<","<<param[1]<<","<<param[2]<<") expect to be "<<param[3]<<" but not.\n";
}
static vector<string> makeParam(string a,string b,string c,string d)
{
    vector<string> ret;
    ret.push_back(a);
    ret.push_back(b);
    ret.push_back(c);
    ret.push_back(d);
    return ret;
}

INSTANTIATE_TEST_CASE_P(test,toRealPathTest,testing::Values(makeParam("test.log","./","default.log","./test.log"),
                                                            makeParam("./log/test.log","./","default.log","./log/test.log"),
                                                            makeParam("/log/test.log","./","default.log","/log/test.log"),
                                                            makeParam("","./","default.log","./default.log")));

