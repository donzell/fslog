/**
 * @file   Conf.h
 * @author donzell <donzell.wu@gmail.com>
 * @date   Thu Sep 27 15:36:23 2012
 * 
 * @brief  �򵥵�key-value���ý�������Ϊʱ���ý�����һ��ֻ���������ʱִ��һ�Σ����Բ��ù�עЧ�ʡ�
 * 1��ȥ��ע��
 * 2������sep�ָ��key-value.
 * 3��trim
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
     * ��ȡĳ���������ֵ�������ж�������Է���vector
     * 
     * @param name ����������
     * 
     * @return ֵ�б�
     */
    std::vector<std::string> getStrings(const std::string& name)
        {
            return confMap_[name];
        }
    

    /** 
     * ��ȡĳ���������ֵ��ֻȡһ��
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
