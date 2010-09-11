//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: RsyncOct.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/11/10 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "RsyncOct.h"

using namespace std;
using namespace CGIMAGIC;

CRsyncOct::CRsyncOct()
{
        m_sSiteName[0] = '\0';
        m_sRemotePathName[0] = '\0';
        m_sLocalPathName[0]= '\0';
        m_sSendTime[0] = '\0';
}
CRsyncOct::~CRsyncOct()
{
}

void CRsyncOct::setServer(const string& sIP, int iPort)
{
        m_sServerIP = sIP;
        m_iServerPort = iPort;
        return;
}

void CRsyncOct::setUploadInfo(UploadFileType cType, const char* sSiteName, const char* sRemotePathName, const char* sLocalPathName)
{
        m_eProtocolType = ProtocolUploadFile;
        m_eFileType = cType;
        strncpy(m_sSiteName, sSiteName, LenOfSiteName);
        strncpy(m_sRemotePathName, sRemotePathName, LenOfPathFileName);
        strncpy(m_sLocalPathName, sLocalPathName, LenOfPathFileName);
        strcpy(m_sSendTime, getMsgTime().c_str());
        return;
}

void CRsyncOct::setDeleteInfo(const char* sSiteName, const char* sRemotePathName)
{
        m_eProtocolType = ProtocolDelFile;
        strncpy(m_sSiteName, sSiteName, LenOfSiteName);
        strncpy(m_sRemotePathName, sRemotePathName, LenOfPathFileName);
        strcpy(m_sSendTime, getMsgTime().c_str());
        return;
}

void CRsyncOct::setMvInfo(const char* sDestSiteName, const char* sDestPathName, const char* sSrcSiteName, const char* sSrcPathName)
{
        m_eProtocolType = ProtocolMvFile;
        strncpy(m_sSiteName, sDestSiteName, LenOfSiteName);
        strncpy(m_sRemotePathName, sDestPathName, LenOfPathFileName);
        sprintf(m_sLocalPathName, "%s:%s",sSrcSiteName, sSrcPathName);
        strcpy(m_sSendTime, getMsgTime().c_str());
        return;
}

void CRsyncOct::setCpInfo(const char* sDestSiteName, const char* sDestPathName, const char* sSrcSiteName, const char* sSrcPathName)
{
        m_eProtocolType = ProtocolCpFile;
        strncpy(m_sSiteName, sDestSiteName, LenOfSiteName);
        strncpy(m_sRemotePathName, sDestPathName, LenOfPathFileName);
        sprintf(m_sLocalPathName, "%s:%s",sSrcSiteName, sSrcPathName);
        strcpy(m_sSendTime, getMsgTime().c_str());
        return;
}

