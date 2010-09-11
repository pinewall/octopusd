//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ComFun.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef COMFUN_H_HEADER_INCLUDED_BCE1841D
#define COMFUN_H_HEADER_INCLUDED_BCE1841D

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stropts.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <setjmp.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <limits.h>
#include <assert.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdarg.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <algorithm>

#include <sys/ioctl.h>
#include <net/if.h> 
#include <net/if_arp.h> 
#include <net/ethernet.h> 

#ifndef NODB  //如果是非数据库模式，则包括数据库信息
#include <mysql/mysql.h>
#endif

#include "ThreadMutex.h"

using namespace std;

const int LenOfSiteName = 32;   //站点名称长度
const int LenOfTime = 20;       //时间格式长度
const int LenOfPathFileName = 256;  //路径和文件名称的长度
const int LenOfIPAddr = 20;   // IP地址
const int LenOfNormal = 64;  //一般的变量长度

//文件的类型
 enum Fun_FileType
{
        FILE_ERROR = 0,  //错误
        FILE_DIR = 1, //目录
        FILE_TARGZ = 2, // *.tar.gz
        FILE_ZIP = 3,  // *.zip
        FILE_NORMAL = 4 //普通文件
};

//公共函数类
class CComFun
{
      public:
        CComFun() {}
        ~CComFun() {}

        /**
         * \brief 检查一个IP地址是否是一个有效的IP
         * \param sIP 字符串形式的IP地址
         * \return 0 成功 -1 出错
         */
        static int checkIP(string sIP);

        /**
         * \brief 将一个字符串形式的IP转化成unsigned int的IP
         * \param sIP 字符串形式的IP地址
         * \param uiIP 输出参数, ui形式的IP
         * \return 0 成功 -1 出错
         */
        static int convertSIP2UIIP(string sIP, unsigned int& uiIP);

        /**
         * \brief 通过分隔符,获得字符串的子串
         * \param pDest 输出参数,子串的值
         * \param pSrc 原字符串
         * \param ppNextSrc  输出参数,剩余字符的首指针,已经跳过了分隔符的字符
         * \param chFlag 分隔字符
         * \return 0 成功 -1 出错
         */
        static int getFlagStr(char *pDest, char *pSrc, char **ppNextSrc, char chFlag);

        /**
         * \brief 通过分隔符,获得字符串的子串
         * \param vecDest 输出vector
         * \param sSrc 原字符串
         * \param chFlag 分隔字符
         * \return 0 成功 -1 出错
         */
        static int divideupStr(const char* sSrc, vector<string>& vecDest, char chFlag);

        /**
         * \brief 读取一个这样的形式的文件  Name = Value, 获得每行的内容
         * \param sPathFile 文件名称,包含路径
         * \param mapNameValue 输出参数,保存结果的map
         * \return 0 成功 -1 出错
         */
        static int readEqualValueFile(string sPathFile, map<string, string>& mapNameValue);

        /**
         * \brief 过滤掉字符串的空格和TAB键
         * \param pDest 输出参数, 过滤后的值
         * \param pSrc 原字符串
         * \param iMode 过滤模式, 0 前后侧都过滤, 1 前侧 2后侧
         * \return 0 成功 -1 出错
         */
        static int trimString(char* pDest, char *pSrc, int iMode);   

        /**
         * \brief 精确到毫秒级别的等待
         * \param tv_sec   秒数
         * \param tv_msec  毫秒数(ms)
         * \return 0 成功 其他 出错
         */
        static int sleep_msec(long tv_sec, long tv_msec );

        /**
         * \brief 获取当前系统的时间，字符串类型
         * \param void
         * \return 字符串形式的时间，
         *  GetCurDateTime格式为%04d-%02d-%02d %02d:%02d:%02d
         *  GetExactCurDateTime格式为%04d-%02d-%02d %02d:%02d:%02d %06d(usec)
         *  GetMsgCurDateTime 20位(最后的微妙取前5位) YYYYMMDDHHMMSSTTTTT
         */
        static string GetCurDate();
        static string GetCurDateTime();
        static string GetExactCurDateTime();
        static string GetMsgCurDateTime();

