//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *
//  *    $RCSfile: ComFun.cpp $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "ComFun.h"


 CThreadMutex CComFun::m_systemCallMutex;

int CComFun::checkIP(string sIP)
{
        if (0 == sIP.size() || 15 < sIP.size())
        {
                return -1;
        }

        unsigned int uiIP;
        return convertSIP2UIIP(sIP, uiIP);
}

int CComFun::convertSIP2UIIP(string sIP, unsigned int& uiIP)
 {
        if (0 == sIP.size() || 15 < sIP.size())
        {
                return -1;
        }
        char sTmpResult[4];

        char sTmpStr[20];
        char sTmpStr1[20];
        strcpy(sTmpStr, sIP.c_str());
        char *str1, *str2;

        //��һ��
        str1 = sTmpStr;
        if (0 != getFlagStr(sTmpStr1, str1, &str2, '.'))
                return -1;
        sTmpResult[0] = atoi(sTmpStr1);

        //�ڶ���
        str1 = str2;
        if (0 != getFlagStr(sTmpStr1, str1, &str2, '.'))
                return -1;
        sTmpResult[1] = atoi(sTmpStr1);

         //������
        str1 = str2;
        if (0 != getFlagStr(sTmpStr1, str1, &str2, '.'))
                return -1;
        sTmpResult[2] = atoi(sTmpStr1);

        //���ĸ�
        sTmpResult[3] = atoi(str2);

        memcpy((char*)&uiIP, sTmpResult, 4);
        uiIP = ntohl(uiIP);

        return 0;


 }

int CComFun::getFlagStr(char *pDest, char *pSrc, char **ppNextSrc, char chFlag)
{
        char * pTmp;
        int    iFlag;
        int    iLen;

        //�Ƿ����Ҫ���ҵ��ַ�
        iFlag = (int)chFlag;
        pTmp = strchr(pSrc, iFlag);
        if (NULL == pTmp)
        {
            return -1;
        }

        //����ַ����ƶ�����һ���ֵĿ�ͷ
        iLen = pTmp - pSrc;
        memcpy(pDest, pSrc, iLen);
        pDest[iLen] = 0;

        pTmp = pTmp + 1;
        *ppNextSrc = pTmp;

        return 0;
}

int CComFun::divideupStr(const char* sSrc, vector<string>& vecDest, char chFlag)
{
        if ("" == sSrc)
        {
                return 0;
        }
        char sTmpSrc[4096 + 1];
        char *str1, *str2;
        char sValue[256 + 1];
        strcpy(sTmpSrc, sSrc);
        str1 = sTmpSrc;
        vector<string> tmpvec;

        while (getFlagStr(sValue, str1, &str2, chFlag) == 0)
        {
                if (0 != strlen(sValue))
                {                        
                        tmpvec.push_back(string(sValue));
                }
                str1 = str2;
        }

        if (0 != strlen(str1))
        {
                tmpvec.push_back(string(str1));
        }

        vecDest = tmpvec;
        return 0;
}

int CComFun::trimString(char* pDest, char *pSrc, int iMode)
{
        // �˺���Ҫ��Ŀ���ַ����������㹻�ĳ���,�����ܳ��Ȳ�Ҫ����4096��
        //����Ϊ����ຯ�������û���ϸ��У��,��ʹ��������
        if (NULL == pDest || NULL == pSrc)
        {
                return -1;
        }

        char *p1, *p2;

        //ֻ������sTmp,�Ƿ�ֹpSrc,pDest��һ���ַ��������
        char sTmp[4096];
        strcpy(sTmp, pSrc);
        p1 = sTmp;
        p2 = sTmp + (strlen(sTmp) - 1);

        while( ' ' == *p1|| '\t' == *p1)
        {
                p1++;
        }
        while( ' ' == *p2|| '\t' == *p2)
        {
                p2--;
        }

        // ȫ��
        if (p1 > p2)
        {
                pDest[0] = '\0';
                return 0;
        }

       if ( 0 == iMode)  // 2 ��
        {
                strncpy(pDest, p1, p2 -p1 + 1);
                pDest[p2 -p1 + 1] = '\0';
        }
        else  if (1 == iMode) // �� ��
        {
                strcpy(pDest, p1);
        }
        else  // �� ��
        {
                p2++;
                *p2 = '\0';
                strcpy(pDest, sTmp);
        }

        return 0;
}

