#ifndef _STRUTIL_H_
#define _STRUTIL_H_

#include <vector>
#include <string>
#include <sys/types.h>

#define likely(x) __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)

/* 最多返回count个域 */
std::vector<std::string> split(const std::string& src, char det, unsigned int count);

/**
 * 转化成绝对路径，去掉路径中间所有的 '.' '/'.
 * 假设当前是/home/
 * toRealPath("test.log","./","default.log") is "/home/test.log"
 * toRealPath("./log/test.log","./","default.log") is "/home/log/test.log"
 * toRealPath("/log/test.log","./","default.log") is "/log/test.log"
 * toRealPath("","./","default.log") is "/home/default.log"
 * toRealPath(NULL,"./","default.log") is "/home/default.log"
 *
 * @param defaultFilename
 * @param oldpath 
 * @param prefix 
 * 
 * @return 
 */
std::string toRealPath(const std::string& oldpath,const std::string& prefix,const std::string& defaultFilename);

std::string trim(std::string& str,const std::string& drop=" ");

void GetTimeString(char** time_str,size_t *len);
//class LogStream;
//void GetTimeString(LogStream& stream);
void GetTimeStringForFileName(char *timeBuff,size_t bufflen);
const char* GetPidStr();
const char* GetTidStr();

#endif /* _STRUTIL_H_ */
