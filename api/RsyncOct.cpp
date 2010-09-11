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

//有返回值，采用胖客户端的思想，减少服务器的压力和出错概率
int CRsyncOct::setUploadPackInfo(UploadFileType cType, const char* sSiteName, const char* sRemotePath, const char* sLocalPathName)
{
        //[1.0] 固定信息
        m_eProtocolType = ProtocolPackFile;
        m_eFileType = cType;
        strncpy(m_sSiteName, sSiteName, LenOfSiteName);
        strcpy(m_sSendTime, getMsgTime().c_str());
        
        //[2.1]分析本地文件
        char sCmd[1024 + 1]; //需要执行的系统命令
        sCmd[0] = '\0';
        string sDoLocalPath, sDoFileName;

        UploadPackFileType uType = getTypeOfUploadPackFile(sLocalPathName);
        if (FILE_DIR == uType)
        {
                //保存目录
                sDoLocalPath = sLocalPathName;  // (1)
                //生成临时的,tar.gz的文件名称(oct_ftok($srcfilename,0).tar.gz)
                char sTargzFileName[LenOfPathFileName + 1];
                int iKey = ftok(sLocalPathName, 0); //借助这个,生成key，来确认相同的文件
                snprintf(sTargzFileName, LenOfPathFileName, "oct_%d.tar.gz",  iKey);                
                sDoFileName = sTargzFileName;  // (2)
                //拼装系统调用
                snprintf(sCmd,  1024, "/bin/tar zcf %s *", sTargzFileName);  // (3)
        }
        else if (FILE_NORMAL == uType)
        {
                //分拆出目录，和具体的文件名称
                string sPath, sFileName;
                divPathFile(sLocalPathName, sPath, sFileName);

                sDoLocalPath = sPath; // (1)

                //生成临时的,tar.gz的文件名称(oct_ftok($srcfilename,0).tar.gz)
                char sTargzFileName[LenOfPathFileName + 1];
                int iKey = ftok(sLocalPathName, 0); //借助这个,生成key，来确认相同的文件
                snprintf(sTargzFileName, LenOfPathFileName, "oct_%d.tar.gz",  iKey);
                sDoFileName = sTargzFileName;  // (2)
                //拼装系统调用
                sprintf(sCmd, "/bin/tar zcf %s %s", sTargzFileName, sFileName.c_str());  // (3)
        }
        else if (uType == FILE_TARGZ || uType == FILE_ZIP) //这2种指定文件名称的方式，本地加压需要外部自己保证，暂定内部不予处理，以免造成浪费
        {
                //分拆出目录，和具体的文件名称
                divPathFile(sLocalPathName, sDoLocalPath, sDoFileName); // (1) (2) no(3)
        }
        else  //出错了 inlcude FILE_ERROR
        {
                MAGIC_LOG(("sLocalPathName(%s) invalid.", sLocalPathName));
                return -100;
        }

        //[1.1](此时已经可以获得本地和远程文件名称了)根据文件名称，拼装本地文件全路径和远端文件的全路径
        //本地
        if (sDoLocalPath[(sDoLocalPath).size() - 1] == '/')
        {
                snprintf(m_sLocalPathName, LenOfPathFileName, "%s%s",  sDoLocalPath.c_str(), sDoFileName.c_str());
        }
        else
        {
                snprintf(m_sLocalPathName, LenOfPathFileName, "%s/%s",  sDoLocalPath.c_str(), sDoFileName.c_str());
        }
        //远端
        if (sRemotePath[strlen(sRemotePath) - 1] == '/')
        {
                snprintf(m_sRemotePathName, LenOfPathFileName, "%s%s",  sRemotePath, sDoFileName.c_str());
        }
        else
        {
                snprintf(m_sRemotePathName, LenOfPathFileName, "%s/%s", sRemotePath, sDoFileName.c_str());
        }        

        //[2.2]处理，切换到对应的目录下,进行文件打包，然后在返回原来的目录
        char sCurDir[512];
        if (NULL == getcwd(sCurDir, 512))  //当前保存
        {
                 MAGIC_LOG(("getcwd failed."));
                 return -100;
        }
        if (0 != chdir(sDoLocalPath.c_str()))  //切换
        {
                MAGIC_LOG(("chdir(%s) failed.", sDoLocalPath.c_str()));
                return -100;
        }
        if (strlen(sCmd) != 0)  //需要执行 (自己打包的2种情况, 不包括已经存在的文件)
        {
                //已经在当前目录下,判断如果存在那个.tar.gz文件，必须删除，如果一个包中，存在一个完全相同的文件，那这样会存在严重的问题
                if (0 == access(m_sLocalPathName,  F_OK)) //文件存在时
                 {
                          int iRet = remove(m_sLocalPathName); // delete
                          if (0 != iRet)
                          {
                               chdir(sCurDir);
                               MAGIC_LOG(("file(%s) exist, del it failed.", m_sLocalPathName));
                               return -100;
                          }
                 }// 不存在的话,可以用来打包了               
                
                //拼装系统调用
                int iRet = system(sCmd);
                if (-1 == iRet)
                {
                       chdir(sCurDir);
                       MAGIC_LOG(("system(%s) failed.", sCmd));
                       return -100;
                }
        }
        chdir(sCurDir);  //返回目录
             
        return 0;
}