int CComFun::readEqualValueFile(string sPathFile, map<string, string>& mapNameValue)
{
        FILE *fp = fopen(sPathFile.c_str(), "r");
        if (NULL == fp)
        {
                return -1;
        }

        char sTmpStr[1024 + 1];
        char sTmpStr1[1024 + 1];
        char sTmpStr2[1024 + 1];
        char *pNext;

        while (1)
        {
                if (NULL == fgets(sTmpStr, 1024, fp))
                {
                        break;
                }
                trimString(sTmpStr, sTmpStr,0);
                if ('#' == sTmpStr[0] || '[' == sTmpStr[0])
                {
                        continue;
                }
                if (0 != getFlagStr(sTmpStr1, sTmpStr, &pNext, '='))
                {
                        continue;
                }
                trimString(sTmpStr1, sTmpStr1, 0);  // ����Name

                strcpy(sTmpStr2, pNext);
                while ('\n' == sTmpStr2[strlen(sTmpStr2) -1] || '\r' == sTmpStr2[strlen(sTmpStr2) -1] )
                {
                        sTmpStr2[strlen(sTmpStr2) -1] = '\0';  // ȥ��ĩβ�Ļس�����
                        if (0 == strlen(sTmpStr2))
                                break;
                }
                trimString(sTmpStr2, sTmpStr2, 0);  // ����value

                mapNameValue[sTmpStr1] = sTmpStr2;

        }
        fclose(fp);

        return 0;
}

int CComFun::sleep_msec(long tv_sec, long tv_msec )
{
        struct timeval tv;
        tv.tv_sec = tv_sec;
        tv.tv_usec = tv_msec*1000;
        return( select( 1, NULL, NULL, NULL, &tv) );
}

string CComFun::GetCurDateTime()
{

        time_t iCurTime;
        time(&iCurTime);
        char s[50];
        struct tm *curr = localtime(&iCurTime);
        if (curr->tm_year > 50)
        {
                sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",
                                curr->tm_year+1900, curr->tm_mon+1, curr->tm_mday,
                                curr->tm_hour, curr->tm_min, curr->tm_sec);
        }
        else
        {
                sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d",
                                curr->tm_year+2000, curr->tm_mon+1, curr->tm_mday,
                                curr->tm_hour, curr->tm_min, curr->tm_sec);
        }

        return s;
}

string CComFun::GetCurDate()
{
        time_t iCurTime;
        time(&iCurTime);
        char s[50];
        struct tm *curr = localtime(&iCurTime);
        if (curr->tm_year > 50)
        {
                sprintf(s, "%04d%02d%02d",
                                curr->tm_year+1900, curr->tm_mon+1, curr->tm_mday);
        }
        else
        {
                sprintf(s, "%04d%02d%02d",
                                curr->tm_year+2000, curr->tm_mon+1, curr->tm_mday);
        }
        return s;
}

string CComFun::GetExactCurDateTime()
{
        struct timeval now;
        gettimeofday(&now, 0);

        time_t iCurTime = now.tv_sec;
        char s[50];
        struct tm *curr = localtime(&iCurTime);
        if (curr->tm_year > 50)
        {
                sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d %06ld",
                                curr->tm_year+1900, curr->tm_mon+1, curr->tm_mday,
                                curr->tm_hour, curr->tm_min, curr->tm_sec, now.tv_usec);
        }
        else
        {
                sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d %06ld",
                                curr->tm_year+2000, curr->tm_mon+1, curr->tm_mday,
                                curr->tm_hour, curr->tm_min, curr->tm_sec, now.tv_usec);
        }

        return s;
}

string CComFun::GetMsgCurDateTime()
{
        struct timeval now;
        gettimeofday(&now, 0);

        time_t iCurTime = now.tv_sec;
        char s[50];
        struct tm *curr = localtime(&iCurTime);
        char sTmp[10];
        if (curr->tm_year > 50)
        {
                sprintf(sTmp, "%06ld", now.tv_usec);
                sTmp[5] = '\0';
                sprintf(s, "%04d%02d%02d%02d%02d%02d%s",
                                curr->tm_year+1900, curr->tm_mon+1, curr->tm_mday,
                                curr->tm_hour, curr->tm_min, curr->tm_sec, sTmp);
        }
        else
        {
                sprintf(sTmp, "%06ld", now.tv_usec);
                sTmp[5] = '\0';
                sprintf(s, "%04d%02d%02d%02d%02d%02d%s",
                                curr->tm_year+2000, curr->tm_mon+1, curr->tm_mday,
                                curr->tm_hour, curr->tm_min, curr->tm_sec, sTmp);
        }

        return s;
}

