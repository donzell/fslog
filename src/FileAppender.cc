#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <sstream>
#include "FileAppender.h"
#include "StrUtil.h"
using namespace std;


static inline void CreateDir(std::string &path) {
    std::string::size_type off = 0;

    while ((off = path.find("/", off)) != std::string::npos)  {
        char backup = path[off];
        
        path[off] = 0;
        mkdir(path.c_str(), 0755);
        path[off] = backup;
        
        ++ off;
    }
}

FileAppender::FileAppender(const string& path,uint64_t splitsize,const string& splitFormat)
    :Appender(path),path_(path),splitSize_(splitsize),checkTimeInterval_(1),lastcheck_(0),fd_(-1),splitFormat_(splitFormat)
{
    CreateDir(path_);
    fd_ = openLogFile(path);
    if(fd_ < 0){
        // ��ռ���ӣ���Ȼ�������޷����ļ���������Ϊ·��Ȩ�����⣬���Ȱ���־ˢ��stderr
        // ���й������˹��޸�·��Ȩ�ޣ������ܴ��ļ��ˣ���ʱ��checkFile�����У�fd_ԭ����ָ�����ļ����Ҿ���д��
        fd_ = open("/dev/stderr",O_WRONLY);
    }
    
    if(splitSize_ <= 0){
        splitSize_ = DEFAULT_SPLITSIZE;
    }
    else if(splitSize_ < MIN_SPLITSIZE){
        splitSize_ = MIN_SPLITSIZE;
    }
    else if(splitSize_ > MAX_SPLITSIZE){
        splitSize_ = MAX_SPLITSIZE;
    }

    generateSplitFormatCharMap();
    parseSplitFormat();
}

void FileAppender::generateSplitFormatCharMap()
{
    formatsCharMap_['Y']=format_t::YEAR;
    formatsCharMap_['m']=format_t::MONTH;
    formatsCharMap_['d']=format_t::DAY;
    formatsCharMap_['H']=format_t::HOUR;
    formatsCharMap_['M']=format_t::MINUTE;
    formatsCharMap_['S']=format_t::SECOND;
    formatsCharMap_['P']=format_t::PID;
    formatsCharMap_['n']=format_t::SEQNUMBER;
}

void FileAppender::parseSplitFormat()
{
    
    string::size_type pos = 0;
    string::size_type find_pos = 0;
    string::size_type use;
    string tmp;
    
    use = splitFormat_.find('%');
    while(use != string::npos){
        assert(pos <= use);
        format_t fmt;
        if(use+1 != string::npos){
            charToFormatMap::iterator it=formatsCharMap_.find(splitFormat_[use+1]);
            if(it!=formatsCharMap_.end()){
                tmp = splitFormat_.substr(pos,use-pos);
                if(!tmp.empty()){
                    fmt.str_ = tmp;
                }
                fmt.type_ = formatsCharMap_[splitFormat_[use+1]];
                formats_.push_back(fmt);
                pos = use+2;
                find_pos = pos;
            }
            else{
                find_pos = use+1;
            }
        }
        else{
            //%H%m%d_xxx% ���һ���ַ�����%
            fmt.type_ = format_t::RAW_STR;
            fmt.str_ = splitFormat_.substr(pos,splitFormat_.length()-pos);
            formats_.push_back(fmt);
            break;
        }
        
        use = splitFormat_.find('%',find_pos);
    }
    format_t fmt;
    // %H%m%dxx �����RAW_STR
    if(pos != string::npos){
        fmt.type_ = format_t::RAW_STR;
        fmt.str_ = splitFormat_.substr(pos,splitFormat_.length()-pos);
        formats_.push_back(fmt);
    }
}

FileAppender::~FileAppender()
{
    if(fd_ >= 0)
        ::close(fd_);
}

int FileAppender::openLogFile(const std::string& filePath)
{
    return open(filePath.c_str(), O_CREAT | O_WRONLY | O_LARGEFILE | O_APPEND, 0755);
}

