/**
 * @file   Conf.h
 * @author donzell <donzell.wu@gmail.com>
 * @date   Thu Sep 27 15:36:23 2012
 * 
 * @brief  简单的key-value配置解析。因为时配置解析，一般只会程序启动时执行一次，所以不用关注效率。
 * 1、去掉注释
 * 2、按照sep分割成key-value.
 * 3、trim
 * 
 * 
 */
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <vector>
#include <string>
#include <map>

class TextConfig
{
public:
    TextConfig(const std::string& filename,std::string sep=":",std::string comment="#");

    /** 
     * 获取某个配置项的值。可能有多个，所以返回vector
     * 
     * @param name 配置项名字
     * 
     * @return 值列表
     */
    std::vector<std::string> getStrings(const std::string& name)
        {
            return confMap_[name];
        }
    

    /** 
     * 获取某个配置项的值，只取一个
     * 
     * @param name 
     * 
     * @return 
     */
    std::string getString(const std::string& name)
        {
            std::vector<std::string> vec = confMap_[name];
            if(!vec.empty()){
                return vec[0];
            }
            else{
                std::string empty;
                return empty;
            }
        }
    
    size_t getConfCount()
    {
        return confMap_.size();
    }
    
    ~TextConfig();
  private:
//     std::string filename_;
//     std::string sep_;
    
    std::map<std::string,std::vector<std::string> > confMap_;
};

#endif /* _CONFIG_H_ */