string CComFun::analysePathFile(string sPathFile1, string sPathFile2, int iCreateDirFlag)
{
        if ("" == sPathFile1 || "" == sPathFile2)
        {
                return "";
        }
        
        //���·��
        string sPathFile;
        if ('/' == sPathFile2[0] && '/' == sPathFile1[sPathFile1.size() -1] )
        {
                const char *px = sPathFile2.c_str();
                px++;
                sPathFile = sPathFile1 + px;
        }
        else if ('/' == sPathFile2[0] || '/' == sPathFile1[sPathFile1.size() -1] )
        {
                sPathFile = sPathFile1 + sPathFile2;
        }
        else  // != && !=
        {
                sPathFile = sPathFile1 + "/" + sPathFile2;
        }

        //�ж��ļ��Ƿ����,�������,������Ŀ¼
        struct stat st1;
        if(stat(sPathFile.c_str(), &st1)== 0)
        {
                if (S_ISDIR(st1.st_mode))
                {
                        return "";
                }
        }

        //Ŀ¼��������ڣ�������
        if (0 == iCreateDirFlag)
        {
                return sPathFile;
        }

        //��ȡ·��
        char sTmpStr[LenOfPathFileName + 1];
        strcpy(sTmpStr, sPathFile.c_str());

         int iTmpLen = strlen(sTmpStr);
         int iLoop = 0;
         for (iLoop = iTmpLen -1; iLoop >= 0; iLoop--)
         {
              if (sTmpStr[iLoop] == '/')
              {
                   break;
              }
         }
         sTmpStr[iLoop + 1] = '\0';

         //���ʹ���Ŀ¼
         int n = 3;
         while(n > 0)
         {
                  if (0 != access(sTmpStr,  F_OK))
                  {
                        char sTmpSysCmd[2*LenOfPathFileName];
                        sprintf(sTmpSysCmd, "mkdir -p \"%s\"",  sTmpStr);

                        //�ȷ�����,��Ϊ�����������
                        //m_systemCallMutex.setLock();
                        int iRet = system(sTmpSysCmd);
                        //m_systemCallMutex.unLock();

                        if (0 != iRet)  
                        //ʧ���ˣ���������Ϊ�����̣߳������߳��Ѿ������ɹ����ٳ���һ��
                        {
                                 n--;  //�ڳ���һ��
                                 continue;
                        }                        
                  }
                  //�ɹ����˳�
                  break;
         }

         return sPathFile;
}

int CComFun::delFile(string sPathFile)
{
        //���ʹ���Ŀ¼
         if (0 == access(sPathFile.c_str(),  F_OK))
         {
                  //char sTmpSysCmd[2*LenOfPathFileName];
                  //sprintf(sTmpSysCmd, "rm -rf \"%s\"",  sPathFile.c_str());

                  //m_systemCallMutex.setLock();
                  //int iRet = system(sTmpSysCmd);
                  int iRet = remove(sPathFile.c_str());
                  //m_systemCallMutex.unLock();
                  if (0 != iRet)
                  {
                        return -1;
                  }
         }
         return 0;
}

int CComFun::chmodFile(string sPathFile, string sFileModeOption)
{
        //���ʹ���Ŀ¼
         if (0 == access(sPathFile.c_str(),  F_OK))
         {
                  //�Ƚ���shell��ִ��
                  char sTmpSysCmd[2*LenOfPathFileName];
                  sprintf(sTmpSysCmd, "chmod %s \"%s\"",  sFileModeOption.c_str(),  sPathFile.c_str());
                  int iRet = system(sTmpSysCmd);
                  if (0 != iRet)
                  {
                        return -1;
                  }
         }
         else
         {
                return -2; //�ļ�������
         }
         return 0;
}
int CComFun::mvFile(string sPathFile1, string sPathFile2)
{
        //���ʹ���Ŀ¼
         if (0 == access(sPathFile1.c_str(),  F_OK))
         {
                //rename �粻ͬ�Ĵ��̷����ᱨ�������Ϊ���ʧ�ܣ�����mv
                //��ǰ�ڲ��Ի�����û�����⣬��ʱ�����ж�errno�ķ���
                int iRet = rename(sPathFile1.c_str(), sPathFile2.c_str());
                if (0 != iRet)
                {
                        //Ϊ�˱�֤�������ٳ���ϵͳ����mv
                        char sTmpSysCmd[2*LenOfPathFileName];
                        sprintf(sTmpSysCmd, "mv \"%s\" \"%s\"",  sPathFile1.c_str(),  sPathFile2.c_str());
                        iRet = system(sTmpSysCmd);
                        if (0 != iRet)
                        {
                                return -1;
                        }
                }                 
         }
         else
        {
                return -1;
        }         
        return 0;
}

