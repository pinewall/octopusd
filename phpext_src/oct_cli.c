#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>

#include "oct_prot.h"
#include "oct_cli.h"

void set_server(const char* ip, short port)
{
        struct hostent* pstHost;

        memset(&server_addr_, 0 ,sizeof(server_addr_));

        if (inet_aton(ip, &server_addr_) == 0) {
                if ((pstHost = gethostbyname(ip)) != NULL) {
                        server_addr_ = *(struct in_addr*)pstHost->h_addr_list[0];
                }
        }

        server_port_ = htons(port);
}

int connect_tm(int* sock_fd, int timeout, char* error)
{
        int fd_flag, ret;
        struct sockaddr_in server_socket;
        fd_set fds;
        int fd_num;
        struct timeval tv;
        int sock_err, sock_err_len;

        *sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (*sock_fd == -1) {
                strcpy(error, strerror(errno));
                return -1;
        }

        // set to non-block
        fd_flag = fcntl(*sock_fd, F_GETFL);
        if (fd_flag == -1) {
                strcpy(error, strerror(errno));
                return -1;
        }
        fd_flag |= O_NONBLOCK;
        if (fcntl(*sock_fd, F_SETFL, fd_flag) == -1) {
                strcpy(error, strerror(errno));
                return -1;
        }

        // construct sockaddr
        memset(&server_socket, 0, sizeof(server_socket));
        server_socket.sin_family = AF_INET;
        server_socket.sin_port = server_port_;
        server_socket.sin_addr = server_addr_;

        // connect
        ret = connect(*sock_fd,  (struct sockaddr *)&server_socket, sizeof(server_socket));
        if (ret == -1 && errno != EINPROGRESS && errno != EWOULDBLOCK) {
                strcpy(error, strerror(errno));
                return -1;
        }

        FD_ZERO(&fds);
        FD_SET(*sock_fd, &fds);

        fd_num = *sock_fd + 1;

        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        ret = select(fd_num, NULL, &fds, NULL, &tv);
        if (ret == -1) {
                strcpy(error, strerror(errno));
                return -1;
        }
        else if (ret == 0) {
                errno = ETIME;
                strcpy(error, strerror(errno));
                return -1;
        }

        sock_err_len = sizeof(sock_err);
        if (getsockopt(*sock_fd, SOL_SOCKET, SO_ERROR, (char*)&sock_err, (socklen_t*)&sock_err_len) == -1) {
                strcpy(error, strerror(errno));
                return -1;
        }

        if (sock_err != 0) {
                errno = sock_err;
                strcpy(error, strerror(errno));
                return -1;
        }

        return 0;
}

int send_n(int sock_fd, const char* buf, int len, int timeout, char* error)
{
        int fd_num, pos;
        fd_set fds;
        struct timeval tv;

        FD_ZERO(&fds);
        FD_SET(sock_fd, &fds);

        fd_num = sock_fd + 1;

        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        pos = 0;
        while (pos < len) {
                int ret = select(fd_num, NULL, &fds, NULL, &tv);
                if (ret == -1) {
                        strcpy(error, strerror(errno));
                        return -1;
                }
                else if (ret == 0) {
                        errno = ETIME;
                        strcpy(error, strerror(errno));
                        return -1;
                }

                ret = send(sock_fd, buf + pos, len - pos, 0);
                if (ret == -1) {
                        if (errno != EINPROGRESS && errno != EWOULDBLOCK) {
                                strcpy(error, strerror(errno));
                                return -1;
                        }
                        else {
                                continue;
                        }
                }
                pos += ret;
        }

        return len;
}