/*===================上传文件和删除文件协议解析 BEGIN=====================
[1] 请求消息
    [1.0]公共部分
        --协议版本号   1byte 当前0x01
        --命令字       1byte 当前0x01上传文件消息 删除文件消息
    [1.1]上传文件消息
        --Fsite_name    String 32  站点英文简称
        --Fserver_path  String 256 服务器上文件相对路径
        --Ftime         String 20  发送请求时间
        --Fdata_type    Char   1   发送数据类型，1：文件，2：数据流
        --Freserve      Char   3   保留
        --Fdata_length  Int    4   数据长度
        --Fdata         Byte   变长  数据内容，如果数据类型为文件，则为本地文件的绝对路径，如果为数据流，则为文件的内容。
        
    [1.2]删除文件消息
        --Fsite_name    String 32   站点英文简称
        --Fserver_path  String 256  服务器上文件相对路径
        --Ftime         String 20   发送请求时间
    
 [2]相应消息
       --Code           Char 1      0x01：成功，其他：失败

===================上传文件和删除文件协议解析 END=====================*/

int CRsyncOct::execute()
{
        //做一下有效性检查，防止某个属性没有设置
        if (0 == m_sServerIP.size() || 0 == strlen(m_sSiteName))
        {
                MAGIC_LOG(("Server or File info is invalid."));
                return -1;
        }
        
        //连接到服务器端
        Magic_TcpConnector insServerConn; 
        Magic_SocketAddr insServerAddr(m_sServerIP, m_iServerPort);                     
        if ( 0 != insServerConn.Connect_tm(insServerAddr, 50))
        {
                MAGIC_LOG(("Connect to server(%s, %d) failed.",m_sServerIP.c_str(), m_iServerPort));
                return -2;
        }
        //消息缓冲区和文件暂存缓冲区
        char sMsgBuff[MsgBufferSize + 1];
        char sFileBuff[WriteToFileSize + 1];
        char *p;

        //发送消息头信息
        sMsgBuff[0] = Version1;
        sMsgBuff[1] = m_eProtocolType;
        
        if (2 != insServerConn.Write_n(sMsgBuff, 2, 10))  //  send 1.1
        {
                 MAGIC_LOG(("Write msg(%x) head data to socket failed.", m_eProtocolType));
                 insServerConn.Close();
                return -3;
        }

        //根据不同的协议，分别进行不同的处理        
        switch(m_eProtocolType)
        {        
        case ProtocolUploadFile:
        case ProtocolPackFile:  // 2007-01-10
        {
                //根据协议拼装消息(公共)
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
                
                p += 3; //预留字段
                iCurSendMsgLen += 3;
                        
                if (FileName == m_eFileType)
                {
                        //文件名长度和内容       
                        int iTmpBodyLen = htonl(strlen(m_sLocalPathName));
                        memcpy(p, (const char*)&iTmpBodyLen, 4);
                        p += 4;
                        iCurSendMsgLen += 4;
                        
                        strncpy(p, m_sLocalPathName, strlen(m_sLocalPathName));
                        p += strlen(m_sLocalPathName);
                        iCurSendMsgLen += strlen(m_sLocalPathName);
                        
                        //发送消息
                        if (insServerConn.Write_n(sMsgBuff, iCurSendMsgLen, 10) != iCurSendMsgLen) 
                        {
                                insServerConn.Close();
                                MAGIC_LOG(("Send upload msg(%s, %s) failed.",m_sSiteName, m_sRemotePathName));
                                return -4;
                        }
                } //else FileData
                else
                {
                        //生成保存的文件名称和路径(前面已经判断过了，站点到这儿已经存在)
                        FILE *fp = fopen(m_sLocalPathName, "r");
                        if (NULL == fp)
                        {
                                insServerConn.Close();
                                MAGIC_LOG(("Open %s failed.", m_sLocalPathName));
                                return -5;
                        }
                        
                        //计算一下文件的长度
                        fseek(fp,0,SEEK_END); 
                        long iFileSize = ftell(fp);
                        fseek(fp,0,SEEK_SET);
                        
                        int iTmpBodyLen = htonl(iFileSize);
                        memcpy(p, (const char*)&iTmpBodyLen, 4);
                        p += 4;
                        iCurSendMsgLen += 4;
                        
                        //发送消息
                        if (insServerConn.Write_n(sMsgBuff, iCurSendMsgLen, 10) != iCurSendMsgLen) 
                        {
                                insServerConn.Close();
                                MAGIC_LOG(("Send upload msg(%s, %s) failed.",m_sSiteName, m_sRemotePathName));
                                return -6;
                        }

                        //开始发送文件流信息
                        int iReadLen = 0;
                        int iSendLen = 0;
                        int iHadSendLen = 0;
                        while(!feof(fp))
                        {
                                int ix = 0;
                                //注意，即使读不到要读的长度，也会返回正确的读到的长度
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
                //根据协议拼装消息
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
                
                //发送消息
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
                //根据协议拼装消息
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
                
                //发送消息
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
        
        //接收响应
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

//获得文件名中的后缀(规则就是取第一个.后面的所有东西)
CRsyncOct::UploadPackFileType CRsyncOct::getTypeOfUploadPackFile(string sFile)
{
        //空文件
        if (sFile.size() == 0)
        {
                return FILE_ERROR;
        }

        //目录
        struct stat st1;
        if (stat(sFile.c_str(), &st1) != 0)
        {
                return FILE_ERROR;
        }
        if(S_ISDIR(st1.st_mode))  //目录
        {
                return  FILE_DIR;
        }

        string::size_type pos = sFile.find(".");
        //一般文件
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
        //是否合法
        if (0 == sPathFile.size())
        {
                return;
        }

        //查找最后一个 / ，认为前面的就是路径，后面的就是文件名称
        string::size_type lastPos = sPathFile.find_last_of("/");
        if (string::npos == lastPos)
        {
                return;
        }

        //取得文件名称
        sPath = sPathFile.substr(0, lastPos);
        sFileName = sPathFile.substr(lastPos+1);
        return;
}


