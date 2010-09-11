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

//�з���ֵ�������ֿͻ��˵�˼�룬���ٷ�������ѹ���ͳ������
int oct_set_upload_pack_(int cType, const char* siteName, const char* remotePathName, const char* localPathName)
{
	//@2
        char sCmd[1024 + 1]={0}; //��Ҫִ�е�ϵͳ����
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
        
        //[2.1]���������ļ�
        sCmd[0] = '\0';

        uType = getTypeOfUploadPackFile((char *)localPathName);
        if (FILE_DIR == uType)
        {
                //����Ŀ¼
                strcpy(sDoLocalPath , localPathName);  // (1)
                //������ʱ��,tar.gz���ļ�����(oct_ftok($srcfilename,0).tar.gz)
                iKey = ftok(localPathName, 0); //�������,����key����ȷ����ͬ���ļ�
                snprintf(sTargzFileName, LenOfPathFileName, "oct_%d.tar.gz",  iKey);                
                strcpy(sDoFileName , sTargzFileName);  // (2)
                //ƴװϵͳ����
                snprintf(sCmd,  1024, "/bin/tar zcf %s *", sTargzFileName);  // (3)
        }
        else if (FILE_NORMAL == uType)
        {
                //�ֲ��Ŀ¼���;�����ļ�����
                divPathFile((char *)localPathName, sPath, sFileName);

                strcpy(sDoLocalPath, sPath); // (1)
                //������ʱ��,tar.gz���ļ�����(oct_ftok($srcfilename,0).tar.gz)
                iKey = ftok(localPathName, 0); //�������,����key����ȷ����ͬ���ļ�
                snprintf(sTargzFileName, LenOfPathFileName, "oct_%d.tar.gz",  iKey);
                strcpy(sDoFileName , sTargzFileName);  // (2)
                //ƴװϵͳ����
                sprintf(sCmd, "/bin/tar zcf %s %s", sTargzFileName, sFileName);  // (3)
        }
        else if (uType == FILE_TARGZ || uType == FILE_ZIP) //��2��ָ���ļ����Ƶķ�ʽ�����ؼ�ѹ��Ҫ�ⲿ�Լ���֤���ݶ��ڲ����账����������˷�
        {
                //�ֲ��Ŀ¼���;�����ļ�����
                divPathFile((char *)localPathName, sDoLocalPath, sDoFileName); // (1) (2) no(3)
        }
        else  //������ inlcude FILE_ERROR
        {
                //MAGIC_LOG(("sLocalPathName(%s) invalid.", sLocalPathName));
                return -100;
        }

        //[1.1](��ʱ�Ѿ����Ի�ñ��غ�Զ���ļ�������)�����ļ����ƣ�ƴװ�����ļ�ȫ·����Զ���ļ���ȫ·��
        //����
        if (sDoLocalPath[strlen(sDoLocalPath) - 1] == '/')
        {
                snprintf((char *)sLocalPathName, LenOfPathFileName, "%s%s",  sDoLocalPath, sDoFileName);
        }
        else
        {
                snprintf((char *)sLocalPathName, LenOfPathFileName, "%s/%s",  sDoLocalPath, sDoFileName);
        }
        //Զ��
        if (remotePathName[strlen(remotePathName) - 1] == '/')
        {
                snprintf((char *)sRemotePathName, LenOfPathFileName, "%s%s",  remotePathName, sDoFileName);
        }
        else
        {
                snprintf((char *)sRemotePathName, LenOfPathFileName, "%s/%s", remotePathName, sDoFileName);
        }        

        //[2.2]�����л�����Ӧ��Ŀ¼��,�����ļ������Ȼ���ڷ���ԭ����Ŀ¼

        if (NULL == getcwd(sCurDir, 512))  //��ǰ����
        {
                 //MAGIC_LOG(("getcwd failed."));
                 return -100;
        }
        if (0 != chdir(sDoLocalPath))  //�л�
        {
                //MAGIC_LOG(("chdir(%s) failed.", sDoLocalPath.c_str()));
                return -100;
        }
        if (strlen(sCmd) != 0)  //��Ҫִ�� (�Լ������2�����, �������Ѿ����ڵ��ļ�)
        {
                //�Ѿ��ڵ�ǰĿ¼��,�ж���������Ǹ�.tar.gz�ļ�������ɾ�������һ�����У�����һ����ȫ��ͬ���ļ�����������������ص�����
                if (0 == access(sLocalPathName,  F_OK)) //�ļ�����ʱ
                 {
                          iRet = remove(sLocalPathName); // delete
                          if (0 != iRet)
                          {
                               chdir(sCurDir);
                               //MAGIC_LOG(("file(%s) exist, del it failed.", m_sLocalPathName));
                               return -100;
                          }
                 }// �����ڵĻ�,�������������               
                
                //ƴװϵͳ����
                iRet = system(sCmd);
                if (-1 == iRet)
                {
                       chdir(sCurDir);
                       //MAGIC_LOG(("system(%s) failed.", sCmd));
                       return -100;
                }
        }
        chdir(sCurDir);  //����Ŀ¼
        
        //return 0;
        //�����ļ�Key��ʹ���ø�API֮�� ����ɾ���ļ�tar����hobo�棩
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

int oct_execute_()
{
        int sock_fd;

        //��Ϣ���������ļ��ݴ滺����
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

        //��һ����Ч�Լ�飬��ֹĳ������û������
        if (0 == strlen(sSiteName)) {
                sprintf(lastError, "Server or File info is invalid.");
                return -1;
        }

        //���ӵ���������
        sock_fd = -1;
        if (connect_tm(&sock_fd, 50, lastError) != 0) {
                close(sock_fd);
                return -2;
        }

        //������Ϣͷ��Ϣ
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
                        //����Э��ƴװ��Ϣ(����)
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

                        p += 3; //Ԥ���ֶ�
                        iCurSendMsgLen += 3;

                        if (FileName == eFileType) {
                                //�ļ������Ⱥ�����       
                                int iTmpBodyLen = htonl(strlen(sLocalPathName));
                                memcpy(p, (const char*)&iTmpBodyLen, 4);
                                p += 4;
                                iCurSendMsgLen += 4;

                                strncpy(p, sLocalPathName, strlen(sLocalPathName));
                                p += strlen(sLocalPathName);
                                iCurSendMsgLen += strlen(sLocalPathName);

                                //������Ϣ
                                if (send_n(sock_fd, sMsgBuff, iCurSendMsgLen, 10, lastError) != iCurSendMsgLen) {
                                        close(sock_fd);
                                        return -4;
                                }
                        } //else FileData
                        else
                        {
                                //���ɱ�����ļ����ƺ�·��(ǰ���Ѿ��жϹ��ˣ�վ�㵽����Ѿ�����)
                                FILE *fp = fopen(sLocalPathName, "r");
                                if (NULL == fp) {
                                        close(sock_fd);
                                        sprintf(lastError, "Open %s failed.", sLocalPathName);
                                        return -5;
                                }

                                //����һ���ļ��ĳ���
                                fseek(fp,0,SEEK_END); 
                                iFileSize = ftell(fp);
                                fseek(fp,0,SEEK_SET);

                                iTmpBodyLen = htonl(iFileSize);
                                memcpy(p, (const char*)&iTmpBodyLen, 4);
                                p += 4;
                                iCurSendMsgLen += 4;

                                //������Ϣ
                                if (send_n(sock_fd, sMsgBuff, iCurSendMsgLen, 10, lastError) != iCurSendMsgLen) {
                                        close(sock_fd);
                                        return -6;
                                }

                                //��ʼ�����ļ�����Ϣ
                                while(!feof(fp)) {
                                        //ע�⣬��ʹ������Ҫ���ĳ��ȣ�Ҳ�᷵����ȷ�Ķ����ĳ���
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
                        //����Э��ƴװ��Ϣ
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

                        //������Ϣ
                        if (send_n(sock_fd, sMsgBuff, iCurSendMsgLen, 10, lastError) != iCurSendMsgLen) {
                                close(sock_fd);
                                return -8;
                        }
                        break;
                }
        case ProtocolMvFile:
        case ProtocolCpFile:
                {
                        //����Э��ƴװ��Ϣ
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

                        //������Ϣ
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

        //������Ӧ
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
	//@1 ���ʹ���ļ�
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

//����ļ����еĺ�׺(�������ȡ��һ��.��������ж���)
int getTypeOfUploadPackFile(char * sFile)
{
        struct stat st1;
        char *pos = NULL;
        char sTmp[32]={0};
        
        //���ļ�
        if (sFile == NULL || strlen(sFile) == 0)
        {
                return FILE_ERROR;
        }

        //Ŀ¼
        if (stat(sFile, &st1) != 0)
        {
                return FILE_ERROR;
        }
        if(S_ISDIR(st1.st_mode))  //Ŀ¼
        {
                return  FILE_DIR;
        }

        pos = strchr(sFile,'.');
        //һ���ļ�
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
        
        //�Ƿ�Ϸ�
        if (sPathFile== NULL || 0 == strlen(sPathFile))
        {
                return;
        }

        //�������һ�� / ����Ϊǰ��ľ���·��������ľ����ļ�����
        lastPos = strrchr(sPathFile,'/');
        if (NULL == lastPos)
        {
                return;
        }

        //ȡ���ļ�����
        iPos = lastPos - sPathFile; 
        memcpy( sPath , sPathFile, iPos);
        memcpy( sFileName ,sPathFile+iPos+1, strlen(sPathFile) - iPos );
        return;
}