int recv_n(int sock_fd, char* buf, int len, int timeout, char* error)
{
        int fd_num, pos;
        fd_set fds;
        struct timeval tv;

        FD_ZERO(&fds);
        FD_SET(sock_fd, &fds);

        fd_num = sock_fd + 1;

        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        pos = 0;
        while (pos < len) {
                int ret = select(fd_num, &fds, NULL, NULL, &tv);
                if (ret == -1) {
                        strcpy(error, strerror(errno));
                        return -1;
                }
                else if (ret == 0) {
                        errno = ETIME;
                        strcpy(error, strerror(errno));
                        return -1;
                }

                ret = recv(sock_fd, buf + pos, len - pos, 0);
                if (ret == 0) {
                        strcpy(error, "connection closed by peer");
                        return -1;
                }
                else if (ret == -1) {
                        if (errno != EINPROGRESS && errno != EWOULDBLOCK) {
                                strcpy(error, strerror(errno));
                                return -1;
                        }
                        else {
                                continue;
                        }
                }
                pos += ret;
        }

        return len;
}

void oct_init_(const char* sIP, int iPort)
{
        sSiteName[0] = '\0';
        sRemotePathName[0] = '\0';
        sLocalPathName[0]= '\0';
        sSendTime[0] = '\0';

        set_server(sIP, iPort);
}

void oct_set_upload_(int cType, const char* siteName, const char* remotePathName, const char* localPathName)
{
        eProtocolType = ProtocolUploadFile;
        eFileType = cType;
        strncpy(sSiteName, siteName, LenOfSiteName);
        strncpy(sRemotePathName, remotePathName, LenOfPathFileName);
        strncpy(sLocalPathName, localPathName, LenOfPathFileName);
        strcpy(sSendTime, oct_get_time_());
        return;
}