//�з���ֵ�������ֿͻ��˵�˼�룬���ٷ�������ѹ���ͳ������
int CRsyncOct::setUploadPackInfo(UploadFileType cType, const char* sSiteName, const char* sRemotePath, const char* sLocalPathName)
{
        //[1.0] �̶���Ϣ
        m_eProtocolType = ProtocolPackFile;
        m_eFileType = cType;
        strncpy(m_sSiteName, sSiteName, LenOfSiteName);
        strcpy(m_sSendTime, getMsgTime().c_str());
        
        //[2.1]���������ļ�
        char sCmd[1024 + 1]; //��Ҫִ�е�ϵͳ����
        sCmd[0] = '\0';
        string sDoLocalPath, sDoFileName;

        UploadPackFileType uType = getTypeOfUploadPackFile(sLocalPathName);
        if (FILE_DIR == uType)
        {
                //����Ŀ¼
                sDoLocalPath = sLocalPathName;  // (1)
                //������ʱ��,tar.gz���ļ�����(oct_ftok($srcfilename,0).tar.gz)
                char sTargzFileName[LenOfPathFileName + 1];
                int iKey = ftok(sLocalPathName, 0); //�������,����key����ȷ����ͬ���ļ�
                snprintf(sTargzFileName, LenOfPathFileName, "oct_%d.tar.gz",  iKey);                
                sDoFileName = sTargzFileName;  // (2)
                //ƴװϵͳ����
                snprintf(sCmd,  1024, "/bin/tar zcf %s *", sTargzFileName);  // (3)
        }
        else if (FILE_NORMAL == uType)
        {
                //�ֲ��Ŀ¼���;�����ļ�����
                string sPath, sFileName;
                divPathFile(sLocalPathName, sPath, sFileName);

                sDoLocalPath = sPath; // (1)

                //������ʱ��,tar.gz���ļ�����(oct_ftok($srcfilename,0).tar.gz)
                char sTargzFileName[LenOfPathFileName + 1];
                int iKey = ftok(sLocalPathName, 0); //�������,����key����ȷ����ͬ���ļ�
                snprintf(sTargzFileName, LenOfPathFileName, "oct_%d.tar.gz",  iKey);
                sDoFileName = sTargzFileName;  // (2)
                //ƴװϵͳ����
                sprintf(sCmd, "/bin/tar zcf %s %s", sTargzFileName, sFileName.c_str());  // (3)
        }
        else if (uType == FILE_TARGZ || uType == FILE_ZIP) //��2��ָ���ļ����Ƶķ�ʽ�����ؼ�ѹ��Ҫ�ⲿ�Լ���֤���ݶ��ڲ����账����������˷�
        {
                //�ֲ��Ŀ¼���;�����ļ�����
                divPathFile(sLocalPathName, sDoLocalPath, sDoFileName); // (1) (2) no(3)
        }
        else  //������ inlcude FILE_ERROR
        {
                MAGIC_LOG(("sLocalPathName(%s) invalid.", sLocalPathName));
                return -100;
        }

        //[1.1](��ʱ�Ѿ����Ի�ñ��غ�Զ���ļ�������)�����ļ����ƣ�ƴװ�����ļ�ȫ·����Զ���ļ���ȫ·��
        //����
        if (sDoLocalPath[(sDoLocalPath).size() - 1] == '/')
        {
                snprintf(m_sLocalPathName, LenOfPathFileName, "%s%s",  sDoLocalPath.c_str(), sDoFileName.c_str());
        }
        else
        {
                snprintf(m_sLocalPathName, LenOfPathFileName, "%s/%s",  sDoLocalPath.c_str(), sDoFileName.c_str());
        }
        //Զ��
        if (sRemotePath[strlen(sRemotePath) - 1] == '/')
        {
                snprintf(m_sRemotePathName, LenOfPathFileName, "%s%s",  sRemotePath, sDoFileName.c_str());
        }
        else
        {
                snprintf(m_sRemotePathName, LenOfPathFileName, "%s/%s", sRemotePath, sDoFileName.c_str());
        }        

        //[2.2]�����л�����Ӧ��Ŀ¼��,�����ļ������Ȼ���ڷ���ԭ����Ŀ¼
        char sCurDir[512];
        if (NULL == getcwd(sCurDir, 512))  //��ǰ����
        {
                 MAGIC_LOG(("getcwd failed."));
                 return -100;
        }
        if (0 != chdir(sDoLocalPath.c_str()))  //�л�
        {
                MAGIC_LOG(("chdir(%s) failed.", sDoLocalPath.c_str()));
                return -100;
        }
        if (strlen(sCmd) != 0)  //��Ҫִ�� (�Լ������2�����, �������Ѿ����ڵ��ļ�)
        {
                //�Ѿ��ڵ�ǰĿ¼��,�ж���������Ǹ�.tar.gz�ļ�������ɾ�������һ�����У�����һ����ȫ��ͬ���ļ�����������������ص�����
                if (0 == access(m_sLocalPathName,  F_OK)) //�ļ�����ʱ
                 {
                          int iRet = remove(m_sLocalPathName); // delete
                          if (0 != iRet)
                          {
                               chdir(sCurDir);
                               MAGIC_LOG(("file(%s) exist, del it failed.", m_sLocalPathName));
                               return -100;
                          }
                 }// �����ڵĻ�,�������������               
                
                //ƴװϵͳ����
                int iRet = system(sCmd);
                if (-1 == iRet)
                {
                       chdir(sCurDir);
                       MAGIC_LOG(("system(%s) failed.", sCmd));
                       return -100;
                }
        }
        chdir(sCurDir);  //����Ŀ¼
             
        return 0;
}