int CComFun::cpFile(string sPathFile1, string sPathFile2)
{
        //���ʹ���Ŀ¼
         if (0 == access(sPathFile1.c_str(),  F_OK))
         {
                  char sTmpSysCmd[2*LenOfPathFileName];
                  sprintf(sTmpSysCmd, "cp \"%s\" \"%s\"",  sPathFile1.c_str(),  sPathFile2.c_str());

                  //m_systemCallMutex.setLock();
                  int iRet = system(sTmpSysCmd);
                  //int iRet = rename(sPathFile1.c_str(), sPathFile2.c_str());
                  //m_systemCallMutex.unLock();
                  if (0 != iRet)
                  {
                        return -1;
                  }
         }
         else
         {
                return -1;
        }
         return 0;
}

int CComFun::unpackFile(string sPathName)
{
        string sDoLocalPath, sDoFileName;
        divPathFile(sPathName, sDoLocalPath, sDoFileName);

        Fun_FileType uType = getFileType(sPathName);
        
        //�л�����Ӧ��Ŀ¼��,�����ļ��Ӱ���Ȼ���ڷ���ԭ����Ŀ¼
        char sCmd[1024 + 1];
        sCmd[0] = '\0';
        if (FILE_TARGZ == uType)
        {
                snprintf(sCmd, 1024,"/bin/tar zxf %s -C %s", sPathName.c_str(), sDoLocalPath.c_str());
                int iRet = system(sCmd);
                if (-1 == iRet)
                {
                       return -3;
                } 
        }
        else if (FILE_ZIP == uType)
        {
                snprintf(sCmd, 1024,"/usr/bin/unzip -oq %s -d %s", sPathName.c_str(), sDoLocalPath.c_str());
                int iRet = system(sCmd);
                if (-1 == iRet)
                {
                       return -4;
                } 
        }
        else
        {
                return -5;
        }
        
        return 0;

}

int CComFun::getSubStrByWord(string sSrc, map<int, string> &mapDec, int iNrOfWord)
{
        if (iNrOfWord <= 0 )
        {
                return -1;
        }
        else if ((unsigned int)iNrOfWord >= sSrc.size())
        {
                mapDec[0] = sSrc;
                return 0;
        }
        int iMyNrOfWord = 0;
        int iArrayNo = 0;
        unsigned int iLoop = 0;
        int iFlag = 0;
        string sDesStr = "";
        
        while(iLoop < sSrc.size() -1)
        {
                if( ( (unsigned char)sSrc[iLoop]>=0x81 
                        && (unsigned char)sSrc[iLoop]<= 0xfe )
                        && ( (unsigned char)sSrc[iLoop+1]>=0x40 && (unsigned char)sSrc[iLoop+1]<=0xFE
                        && (unsigned char)sSrc[iLoop+1]!=0x7F ) )  // chinese
                {
                        if (iLoop == sSrc.size() -2)
                        {
                                iFlag = 1;
                        }
                        iMyNrOfWord++;
                        sDesStr = sDesStr + sSrc[iLoop] + sSrc[iLoop + 1];
                        iLoop = iLoop + 2;
                }
                else  // other
                {
                        iMyNrOfWord++;
                        sDesStr = sDesStr + sSrc[iLoop];
                        iLoop++;
                }
                if (iMyNrOfWord == iNrOfWord)
                {
                        mapDec[iArrayNo] = sDesStr;
                        iArrayNo++;
                        sDesStr = "";
                        iMyNrOfWord = 0;
                }
        }

        if (sDesStr.size() > 0)
        {
                mapDec[iArrayNo] = sDesStr;
        }

        return 0;
                
}

int CComFun::isDirectory(string sPathFile)
{
        //�ж��ļ��Ƿ����,�������,������Ŀ¼
        struct stat st1;
        if (stat(sPathFile.c_str(), &st1) != 0)
        {
                return -1;  //������
        }
        if(S_ISDIR(st1.st_mode))  //Ŀ¼
        {
                return  1;
        }
        return 0; //�����ļ�
}