//有返回值，采用胖客户端的思想，减少服务器的压力和出错概率
int oct_set_upload_pack_(int cType, const char* siteName, const char* remotePathName, const char* localPathName)
{
	//@2
        char sCmd[1024 + 1]={0}; //需要执行的系统命令
        int  uType = 0;
        int iKey =0;
        char sTargzFileName[LenOfPathFileName + 1]={0};
        char sDoLocalPath[LenOfPathFileName+ 1]={0}, sDoFileName[LenOfPathFileName + 1]={0};
        char sPath[LenOfPathFileName+1]={0}, sFileName[LenOfPathFileName + 1]={0};
        char sCurDir[512];
        int iRet = 0;
        
        eProtocolType = ProtocolPackFile;
        eFileType = cType;
        strncpy(sSiteName, siteName, LenOfSiteName);
        strcpy(sSendTime, oct_get_time_());
        
        //[2.1]分析本地文件
        sCmd[0] = '\0';

        uType = getTypeOfUploadPackFile((char *)localPathName);
        if (FILE_DIR == uType)
        {
                //保存目录
                strcpy(sDoLocalPath , localPathName);  // (1)
                //生成临时的,tar.gz的文件名称(oct_ftok($srcfilename,0).tar.gz)
                iKey = ftok(localPathName, 0); //借助这个,生成key，来确认相同的文件
                snprintf(sTargzFileName, LenOfPathFileName, "oct_%d.tar.gz",  iKey);                
                strcpy(sDoFileName , sTargzFileName);  // (2)
                //拼装系统调用
                snprintf(sCmd,  1024, "/bin/tar zcf %s *", sTargzFileName);  // (3)
        }
        else if (FILE_NORMAL == uType)
        {
                //分拆出目录，和具体的文件名称
                divPathFile((char *)localPathName, sPath, sFileName);

                strcpy(sDoLocalPath, sPath); // (1)
                //生成临时的,tar.gz的文件名称(oct_ftok($srcfilename,0).tar.gz)
                iKey = ftok(localPathName, 0); //借助这个,生成key，来确认相同的文件
                snprintf(sTargzFileName, LenOfPathFileName, "oct_%d.tar.gz",  iKey);
                strcpy(sDoFileName , sTargzFileName);  // (2)
                //拼装系统调用
                sprintf(sCmd, "/bin/tar zcf %s %s", sTargzFileName, sFileName);  // (3)
        }
        else if (uType == FILE_TARGZ || uType == FILE_ZIP) //这2种指定文件名称的方式，本地加压需要外部自己保证，暂定内部不予处理，以免造成浪费
        {
                //分拆出目录，和具体的文件名称
                divPathFile((char *)localPathName, sDoLocalPath, sDoFileName); // (1) (2) no(3)
        }
        else  //出错了 inlcude FILE_ERROR
        {
                //MAGIC_LOG(("sLocalPathName(%s) invalid.", sLocalPathName));
                return -100;
        }

        //[1.1](此时已经可以获得本地和远程文件名称了)根据文件名称，拼装本地文件全路径和远端文件的全路径
        //本地
        if (sDoLocalPath[strlen(sDoLocalPath) - 1] == '/')
        {
                snprintf((char *)sLocalPathName, LenOfPathFileName, "%s%s",  sDoLocalPath, sDoFileName);
        }
        else
        {
                snprintf((char *)sLocalPathName, LenOfPathFileName, "%s/%s",  sDoLocalPath, sDoFileName);
        }
        //远端
        if (remotePathName[strlen(remotePathName) - 1] == '/')
        {
                snprintf((char *)sRemotePathName, LenOfPathFileName, "%s%s",  remotePathName, sDoFileName);
        }
        else
        {
                snprintf((char *)sRemotePathName, LenOfPathFileName, "%s/%s", remotePathName, sDoFileName);
        }        

        //[2.2]处理，切换到对应的目录下,进行文件打包，然后在返回原来的目录

        if (NULL == getcwd(sCurDir, 512))  //当前保存
        {
                 //MAGIC_LOG(("getcwd failed."));
                 return -100;
        }
        if (0 != chdir(sDoLocalPath))  //切换
        {
                //MAGIC_LOG(("chdir(%s) failed.", sDoLocalPath.c_str()));
                return -100;
        }
        if (strlen(sCmd) != 0)  //需要执行 (自己打包的2种情况, 不包括已经存在的文件)
        {
                //已经在当前目录下,判断如果存在那个.tar.gz文件，必须删除，如果一个包中，存在一个完全相同的文件，那这样会存在严重的问题
                if (0 == access(sLocalPathName,  F_OK)) //文件存在时
                 {
                          iRet = remove(sLocalPathName); // delete
                          if (0 != iRet)
                          {
                               chdir(sCurDir);
                               //MAGIC_LOG(("file(%s) exist, del it failed.", m_sLocalPathName));
                               return -100;
                          }
                 }// 不存在的话,可以用来打包了               
                
                //拼装系统调用
                iRet = system(sCmd);
                if (-1 == iRet)
                {
                       chdir(sCurDir);
                       //MAGIC_LOG(("system(%s) failed.", sCmd));
                       return -100;
                }
        }
        chdir(sCurDir);  //返回目录
        
        //return 0;
        //传出文件Key，使调用该API之后 可以删除文件tar包（hobo版）
        return iKey;
        

        
}

void oct_set_delete_(const char* siteName, const char* remotePathName)
{
        eProtocolType = ProtocolDelFile;
        strncpy(sSiteName, siteName, LenOfSiteName);
        strncpy(sRemotePathName, remotePathName, LenOfPathFileName);
        strcpy(sSendTime, oct_get_time_());
        return;
}

void oct_set_mv_(const char* destSiteName, const char* destPathName, const char* srcSiteName, const char* srcPathName)
{
        eProtocolType = ProtocolMvFile;
        strncpy(sSiteName, destSiteName, LenOfSiteName);
        strncpy(sRemotePathName, destPathName, LenOfPathFileName);
        sprintf(sLocalPathName, "%s:%s", srcSiteName, srcPathName);
        strcpy(sSendTime, oct_get_time_());
        return;
}

