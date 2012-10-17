#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/time.h>
#include "StrUtil.h"
#include "LogStream.h"
using namespace std;

enum{
    MAXPATH = 4096,
};

static char *_realpath (const char *name, char *resolved)
{
    char *rpath, *dest;
    const char *start, *end, *rpath_limit;
    long int path_max;

    if (name == NULL)
    {
        /* As per Single Unix Specification V2 we must return an error if
           either parameter is a null pointer.  We extend this to allow
           the RESOLVED parameter to be NULL in case the we are expected to
           allocate the room for the return value.  */
        errno = EINVAL;
        return NULL;
    }

    if (name[0] == '\0')
    {
        /* As per Single Unix Specification V2 we must return an error if
           the name argument points to an empty string.  */
        errno = ENOENT;
        return NULL;
    }

    #ifdef PATH_MAX
    path_max = PATH_MAX;
    #else
    path_max = pathconf (name, _PC_PATH_MAX);
    if (path_max <= 0)
        path_max = 1024;
    #endif

    if (resolved == NULL)
    {
        rpath = (char*)malloc (path_max);
        if (rpath == NULL)
            return NULL;
    }
    else
        rpath = resolved;
    rpath_limit = rpath + path_max;

    if (name[0] != '/')
    {
        if (!getcwd (rpath, path_max))
        {
            rpath[0] = '\0';
            goto error;
        }
        dest = (char*)rawmemchr (rpath, '\0');
    }
    else
    {
        rpath[0] = '/';
        dest = rpath + 1;
    }

    for (start = end = name; *start; start = end)
    {
        /* Skip sequence of multiple path-separators.  */
        while (*start == '/')
            ++start;

        /* Find end of path component.  */
        for (end = start; *end && *end != '/'; ++end)
            /* Nothing.  */;

        if (end - start == 0)
            break;
        else if (end - start == 1 && start[0] == '.')
            /* nothing */;
        else if (end - start == 2 && start[0] == '.' && start[1] == '.')
        {
            /* Back up to previous component, ignore if at root already.  */
            if (dest > rpath + 1)
                while ((--dest)[-1] != '/');
        }
        else
        {
            size_t new_size;

            if (dest[-1] != '/')
                *dest++ = '/';

            if (dest + (end - start) >= rpath_limit)
            {
                ptrdiff_t dest_offset = dest - rpath;
                char *new_rpath;

                if (resolved)
                {
                    errno = ENAMETOOLONG;
                    if (dest > rpath + 1)
                        dest--;
                    *dest = '\0';
                    goto error;
                }
                new_size = rpath_limit - rpath;
                if (end - start + 1 > path_max)
                    new_size += end - start + 1;
                else
                    new_size += path_max;
                new_rpath = (char *) realloc (rpath, new_size);
                if (new_rpath == NULL)
                    goto error;
                rpath = new_rpath;
                rpath_limit = rpath + new_size;

                dest = rpath + dest_offset;
            }

            dest = (char*)mempcpy (dest, start, end - start);
            *dest = '\0';

        }
    }
    if (dest > rpath + 1 && dest[-1] == '/')
        --dest;
    *dest = '\0';

    assert (resolved == NULL || resolved == rpath);
    return rpath;

  error:
    assert (resolved == NULL || resolved == rpath);
    if (resolved == NULL)
        free (rpath);
    return NULL;
}

string toRealPath(const string& oldpath,const string& prefix,const string& defaultFilename)
{
    string realPrefix = prefix;
    if(realPrefix.empty()){
        realPrefix="./";
    }
    trim(realPrefix);
    
    string strDefaultFilename=defaultFilename;
    trim(strDefaultFilename);
    if(strDefaultFilename.empty()){
        strDefaultFilename = "default.log";
    }
    
    string tmp = oldpath;
    trim(tmp);

    if(tmp.empty()){
        tmp+=realPrefix;
    }
    
    if(tmp[tmp.length()-1]=='.'){
        tmp+="/";
        tmp+=strDefaultFilename;
    }
    
    if(tmp[tmp.length()-1]=='/'){
        tmp+=strDefaultFilename;
    }
    
    if(tmp[0]!='/' && tmp[0]!='.'){
        tmp = (*(realPrefix.end()-1)=='/' ? realPrefix+tmp : realPrefix+"/"+tmp);
    }
    char *absPathBuff = _realpath(tmp.c_str(),NULL);
    assert(NULL != absPathBuff);
    string absPath(absPathBuff);
    free(absPathBuff);
    return absPath;
}