void  CComFun::find8Replace(string& sInput, const string& sFind, const string& sReplaced,
         int iReplaceType)
{
        int iFindLen = sFind.length();
        int iReplacedLen = sReplaced.length();

        string::size_type lastPos=0;
        string::size_type pos = 0;

        if (2 == iReplaceType)
        {
                //�滻����
                while(string::npos != lastPos)
                {
                        pos = sInput.find(sFind, lastPos);
                        if(string::npos == pos) break;

                        sInput = sInput.substr(0, pos) + sReplaced + sInput.substr(pos+iFindLen);
                        lastPos = pos+iReplacedLen;
                }
        }
        else if (1 == iReplaceType)
        {
                //ֻ�滻��һ��
                pos = sInput.find(sFind, lastPos);
                if(string::npos == pos) return;

                sInput = sInput.substr(0, pos) + sReplaced + sInput.substr(pos+iFindLen);
        }
        return ;
}


void CComFun::divPathFile(string sPathFile, string& sPath, string& sFileName)
{
        //�Ƿ�Ϸ�
        if (0 == sPathFile.size())
        {
                return;
        }

        //�������һ�� / ����Ϊǰ��ľ���·��������ľ����ļ�����
        string::size_type lastPos = sPathFile.find_last_of("/");
        if (string::npos == lastPos)
        {
                return;
        }

        //ȡ���ļ�����
        sPath = sPathFile.substr(0, lastPos);
        sFileName = sPathFile.substr(lastPos+1);
        return;
}

//����ļ����еĺ�׺(�������ȡ��һ��.��������ж���)
Fun_FileType CComFun::getFileType(string sFile)
{
        //���ļ�
        if (sFile.size() == 0)
        {
                return FILE_ERROR;
        }

        //Ŀ¼
        int iRet = isDirectory(sFile);
        if(1 == iRet)
        {
                return  FILE_DIR;
        }
        else if (-1 == iRet)  //������
        {
                 return FILE_ERROR;
        }

        //��ͨ�ļ�
        string sPath, sFileName;
        divPathFile(sFile, sPath, sFileName);

        string::size_type pos = sFileName.find(".");
        //һ���ļ�
        if(string::npos == pos) 
        {
                return FILE_NORMAL;
        }
        
        string sTmp = sFileName.substr(pos+1);
        //gz
        if (sTmp == "tar.gz")
        {
                return FILE_TARGZ;
        }
        //zip
        else if (sTmp == "zip")
        {
                return FILE_ZIP;
        }
        else
        {
                return FILE_NORMAL;
        }

}

int CComFun::getHostName8IP(string& sName, string& sIP)
{
        //Name
        char sTmpName[100];
        if (0 != gethostname(sTmpName, 100))
        {
                return -1;
        }
        sName = sTmpName;

        //IP
        struct hostent *hp = gethostbyname(sTmpName) ;
        if(NULL == hp)
        {
                return -1;
        }

        struct in_addr  in;
        struct sockaddr_in  local_addr;
        memcpy(&local_addr.sin_addr.s_addr, hp->h_addr, 4);
        in.s_addr= local_addr.sin_addr.s_addr;
        sIP = inet_ntoa(in);

        return 0;
}

//�����豸�ţ����IP�����ֻ����root����ִ��
int CComFun::getHostName8IP(const char* sDevice,  string& sMac, string& sMask,  string& sIP)
{
        int     fd_arp;                 /* socket fd for receive packets */
        struct  ifreq   ifr;            /* ifr structure */
        struct  in_addr myself, mymask;

        struct sockaddr_in *sin_ptr;
        u_char             *ptr;

        //socket
        if ((fd_arp = socket(AF_INET, SOCK_PACKET, htons(0x0808))) < 0)
        {
                return -1;
        }

        strcpy(ifr.ifr_name, sDevice);

        // get ip address of my interface
        if (ioctl(fd_arp, SIOCGIFADDR, &ifr) < 0)
        {
                close(fd_arp);
                return -1;
        }
        sin_ptr = (struct sockaddr_in *)&ifr.ifr_addr;
        myself  = sin_ptr->sin_addr;

        // get network mask of my interface
        if (ioctl(fd_arp, SIOCGIFNETMASK, &ifr) < 0)
        {
                close(fd_arp);
                return -1;
        }
        sin_ptr = (struct sockaddr_in *)&ifr.ifr_addr;
        mymask  = sin_ptr->sin_addr;

        // get mac address of the interface
        if (ioctl(fd_arp, SIOCGIFHWADDR, &ifr) < 0)
        {
                close(fd_arp);
                return -1;
        }
        ptr = (u_char *)&ifr.ifr_ifru.ifru_hwaddr.sa_data[0];
        char sTmpMac[100];
        sprintf(sTmpMac, "%02x:%02x:%02x:%02x:%02x:%02x",
                *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3),
                *(ptr + 4), *(ptr + 5) );
        sMac = sTmpMac;
        sMask = inet_ntoa(mymask);
        sIP = inet_ntoa(myself);

        close(fd_arp);
        return 0;
}