/*===================�ϴ��ļ���ɾ���ļ�Э����� BEGIN=====================
[1] ������Ϣ
    [1.0]��������
        --Э��汾��   1byte ��ǰ0x01
        --������       1byte ��ǰ0x01�ϴ��ļ���Ϣ ɾ���ļ���Ϣ
    [1.1]�ϴ��ļ���Ϣ
        --Fsite_name    String 32  վ��Ӣ�ļ��
        --Fserver_path  String 256 ���������ļ����·��
        --Ftime         String 20  ��������ʱ��
        --Fdata_type    Char   1   �����������ͣ�1���ļ���2��������
        --Freserve      Char   3   ����
        --Fdata_length  Int    4   ���ݳ���
        --Fdata         Byte   �䳤  �������ݣ������������Ϊ�ļ�����Ϊ�����ļ��ľ���·�������Ϊ����������Ϊ�ļ������ݡ�
        
    [1.2]ɾ���ļ���Ϣ
        --Fsite_name    String 32   վ��Ӣ�ļ��
        --Fserver_path  String 256  ���������ļ����·��
        --Ftime         String 20   ��������ʱ��
    
 [2]��Ӧ��Ϣ
       --Code           Char 1      0x01���ɹ���������ʧ��

===================�ϴ��ļ���ɾ���ļ�Э����� END=====================*/