string trim(string& str,const string& drop)
{
    str = str.erase(0,str.find_first_not_of(drop));
    str = str.erase(str.find_last_not_of(drop)+1);
    return str;
}

vector<string> split(const string& src, char det, unsigned int count)
{
    vector<string> ret;
    string::size_type pos = 0;
    string::size_type use;
    use = src.find(det);
    while(use != string::npos)
    {
        ret.push_back(src.substr(pos, use - pos));
        pos = use + 1;
        use = src.find(det, pos);
    }
    
    if(pos >= src.length()){
        ret.push_back(string());
    }
    else{
        ret.push_back(src.substr(pos,use));
    }

    if(ret.size()>count){
        ret.resize(count);
    }
    
    return ret;
}
__thread time_t t_lastSecond;
__thread char t_time[32];
__thread int offset;

void GetTimeString(LogStream& stream)
{
    struct timeval tmval;
    gettimeofday(&tmval, NULL);
    //time_t now = time(NULL);
    time_t now = tmval.tv_sec;
    if(now != t_lastSecond){
        t_lastSecond = now;
    
        struct tm newTimeObj;
        struct tm *newTime = localtime_r(&now, &newTimeObj);
        // 2007-12-25 01:45:32:123456
        offset = snprintf(t_time,sizeof(t_time), "%4d-%02d-%02d %02d:%02d:%02d:"
                 , newTime->tm_year + 1900, newTime->tm_mon + 1
                 , newTime->tm_mday, newTime->tm_hour
                 , newTime->tm_min, newTime->tm_sec);
        
    }
    t_time[offset]=0;
    stream<<t_time<<tmval.tv_usec;
}

void GetTimeString(char **timeBuff,size_t* bufflen)
{
    struct timeval tmval;
    gettimeofday(&tmval, NULL);
    //time_t now = time(NULL);
    time_t now = tmval.tv_sec;
    if(now != t_lastSecond){
        t_lastSecond = now;
    
        struct tm newTimeObj;
        struct tm *newTime = localtime_r(&now, &newTimeObj);
        // 2007-12-25 01:45:32:123456
        offset = snprintf(t_time,sizeof(t_time), "%4d-%02d-%02d %02d:%02d:%02d:"
                 , newTime->tm_year + 1900, newTime->tm_mon + 1
                 , newTime->tm_mday, newTime->tm_hour
                 , newTime->tm_min, newTime->tm_sec);
        
    }
    int ret = snprintf(t_time+offset,sizeof(t_time)-offset,"%06ld",static_cast<long>(tmval.tv_usec));
    *timeBuff = t_time;
    *bufflen = static_cast<size_t>(offset+ret);
}

void GetTimeStringForFileName(char* timeBuff,size_t bufflen)
{
    struct timeval tmval;
    memset(&tmval,0,sizeof(tmval));
    
    gettimeofday(&tmval, NULL);
    //time_t now = time(NULL);
    time_t now = tmval.tv_sec;
    struct tm newTimeObj;
    struct tm *newTime = localtime_r(&now, &newTimeObj);
    // 2007-12-25-01
    snprintf(timeBuff,bufflen, "%4d-%02d-%02d-%02d",newTime->tm_year + 1900,newTime->tm_mon + 1,newTime->tm_mday,newTime->tm_hour);
}


static pid_t _pid;
static char _pidStr[32];

static pthread_once_t _getpidOnce = PTHREAD_ONCE_INIT;
static __thread char _tidStr[32];
static __thread pthread_t _tid;

static void fork_prepare()
{}
static void fork_child()
{
    _pid = getpid();
    snprintf(_pidStr,sizeof(_pidStr),"%d",_pid);
}
static void fork_parent()
{}
static void getpidOnceFunc()
{
    _pid = getpid();
    snprintf(_pidStr,sizeof(_pidStr),"%d",_pid);
    pthread_atfork(&fork_prepare,&fork_parent,&fork_child);
}

const char* GetPidStr()
{
    pthread_once(&_getpidOnce,getpidOnceFunc);
    return _pidStr;
}

const char* GetTidStr()
{
    if(likely(_tid)){
        return _tidStr;
    }
    _tid = pthread_self();
    snprintf(_tidStr,sizeof(_tidStr),"%ld",_tid);
    return _tidStr;
}
