#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>
#include "Conf.h"
#include "StrUtil.h"
using namespace std;

TextConfig::TextConfig(const string& filename,string sep,string comment)
{
    FILE* fp = ::fopen(filename.c_str(),"r");
    if(NULL == fp){
        error(0,errno,"load conf fail,filename=%s",filename.c_str());
        return;
    }
    char* line=NULL;
    ssize_t size=-1;
    size_t len=0;
    
    while((size=getline(&line,&len,fp))!=-1){
        string str_line(line,size);
        if(!str_line.empty() && str_line[str_line.length()-1]=='\n'){
            str_line.erase(str_line.end()-1,str_line.end());
        }
        
        string::size_type index=str_line.find(comment);
        string str(str_line,0,index);
        index=str.find(sep);
        string key(str,0,index);
        string value;
        if(index != string::npos){
            value.append(str,index+sep.length(),str.length()-(index+sep.length()));
        }
        key=trim(key," ");
        value=trim(value," ");

        if(!key.empty())
            confMap_[key].push_back(value);
    }
    free(line);
    fclose(fp);
}

TextConfig::~TextConfig()
{}

