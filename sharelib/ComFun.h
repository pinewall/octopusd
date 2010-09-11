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

#ifndef NODB  //����Ƿ����ݿ�ģʽ����������ݿ���Ϣ
#include <mysql/mysql.h>
#endif

#include "ThreadMutex.h"

using namespace std;

const int LenOfSiteName = 32;   //վ�����Ƴ���
const int LenOfTime = 20;       //ʱ���ʽ����
const int LenOfPathFileName = 256;  //·�����ļ����Ƶĳ���
const int LenOfIPAddr = 20;   // IP��ַ
const int LenOfNormal = 64;  //һ��ı�������

//�ļ�������
 enum Fun_FileType
{
        FILE_ERROR = 0,  //����
        FILE_DIR = 1, //Ŀ¼
        FILE_TARGZ = 2, // *.tar.gz
        FILE_ZIP = 3,  // *.zip
        FILE_NORMAL = 4 //��ͨ�ļ�
};

//����������
class CComFun
{
      public:
        CComFun() {}
        ~CComFun() {}

        /**
         * \brief ���һ��IP��ַ�Ƿ���һ����Ч��IP
         * \param sIP �ַ�����ʽ��IP��ַ
         * \return 0 �ɹ� -1 ����
         */
        static int checkIP(string sIP);

        /**
         * \brief ��һ���ַ�����ʽ��IPת����unsigned int��IP
         * \param sIP �ַ�����ʽ��IP��ַ
         * \param uiIP �������, ui��ʽ��IP
         * \return 0 �ɹ� -1 ����
         */
        static int convertSIP2UIIP(string sIP, unsigned int& uiIP);

        /**
         * \brief ͨ���ָ���,����ַ������Ӵ�
         * \param pDest �������,�Ӵ���ֵ
         * \param pSrc ԭ�ַ���
         * \param ppNextSrc  �������,ʣ���ַ�����ָ��,�Ѿ������˷ָ������ַ�
         * \param chFlag �ָ��ַ�
         * \return 0 �ɹ� -1 ����
         */
        static int getFlagStr(char *pDest, char *pSrc, char **ppNextSrc, char chFlag);

        /**
         * \brief ͨ���ָ���,����ַ������Ӵ�
         * \param vecDest ���vector
         * \param sSrc ԭ�ַ���
         * \param chFlag �ָ��ַ�
         * \return 0 �ɹ� -1 ����
         */
        static int divideupStr(const char* sSrc, vector<string>& vecDest, char chFlag);

        /**
         * \brief ��ȡһ����������ʽ���ļ�  Name = Value, ���ÿ�е�����
         * \param sPathFile �ļ�����,����·��
         * \param mapNameValue �������,��������map
         * \return 0 �ɹ� -1 ����
         */
        static int readEqualValueFile(string sPathFile, map<string, string>& mapNameValue);

        /**
         * \brief ���˵��ַ����Ŀո��TAB��
         * \param pDest �������, ���˺��ֵ
         * \param pSrc ԭ�ַ���
         * \param iMode ����ģʽ, 0 ǰ��඼����, 1 ǰ�� 2���
         * \return 0 �ɹ� -1 ����
         */
        static int trimString(char* pDest, char *pSrc, int iMode);   

        /**
         * \brief ��ȷ�����뼶��ĵȴ�
         * \param tv_sec   ����
         * \param tv_msec  ������(ms)
         * \return 0 �ɹ� ���� ����
         */
        static int sleep_msec(long tv_sec, long tv_msec );

        /**
         * \brief ��ȡ��ǰϵͳ��ʱ�䣬�ַ�������
         * \param void
         * \return �ַ�����ʽ��ʱ�䣬
         *  GetCurDateTime��ʽΪ%04d-%02d-%02d %02d:%02d:%02d
         *  GetExactCurDateTime��ʽΪ%04d-%02d-%02d %02d:%02d:%02d %06d(usec)
         *  GetMsgCurDateTime 20λ(����΢��ȡǰ5λ) YYYYMMDDHHMMSSTTTTT
         */
        static string GetCurDate();
        static string GetCurDateTime();
        static string GetExactCurDateTime();
        static string GetMsgCurDateTime();

        /**
         * \brief ��2���ַ������ϳ�һ��·�����������Ϊ�գ���ֻ��鲢����Ŀ¼
         * \param sPathFile1 �ļ�����,����·��
         * \param sPathFile2 �ļ�����,����·��
         * \param iCreateDirFlag Ŀ¼����������Ƿ񴴽� 0 ����Ҫ 1��Ҫ
         * \return sPathFile �ɹ� ""����
         */
         static CThreadMutex m_systemCallMutex;
         static string analysePathFile(string sPathFile1, string sPathFile2,  int iCreateDirFlag);

        /**
         * \brief ϵͳ�����ķ�װ
         * \param sPathFile1 �ļ�����,����·��
         * \param sPathFile2 �ļ�����,����·��
         * \return 0 successfully other fail
         */        
        static int delFile(string sPathFile);
        static int chmodFile(string sPathFile, string sFileModeOption);
        static int mvFile(string sPathFile1, string sPathFile2);
        static int cpFile(string sPathFile1, string sPathFile2);
        static int unpackFile(string sPathName);

        /**
         * \brief ����������ָ�����ȣ��ֳɹ̶�����
         * \param sSrc ԭʼ��Ϣ
         * \param mapDec ���洦������Ϣ
         * \param iNrOfWord ÿ�εĳ���
         * \return 0 successfully other fail
         */
        static int getSubStrByWord(string sSrc, map<int, string> &mapDec, int iNrOfWord);

        /**
         * \brief �Ƿ���·��
         * \param sPathFile Ҫ�жϵ�·����
         * \return 0 ����·�� 1 ��·��
         */
        static int isDirectory(string sPathFile);

         /**
         * \brief ���ַ����в��ҵ���Ӧ���ַ�����Ȼ���滻���µ�����
         * \param sInput ԭʼ�ַ��� sFind Ҫ�ҵ��ַ��� sReplaced Ҫ�滻�ɵ��ַ��� iReplaceType�滻��� 0���滻 1�滻��һ�� 2ȫ���滻
         * \return 0 ����·�� 1 ��·��
         */
         static void find8Replace(string& sInput, const string& sFind, const string& sReplaced, int iReplaceType);

        //�ָ�Ŀ¼�ļ�Ϊ,Ŀ¼���ļ�����
        static void divPathFile(string sPathFile, string& sPath, string& sFileName);

        //����ļ������ͣ���Ϊ��һ . ֮���ȫ������չ��,��ҵ���Լ���֤
        //Ҫ����������������ȫ·��,���򽫻���ֽ�������
        static Fun_FileType getFileType(string sFile);

        //��û�����IP
        static int getHostName8IP(string& sName, string& sIP);
        //��Ҫroot�����ſ���
        static int getHostName8IP(const char * sDevice, string & sMac, string & sMask, string & sIP);

        //��Ϣ�Ŀ��������ϵ��
        //��������pCur ��ǰ��������λ��   value ��Ҫ�����ֵ  offset�����ƫ�Ƶ�ַ
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

        //��ȡ�����ļ�������
        static int readWholeFileContent(const string& sFileName, string& sContent);
        static int writeWholeFileContent(const string& sFileName, const string& sContent);

};

#endif /* COMFUN_H_HEADER_INCLUDED_BCE1841D */