//��Ϣ�ñ���
unsigned int CComFun::msg_putChar(char* pCur, char value, int offset)
{
        memcpy(pCur + offset, &value, sizeof(value));
        return sizeof(value);
}

unsigned int CComFun::msg_putShort(char* pCur, unsigned short value, int offset)
{
        unsigned short tmpValue = htons(value);
        memcpy(pCur + offset, &tmpValue, sizeof(value));
        return sizeof(value);
}

unsigned int CComFun::msg_putLong(char* pCur, unsigned long value, int offset)
{
        unsigned long tmpValue = htonl(value);
        memcpy(pCur + offset, &tmpValue, sizeof(value));
        return sizeof(value);
}

unsigned int CComFun::msg_putInt(char* pCur, int value, int offset)
{
        int tmpValue = htonl(value);
        memcpy(pCur + offset, &tmpValue, sizeof(value));
        return sizeof(value);
}

unsigned int CComFun::msg_putString(char* pCur, const char* value, int iValueLen, int offset)
{
        if ( 0 == iValueLen)
        {
                return 0;
        }
        else if (0 > iValueLen)
        {
                return 0;
        }
        memcpy(pCur + offset, value, iValueLen);
        return iValueLen;
}

unsigned int CComFun::msg_getChar(const char* pCur, int offset, char& value)
{
        memcpy(&value, pCur + offset, sizeof(value));
        return sizeof(value);
}

unsigned int CComFun::msg_getShort(const char* pCur, int offset, unsigned short& value)
{
        memcpy(&value, pCur + offset, sizeof(value));
        value = ntohs(value);
        return sizeof(value);
}

unsigned int CComFun::msg_getLong(const char* pCur, int offset, unsigned long& value)
{
        memcpy(&value, pCur + offset, sizeof(value));
        value = ntohl(value);
        return sizeof(value);
}

unsigned int CComFun::msg_getInt(const char* pCur, int offset,  int& value)
{
        memcpy(&value, pCur + offset, sizeof(value));
        value = ntohl(value);
        return sizeof(value);
}

unsigned int CComFun::msg_getString(const char* pCur, int offset, string& value, int iValueLen)
{
        if (0 == iValueLen)
        {
                value = "";  //��
                return 0;  //����Ϊ0
        }
        else if (0 > iValueLen)
        {
                value = "";
                return 0;
        }

        char* pValue = new char[iValueLen + 1];
        memset(pValue, 0, iValueLen + 1);
        memcpy(pValue, pCur + offset, iValueLen);
        value = pValue;
        delete[] pValue;  //ɾ������Ŀռ�
        return iValueLen;
}

//��ȡĿ¼�µ������ļ�
int CComFun::readWholeFileContent(const string& sFileName, string& sContent)
{
        struct stat stStat;
        if(stat(sFileName.c_str(), &stStat) < 0 || stStat.st_size <= 0)
        {
                return -1;
        }
        FILE *fp = fopen(sFileName.c_str(), "r");
        if (NULL == fp)
        {
                return -2;
        }
        char *p = new char[stStat.st_size + 1];
        p[stStat.st_size] = '\0';
        if (1 != fread(p, stStat.st_size, 1, fp))
        {
                fclose(fp);
                delete[] p;
                return -3;
        }
        fclose(fp);
        sContent = p;
        delete[] p;
        return 0;
}

//�����������˵���ļ��ı��ݺͻָ�Ӧ�������ڲ���
//�������ڴ�����ļ������߼����ⲿҲ���ڣ�����������Ʒ����ⲿ����
int CComFun::writeWholeFileContent(const string& sFileName, const string& sContent)
{
        FILE *fp = fopen(sFileName.c_str(), "w");
        if (NULL == fp)
        {
                return -2;
        }
        if (sContent.size() == 0)
        {
                fclose(fp); //ֱ�ӷ��أ�������ֱ��Ч����������ļ�
                return 1;
        }
        if ( 1 != fwrite(sContent.c_str(), sContent.size(), 1, fp))
        {
                fclose(fp);
                return -3; 
        }
        
        fclose(fp);
        return 0;
}