void FileAppender::reopen()
{
    // if fd == -1,û��Ч����Ҳû���κθ����á�fd_����Ȼָ����ļ����´μ�黹�ᷢ��inode�仯���������ԡ�
    // ���open���ڲ���Ŀ��fd��Ҳ����open����fd_һ������ֵ��dup2���� EBUSY��������Ϊ���Ǳ�֤��fd_ʼ����Ч��open��Ȼ���᷵��fd_һ������ֵ������dupҪô�ɹ���Ҫô��Ϊfd==-1��ʧ�ܣ�������˵��fd=-1û��Ӱ�졣
    // dup2��ԭ�ӵġ�������һ�㣬���ǲ��ö��̼߳����ˣ���һ���������Ա��Ŀ�Դtbcommon-utils�е�tblog����л��¡����

    int fd = openLogFile(path_);
    if(fd < 0 && errno == ENOENT){
        CreateDir(path_);
        fd = openLogFile(path_);
    }
    dup2(fd, fd_);
    close(fd);

    // ����һ��Ҫ˵����ͬһ�����̣����openͬһ���ļ���ÿ��fd��ͬ���ҿ�����������������ļ�һ��fd��һ���Ļ���fd=open();close(fd);���̲߳�������£����ǿ��ܻ���closeͬһ��fd�������fd������close֮���п��ܱ�open���������豸������������һ������fd���Ǿͱ����ˡ�
}

string FileAppender::getNewFilename(time_t now,int seqNumber)const
{
    struct tm newTimeObj;
    localtime_r(&now,&newTimeObj);
    char tmpStr[16]={0};
    
    // ��ͬdate����ĸ�ʽ%Y%m%dH%M%S
    // %P���̺ţ�%n���кţ���000��ʼ�����ͬ���ļ����ڣ��Զ��ۼ�ֱ���ҵ�һ���������֣�����999. example: a.log_%Y%m%d%H%M%S_%P_%n,���a.log.20121010090048_3164_000��ռ�ã���ȥ��a.log.20121010090048_3164_001�Ƿ����
    stringstream stream;
    stream<<path_;
    
    for (vector<format_t>::const_iterator it=formats_.begin(); it != formats_.end(); ++it)
    {
        switch(it->type_){
            case format_t::RAW_STR:
                stream<<it->str_;
                break;
                
            case format_t::YEAR:
                snprintf(tmpStr,sizeof(tmpStr),"%04d",newTimeObj.tm_year+1900);
                stream<<it->str_;
                stream<<tmpStr;
                break;
            
            case format_t::MONTH:
                snprintf(tmpStr,sizeof(tmpStr),"%02d",newTimeObj.tm_mon+1);
                stream<<it->str_;
                stream<<tmpStr;
                break;

            case format_t::DAY:
                snprintf(tmpStr,sizeof(tmpStr),"%02d",newTimeObj.tm_mday);
                stream<<it->str_;
                stream<<tmpStr;
                break;

            case format_t::HOUR:
                snprintf(tmpStr,sizeof(tmpStr),"%02d",newTimeObj.tm_hour);
                stream<<it->str_;
                stream<<tmpStr;
                break;

            case format_t::MINUTE:
                snprintf(tmpStr,sizeof(tmpStr),"%02d",newTimeObj.tm_min);
                stream<<it->str_;
                stream<<tmpStr;
                break;

            case format_t::SECOND:
                snprintf(tmpStr,sizeof(tmpStr),"%02d",newTimeObj.tm_sec);
                stream<<it->str_;
                stream<<tmpStr;
                break;

            case format_t::PID:
                snprintf(tmpStr,sizeof(tmpStr),"%d",getpid());
                stream<<it->str_;
                stream<<tmpStr;
                break;

            case format_t::SEQNUMBER:
                snprintf(tmpStr,sizeof(tmpStr),"%03d",seqNumber);
                stream<<it->str_;
                stream<<tmpStr;
                break;

            default:
                fprintf(stderr,"%s(%s:%d) fatal error,fileAppender met unknown format type!",__func__,__FILE__,__LINE__);
                abort();
                break;
        }
    }
    return stream.str();
}