int CRsyncOct::execute()
{
        //��һ����Ч�Լ�飬��ֹĳ������û������
        if (0 == m_sServerIP.size() || 0 == strlen(m_sSiteName))
        {
                MAGIC_LOG(("Server or File info is invalid."));
                return -1;
        }
        
        //���ӵ���������
        Magic_TcpConnector insServerConn; 
        Magic_SocketAddr insServerAddr(m_sServerIP, m_iServerPort);                     
        if ( 0 != insServerConn.Connect_tm(insServerAddr, 50))
        {
                MAGIC_LOG(("Connect to server(%s, %d) failed.",m_sServerIP.c_str(), m_iServerPort));
                return -2;
        }
        //��Ϣ���������ļ��ݴ滺����
        char sMsgBuff[MsgBufferSize + 1];
        char sFileBuff[WriteToFileSize + 1];
        char *p;

        //������Ϣͷ��Ϣ
        sMsgBuff[0] = Version1;
        sMsgBuff[1] = m_eProtocolType;
        
        if (2 != insServerConn.Write_n(sMsgBuff, 2, 10))  //  send 1.1
        {
                 MAGIC_LOG(("Write msg(%x) head data to socket failed.", m_eProtocolType));
                 insServerConn.Close();
                return -3;
        }

        //���ݲ�ͬ��Э�飬�ֱ���в�ͬ�Ĵ���        
        switch(m_eProtocolType)
        {        
        case ProtocolUploadFile:
        case ProtocolPackFile:  // 2007-01-10
        {
                //����Э��ƴװ��Ϣ(����)
                int iCurSendMsgLen = 0;
                p = sMsgBuff;
                
                strncpy(p, m_sSiteName, LenOfSiteName); 
                p += LenOfSiteName; 
                iCurSendMsgLen += LenOfSiteName;
                
                strncpy(p, m_sRemotePathName, LenOfPathFileName); 
                p += LenOfPathFileName; 
                iCurSendMsgLen += LenOfPathFileName;
                
                strncpy(p, m_sSendTime, LenOfMsgTime); 
                p += LenOfMsgTime;
                iCurSendMsgLen += LenOfMsgTime;
                
                *p = (char)m_eFileType; 
                p += 1;
                iCurSendMsgLen += 1;
                
                p += 3; //Ԥ���ֶ�
                iCurSendMsgLen += 3;
                        
                if (FileName == m_eFileType)
                {
                        //�ļ������Ⱥ�����       
                        int iTmpBodyLen = htonl(strlen(m_sLocalPathName));
                        memcpy(p, (const char*)&iTmpBodyLen, 4);
                        p += 4;
                        iCurSendMsgLen += 4;
                        
                        strncpy(p, m_sLocalPathName, strlen(m_sLocalPathName));
                        p += strlen(m_sLocalPathName);
                        iCurSendMsgLen += strlen(m_sLocalPathName);
                        
                        //������Ϣ
                        if (insServerConn.Write_n(sMsgBuff, iCurSendMsgLen, 10) != iCurSendMsgLen) 
                        {
                                insServerConn.Close();
                                MAGIC_LOG(("Send upload msg(%s, %s) failed.",m_sSiteName, m_sRemotePathName));
                                return -4;
                        }
                } //else FileData
                else
                {
                        //���ɱ�����ļ����ƺ�·��(ǰ���Ѿ��жϹ��ˣ�վ�㵽����Ѿ�����)
                        FILE *fp = fopen(m_sLocalPathName, "r");
                        if (NULL == fp)
                        {
                                insServerConn.Close();
                                MAGIC_LOG(("Open %s failed.", m_sLocalPathName));
                                return -5;
                        }
                        
                        //����һ���ļ��ĳ���
                        fseek(fp,0,SEEK_END); 
                        long iFileSize = ftell(fp);
                        fseek(fp,0,SEEK_SET);
                        
                        int iTmpBodyLen = htonl(iFileSize);
                        memcpy(p, (const char*)&iTmpBodyLen, 4);
                        p += 4;
                        iCurSendMsgLen += 4;
                        
                        //������Ϣ
                        if (insServerConn.Write_n(sMsgBuff, iCurSendMsgLen, 10) != iCurSendMsgLen) 
                        {
                                insServerConn.Close();
                                MAGIC_LOG(("Send upload msg(%s, %s) failed.",m_sSiteName, m_sRemotePathName));
                                return -6;
                        }

                        //��ʼ�����ļ�����Ϣ
                        int iReadLen = 0;
                        int iSendLen = 0;
                        int iHadSendLen = 0;
                        while(!feof(fp))
                        {
                                int ix = 0;
                                //ע�⣬��ʹ������Ҫ���ĳ��ȣ�Ҳ�᷵����ȷ�Ķ����ĳ���
                                //memset(sFileBuff, 0, WriteToFileSize + 1);
                                iReadLen = fread(sFileBuff, 1, WriteToFileSize, fp);   
                                iHadSendLen = 0;
                                while(iReadLen > 0)
                                {
                                        iSendLen = (iReadLen >= MsgBufferSize)?MsgBufferSize:iReadLen;
                                        memcpy(sMsgBuff, sFileBuff+ iHadSendLen, iSendLen);
                                        if( iSendLen !=  insServerConn.Write_n(sMsgBuff, iSendLen, 30))
                                        {
                                                insServerConn.Close();
                                                MAGIC_LOG(("Send File(%s) data block failed.", m_sLocalPathName));
                                                return -7;
                                        }                                        
                                        iHadSendLen = iHadSendLen + iSendLen;
                                        iReadLen = iReadLen - iSendLen;
                                }  
                        }
                        fclose(fp);
                }
                break;
        }
        case ProtocolDelFile:
        {
                //����Э��ƴװ��Ϣ
                int iCurSendMsgLen = 0;
                p = sMsgBuff;
                
                strncpy(p, m_sSiteName, LenOfSiteName); 
                p += LenOfSiteName; 
                iCurSendMsgLen += LenOfSiteName;
                
                strncpy(p, m_sRemotePathName, LenOfPathFileName); 
                p += LenOfPathFileName; 
                iCurSendMsgLen += LenOfPathFileName;
                
                strncpy(p, m_sSendTime, LenOfMsgTime); 
                p += LenOfMsgTime;
                iCurSendMsgLen += LenOfMsgTime;
                
                //������Ϣ
                if (insServerConn.Write_n(sMsgBuff, iCurSendMsgLen, 10) != iCurSendMsgLen) 
                {
                        insServerConn.Close();
                        MAGIC_LOG(("Send delete msg(%s, %s) failed.",m_sSiteName, m_sRemotePathName));
                        return -8;
                }
                break;
        }
        case ProtocolMvFile:
        case ProtocolCpFile:
        {
                //����Э��ƴװ��Ϣ
                int iCurSendMsgLen = 0;
                p = sMsgBuff;
                
                strncpy(p, m_sSiteName, LenOfSiteName); 
                p += LenOfSiteName; 
                iCurSendMsgLen += LenOfSiteName;
                
                strncpy(p, m_sRemotePathName, LenOfPathFileName); 
                p += LenOfPathFileName; 
                iCurSendMsgLen += LenOfPathFileName;
                
                strncpy(p, m_sLocalPathName, LenOfPathFileName); 
                p += LenOfPathFileName; 
                iCurSendMsgLen += LenOfPathFileName;
                
                strncpy(p, m_sSendTime, LenOfMsgTime); 
                p += LenOfMsgTime;
                iCurSendMsgLen += LenOfMsgTime;
                
                //������Ϣ
                if (insServerConn.Write_n(sMsgBuff, iCurSendMsgLen, 10) != iCurSendMsgLen) 
                {
                        insServerConn.Close();
                        MAGIC_LOG(("Send delete msg(%s, %s) failed.",m_sSiteName, m_sRemotePathName));
                        return -8;
                }
                break;
        }
        default:
        {
                MAGIC_LOG(("Unkown msg type."));
                insServerConn.Close();
                return -9;
        }
        }
        
        //������Ӧ
        if (insServerConn.Read_n(sMsgBuff, 1, 30) != 1) 
        {
                insServerConn.Close();
                MAGIC_LOG(("Recived response code failed."));
                return -10;
        }        
        insServerConn.Close();
        
        if (sMsgBuff[0] != 0x01)
        {
                MAGIC_LOG(("Recived error response code(%x).", sMsgBuff[0]));
                return -11;
        }
        return 0;
}