void oct_set_cp_(const char* destSiteName, const char* destPathName, const char* srcSiteName, const char* srcPathName)
{
        eProtocolType = ProtocolCpFile;
        strncpy(sSiteName, destSiteName, LenOfSiteName);
        strncpy(sRemotePathName, destPathName, LenOfPathFileName);
        sprintf(sLocalPathName, "%s:%s",srcSiteName, srcPathName);
        strcpy(sSendTime, oct_get_time_());
        return;
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

int oct_execute_()
{
        int sock_fd;

        //消息缓冲区和文件暂存缓冲区
        char sMsgBuff[MsgBufferSize + 1];
        char sFileBuff[WriteToFileSize + 1];
        char *p;
        int iCurSendMsgLen;
        long iFileSize;
        int iTmpBodyLen;
        int ix = 0;
        int iReadLen = 0;
        int iSendLen = 0;
        int iHadSendLen = 0;

        //做一下有效性检查，防止某个属性没有设置
        if (0 == strlen(sSiteName)) {
                sprintf(lastError, "Server or File info is invalid.");
                return -1;
        }

        //连接到服务器端
        sock_fd = -1;
        if (connect_tm(&sock_fd, 50, lastError) != 0) {
                close(sock_fd);
                return -2;
        }

        //发送消息头信息
        sMsgBuff[0] = Version1;
        sMsgBuff[1] = eProtocolType;

        if (send_n(sock_fd, sMsgBuff, 2, 10, lastError) != 2) {
                close(sock_fd);
                return -3;
        }

        switch (eProtocolType) {
        case ProtocolUploadFile:
        case ProtocolPackFile:
                {
                        //根据协议拼装消息(公共)
                        iCurSendMsgLen = 0;
                        p = sMsgBuff;

                        strncpy(p, sSiteName, LenOfSiteName); 
                        p += LenOfSiteName; 
                        iCurSendMsgLen += LenOfSiteName;

                        strncpy(p, sRemotePathName, LenOfPathFileName); 
                        p += LenOfPathFileName; 
                        iCurSendMsgLen += LenOfPathFileName;

                        strncpy(p, sSendTime, LenOfMsgTime); 
                        p += LenOfMsgTime;
                        iCurSendMsgLen += LenOfMsgTime;

                        *p = (char)eFileType; 
                        p += 1;
                        iCurSendMsgLen += 1;

                        p += 3; //预留字段
                        iCurSendMsgLen += 3;

                        if (FileName == eFileType) {
                                //文件名长度和内容       
                                int iTmpBodyLen = htonl(strlen(sLocalPathName));
                                memcpy(p, (const char*)&iTmpBodyLen, 4);
                                p += 4;
                                iCurSendMsgLen += 4;

                                strncpy(p, sLocalPathName, strlen(sLocalPathName));
                                p += strlen(sLocalPathName);
                                iCurSendMsgLen += strlen(sLocalPathName);

                                //发送消息
                                if (send_n(sock_fd, sMsgBuff, iCurSendMsgLen, 10, lastError) != iCurSendMsgLen) {
                                        close(sock_fd);
                                        return -4;
                                }
                        } //else FileData
                        else
                        {
                                //生成保存的文件名称和路径(前面已经判断过了，站点到这儿已经存在)
                                FILE *fp = fopen(sLocalPathName, "r");
                                if (NULL == fp) {
                                        close(sock_fd);
                                        sprintf(lastError, "Open %s failed.", sLocalPathName);
                                        return -5;
                                }

                                //计算一下文件的长度
                                fseek(fp,0,SEEK_END); 
                                iFileSize = ftell(fp);
                                fseek(fp,0,SEEK_SET);

                                iTmpBodyLen = htonl(iFileSize);
                                memcpy(p, (const char*)&iTmpBodyLen, 4);
                                p += 4;
                                iCurSendMsgLen += 4;

                                //发送消息
                                if (send_n(sock_fd, sMsgBuff, iCurSendMsgLen, 10, lastError) != iCurSendMsgLen) {
                                        close(sock_fd);
                                        return -6;
                                }

                                //开始发送文件流信息
                                while(!feof(fp)) {
                                        //注意，即使读不到要读的长度，也会返回正确的读到的长度
                                        //memset(sFileBuff, 0, WriteToFileSize + 1);
                                        iReadLen = fread(sFileBuff, 1, WriteToFileSize, fp);   
                                        iHadSendLen = 0;
                                        while(iReadLen > 0)
                                        {
                                                iSendLen = (iReadLen >= MsgBufferSize)?MsgBufferSize:iReadLen;
                                                memcpy(sMsgBuff, sFileBuff+ iHadSendLen, iSendLen);
                                                if (send_n(sock_fd, sMsgBuff, iSendLen, 30, lastError) != iSendLen) {
                                                        close(sock_fd);
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
                        iCurSendMsgLen = 0;
                        p = sMsgBuff;

                        strncpy(p, sSiteName, LenOfSiteName); 
                        p += LenOfSiteName; 
                        iCurSendMsgLen += LenOfSiteName;

                        strncpy(p, sRemotePathName, LenOfPathFileName); 
                        p += LenOfPathFileName; 
                        iCurSendMsgLen += LenOfPathFileName;

                        strncpy(p, sSendTime, LenOfMsgTime); 
                        p += LenOfMsgTime;
                        iCurSendMsgLen += LenOfMsgTime;

                        //发送消息
                        if (send_n(sock_fd, sMsgBuff, iCurSendMsgLen, 10, lastError) != iCurSendMsgLen) {
                                close(sock_fd);
                                return -8;
                        }
                        break;
                }
        case ProtocolMvFile:
        case ProtocolCpFile:
                {
                        //根据协议拼装消息
                        iCurSendMsgLen = 0;
                        p = sMsgBuff;

                        strncpy(p, sSiteName, LenOfSiteName); 
                        p += LenOfSiteName; 
                        iCurSendMsgLen += LenOfSiteName;

                        strncpy(p, sRemotePathName, LenOfPathFileName); 
                        p += LenOfPathFileName; 
                        iCurSendMsgLen += LenOfPathFileName;

                        strncpy(p, sLocalPathName, LenOfPathFileName); 
                        p += LenOfPathFileName; 
                        iCurSendMsgLen += LenOfPathFileName;

                        strncpy(p, sSendTime, LenOfMsgTime); 
                        p += LenOfMsgTime;
                        iCurSendMsgLen += LenOfMsgTime;

                        //发送消息
                        if (send_n(sock_fd, sMsgBuff, iCurSendMsgLen, 10, lastError) != iCurSendMsgLen) {
                                close(sock_fd);
                                return -8;
                        }
                        break;
                }
        default:
                {
                        close(sock_fd);
                        sprintf(lastError, "Unkown msg type.");
                        return -9;
                }
        }       

        //接收响应
        if (recv_n(sock_fd, sMsgBuff, 1, 30, lastError) != 1) {
                close(sock_fd);
                return -10;
        }     

        close(sock_fd);

        if (sMsgBuff[0] != 0x01) {
                sprintf(lastError, "Recived error response code(%x).", sMsgBuff[0]);
                return -11;
        }

        return 0;
}

int oct_upload_file_(const char* sIP, int iPort, int cType, const char* siteName, const char* remotePathName, const char* localPathName)
{
        oct_init_(sIP, iPort);
        oct_set_upload_(cType, siteName, remotePathName, localPathName);
        return oct_execute_();

}

int oct_upload_pack_file_(const char* sIP, int iPort, int cType, const char* siteName, const char* remotePathName, const char* localPathName)
{
	//@1 发送打包文件
	    int iRtn=0, iKey=0;
        oct_init_(sIP, iPort);
        iKey = oct_set_upload_pack_(cType, siteName, remotePathName, localPathName);
        if (-100 == iKey)
        {
            return -100;
        }
        
        iRtn = oct_execute_();
        if ( iRtn == 0 )
        {
            return iKey;
        }
        else
        {
            return iRtn;
        }
}

int oct_delete_file_(const char* sIP, int iPort,const char* siteName, const char* remotePathName)
{
        oct_init_(sIP, iPort);
        oct_set_delete_(siteName, remotePathName);
        return oct_execute_();
}

int oct_move_file_(const char* sIP, int iPort, const char* destSiteName, const char* destPathName, const char* srcSiteName, const char* srcPathName)
{
        oct_init_(sIP, iPort);
        oct_set_mv_(destSiteName, destPathName,srcSiteName,srcPathName);
        return oct_execute_();
}

int oct_copy_file_(const char* sIP, int iPort, const char* destSiteName, const char* destPathName, const char* srcSiteName, const char* srcPathName)
{
        oct_init_(sIP, iPort);
        oct_set_cp_(destSiteName, destPathName,srcSiteName,srcPathName);
        return oct_execute_();
}

const char* oct_get_error_()
{
        return lastError;
}

const char* oct_get_time_()
{
        struct timeval now;
        struct tm *curr;
        char sTmp[10];
        time_t iCurTime;

        gettimeofday(&now, 0);

        iCurTime = now.tv_sec;

        curr = localtime(&iCurTime);

        if (curr->tm_year > 50) {
                sprintf(sTmp, "%06ld", now.tv_usec);
                sTmp[5] = '\0';
                sprintf(s, "%04d%02d%02d%02d%02d%02d%s",
                        curr->tm_year+1900, curr->tm_mon+1, curr->tm_mday,
                        curr->tm_hour, curr->tm_min, curr->tm_sec, sTmp);
        }
        else {
                sprintf(sTmp, "%06ld", now.tv_usec);
                sTmp[5] = '\0';
                sprintf(s, "%04d%02d%02d%02d%02d%02d%s",
                        curr->tm_year+2000, curr->tm_mon+1, curr->tm_mday,
                        curr->tm_hour, curr->tm_min, curr->tm_sec, sTmp);
        }

        return s;
}

//获得文件名中的后缀(规则就是取第一个.后面的所有东西)
int getTypeOfUploadPackFile(char * sFile)
{
        struct stat st1;
        char *pos = NULL;
        char sTmp[32]={0};
        
        //空文件
        if (sFile == NULL || strlen(sFile) == 0)
        {
                return FILE_ERROR;
        }

        //目录
        if (stat(sFile, &st1) != 0)
        {
                return FILE_ERROR;
        }
        if(S_ISDIR(st1.st_mode))  //目录
        {
                return  FILE_DIR;
        }

        pos = strchr(sFile,'.');
        //一般文件
        if(NULL == pos) 
        {
                return FILE_NORMAL;
        }
        
        strcpy( sTmp, pos+1 );
        //gz
        if (strcmp(sTmp,"tar.gz")==0)
        {
                return FILE_TARGZ;
        }
        //zip
        else if (strcmp(sTmp,"zip")==0)
        {
                return FILE_ZIP;
        }
        else
        {
                return FILE_NORMAL;
        }

}

void divPathFile(char *sPathFile, char *sPath, char *sFileName)
{
        char *lastPos = NULL;
        int iPos = 0;
        
        //是否合法
        if (sPathFile== NULL || 0 == strlen(sPathFile))
        {
                return;
        }

        //查找最后一个 / ，认为前面的就是路径，后面的就是文件名称
        lastPos = strrchr(sPathFile,'/');
        if (NULL == lastPos)
        {
                return;
        }

        //取得文件名称
        iPos = lastPos - sPathFile; 
        memcpy( sPath , sPathFile, iPos);
        memcpy( sFileName ,sPathFile+iPos+1, strlen(sPathFile) - iPos );
        return;
}