void FileAppender::checkFile()
{
    // ��һ���������һ���Ƿ���Ҫ���´򿪣�ÿ�ζ����Ч��̫��.�ļ��зִ�С����������ⲻ��
    struct timeval* tm = TestSetCurrentTm();    
    time_t now = tm->tv_sec;
    {
        // �˴���ԭ�Ӳ���Ҳ��ʵ�֡���ǰ�ҳ�gcc/g++�汾�¾ɣ�û��__sync_lock_test_and_set���ָ߼���ɫ
        // linux�Դ���atomic����������Ҳ�����û���һ�Σ��Ǹ�������ֲ��.
        boost::mutex::scoped_lock guard(checkMutex_);
        if(lastcheck_ + checkTimeInterval_ > now){
            return;
        }
        lastcheck_ = now;
    }
    
        
// 1.��鵱ǰfd�Ƿ�ָ��log�ļ���������˵����mv����rm�ˣ�����openLogFile()
// 2.���log�ļ��Ĵ�С������Ѿ������зִ�С��mv��open
    struct stat stFile;
    struct stat stFd;
    memset(&stFd,0,sizeof(stFd));
    memset(&stFile,0,sizeof(stFile));
    
    // ��һ�α�֤����־�ļ���ȡ��Чfd_.֮��ֻ�гɹ�open�����ļ��Ż�dup��fd_��ȥ����֤fd_һֱ����Ч�ġ�
    fstat(fd_, &stFd);
    int err = stat(path_.c_str(), &stFile);
    if ((err == -1 && errno == ENOENT)
        || (err == 0 && (stFile.st_dev != stFd.st_dev || stFile.st_ino != stFd.st_ino))) {

        // ��ϸ�Ķ�reopen˵����
        reopen();
        // �������Ѿ�������reopen,��ο��Բ�Ҫ�ټ���ˡ�
        return;
    }
    
    if(err == 0 && static_cast<uint64_t>(stFile.st_size) >= splitSize_){
        // mv reopen.
        string newFileName;
        bool foundFileName=false;//�Ƿ��ҵ�һ�����õ��ļ���
        for (int surfix = 0; surfix < 1000; ++surfix)
        {
            newFileName=getNewFilename(now,surfix);
            struct stat checkBuf;
            if(stat(newFileName.c_str(),&checkBuf) < 0){
                foundFileName=true;
                break;          // �ҵ�һ�������ڵ��ļ���
            }
        }
        // rename,reopen
        if(foundFileName){
            boost::mutex::scoped_lock guard(splitMutex_);
            struct stat stAgain;
            if(stat(path_.c_str(),&stAgain) == 0 && stAgain.st_dev == stFile.st_dev && stAgain.st_ino == stFile.st_ino){
                // �ļ������Ǹ��ļ����ҿ��Բ�����Ҫ���ļ��Ѿ����ˣ�˵�����˸պ��й��ˣ�����ǵ��͵�double-check����
                // ���߳�����£����������߳�ͬʱ�ߵ��������rename���ܰѸ�rename�������ļ����ǵ�����־�Ͷ��ˡ�
                // ����mutex��Ȼ�޷��赲�����ͬʱrotate�����Ҫ�������̵����⣬��Ȼ����fcntl�ļ�������system v sem�ź���
                // ������ĿǰΨһ���ڽ����˳�ʱ�Զ��������ķ�ʽ��Ȼ��fcntl������һ���������ļ�����һ�˹���������ļ�������ˣ�
                // �������Զ�����������־������find . -ctime +15 |xargs rm.��ʱ��ͷǳ��鷳�ˣ�system v �ź���������unpv2��������
                // �����ͳ�ʼ������һ��ԭ�ӵģ�ĿǰҲû�������ķ�ʽ���ֻ��һ�����̳�ʼ�����������̵ȳ�ʼ��������ʹ�õķ����������մ�����������ˣ�������ԶҲ�޷��ȵ���ʼ����ɡ�
                // ��֮��֧�ֶ���̻�����ļ��зַǳ��鷳�����ѣ�Ҳ���п��׵ķ����������Ҿ��õò���ʧ��
                // ��ʵ����������Ƕ���̵ģ���ô����ֻҪ���з��ļ��ĸ�ʽ�����%P�������̺ż��룬�Ϳ��Ա�֤���̼����ļ����Բ����໥���ǡ���˼򵥶��ѡ�
                
                rename(path_.c_str(),newFileName.c_str());
                reopen();
            }
        }
        else{
            // shit,����1000�ξ�Ȼ����ռ���ˣ�ֻ�ü�����ԭ�����ļ�����һ���ٳ����з֣���ʱʱ����ˣ������п����ļ�����
            // do nothing
        }
    }
}

void FileAppender::output(char* msg,size_t len)
{
    checkFile();
    
    ssize_t ret = ::write(fd_,msg,len);
    (void)ret;
    free(msg);
}

void FileAppender::outputWithoutLock(char* msg,size_t len)
{
    checkFile();
    
    ssize_t left=len;
    do{
        ssize_t nwrite =  ::write(fd_,msg+(len-left),left);
        if(nwrite >= 0){
            left -= nwrite;
        }
        else{
            // error.
            fprintf(stderr,"error=write_fail logger=%s filepath=%s fd=%d errno=%d log=%s\n",this->getAppenderName().c_str(),path_.c_str(),fd_,errno,msg);
            break;
        }
    }while(left>0);
}