        /**
         * \brief 用2个字符串，合成一个路径，如果后者为空，则只检查并创建目录
         * \param sPathFile1 文件名称,包含路径
         * \param sPathFile2 文件名称,包含路径
         * \param iCreateDirFlag 目录如果不存在是否创建 0 不需要 1需要
         * \return sPathFile 成功 ""出错
         */
         static CThreadMutex m_systemCallMutex;
         static string analysePathFile(string sPathFile1, string sPathFile2,  int iCreateDirFlag);

        /**
         * \brief 系统操作的封装
         * \param sPathFile1 文件名称,包含路径
         * \param sPathFile2 文件名称,包含路径
         * \return 0 successfully other fail
         */        
        static int delFile(string sPathFile);
        static int chmodFile(string sPathFile, string sFileModeOption);
        static int mvFile(string sPathFile1, string sPathFile2);
        static int cpFile(string sPathFile1, string sPathFile2);
        static int unpackFile(string sPathName);

        /**
         * \brief 将长串按照指定长度，分成固定的组
         * \param sSrc 原始信息
         * \param mapDec 保存处理后的信息
         * \param iNrOfWord 每段的长度
         * \return 0 successfully other fail
         */
        static int getSubStrByWord(string sSrc, map<int, string> &mapDec, int iNrOfWord);

        /**
         * \brief 是否是路径
         * \param sPathFile 要判断的路径名
         * \return 0 不是路径 1 是路径
         */
        static int isDirectory(string sPathFile);

         /**
         * \brief 在字符串中查找到相应的字符串后，然后替换掉新的内容
         * \param sInput 原始字符串 sFind 要找的字符串 sReplaced 要替换成的字符串 iReplaceType替换类别 0不替换 1替换第一个 2全部替换
         * \return 0 不是路径 1 是路径
         */
         static void find8Replace(string& sInput, const string& sFind, const string& sReplaced, int iReplaceType);

        //分割目录文件为,目录和文件名称
        static void divPathFile(string sPathFile, string& sPath, string& sFileName);

        //获得文件的类型，认为第一 . 之后的全部是扩展名,请业务自己保证
        //要求输入条件必须是全路径,否则将会出现解析错误
        static Fun_FileType getFileType(string sFile);

        //获得机器的IP
        static int getHostName8IP(string& sName, string& sIP);
        //需要root启动才可以
        static int getHostName8IP(const char * sDevice, string & sMac, string & sMask, string & sIP);

        //消息的拷贝和黏贴系列
        //三个参数pCur 当前缓冲区的位置   value 需要加入的值  offset加入的偏移地址
        static unsigned int msg_putChar(char* pCur, char value, int offset);
        static unsigned int msg_putShort(char* pCur, unsigned short value, int offset);
        static unsigned int msg_putLong(char* pCur, unsigned long value, int offset);
        static unsigned int msg_putInt(char* pCur, int value, int offset);
        static unsigned int msg_putString(char* pCur, const char* value, int iValueLen, int offset);

        static  unsigned int msg_getChar(const char* pCur, int offset, char& value);
        static  unsigned int msg_getShort(const char* pCur, int offset, unsigned short& value);
        static  unsigned int msg_getLong(const char* pCur, int offset, unsigned long& value);
        static  unsigned int msg_getInt(const char* pCur, int offset, int& value);
        static  unsigned int msg_getString(const char* pCur, int offset, string& value, int iValueLen);

        //读取整个文件的内容
        static int readWholeFileContent(const string& sFileName, string& sContent);
        static int writeWholeFileContent(const string& sFileName, const string& sContent);

};

#endif /* COMFUN_H_HEADER_INCLUDED_BCE1841D */