int CRsyncOct::uploadFile(const string& sIP, int iPort,UploadFileType cType, const char* sSiteName, const char* sRemotePathName, const char* sLocalPathName)
{
        setServer(sIP,iPort);
        setUploadInfo(cType, sSiteName, sRemotePathName, sLocalPathName);
        return execute();
        
}

int CRsyncOct::deleteFile(const string& sIP, int iPort,const char* sSiteName, const char* sRemotePathName)
{
        setServer(sIP,iPort);
        setDeleteInfo(sSiteName, sRemotePathName);
        return execute();
}

int CRsyncOct::mvFile(const string& sIP, int iPort, const char* sDestSiteName, const char* sDestPathName, const char* sSrcSiteName, const char* sSrcPathName)
{
        setServer(sIP,iPort);
        setMvInfo(sDestSiteName, sDestPathName,sSrcSiteName,sSrcPathName);
        return execute();
}

int CRsyncOct::cpFile(const string& sIP, int iPort, const char* sDestSiteName, const char* sDestPathName, const char* sSrcSiteName, const char* sSrcPathName)
{
        setServer(sIP,iPort);
        setCpInfo(sDestSiteName, sDestPathName,sSrcSiteName, sSrcPathName);
        return execute();
}

int CRsyncOct::uploadPackFile(const string& sIP, int iPort,UploadFileType cType, const char* sSiteName, const char* sRemotePath, const char* sLocalPathName)
{
        setServer(sIP,iPort);
        if (-100 == setUploadPackInfo(cType, sSiteName, sRemotePath, sLocalPathName))
        {
                return -100;
        }
        return execute();
}
string CRsyncOct::getMsgTime()
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

//����ļ����еĺ�׺(�������ȡ��һ��.��������ж���)
CRsyncOct::UploadPackFileType CRsyncOct::getTypeOfUploadPackFile(string sFile)
{
        //���ļ�
        if (sFile.size() == 0)
        {
                return FILE_ERROR;
        }

        //Ŀ¼
        struct stat st1;
        if (stat(sFile.c_str(), &st1) != 0)
        {
                return FILE_ERROR;
        }
        if(S_ISDIR(st1.st_mode))  //Ŀ¼
        {
                return  FILE_DIR;
        }

        string::size_type pos = sFile.find(".");
        //һ���ļ�
        if(string::npos == pos) 
        {
                return FILE_NORMAL;
        }
        
        string sTmp = sFile.substr(pos+1);
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

void CRsyncOct::divPathFile(string sPathFile, string& sPath, string& sFileName)
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


