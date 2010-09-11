#ifndef RSYNCOCT_H_HEADER_INCLUDED
#define RSYNCOCT_H_HEADER_INCLUDED

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/stat.h>

#include "TcpConnector.h"
#include "SocketAddr.h"
#include "Trace.h"

const int MsgBufferSize = 1024 * 10;   //10K
const int WriteToFileSize = 1024 * 1000;   //500K ��ʼдһ���ļ�,����MsgBufferSize
const int LenOfSiteName = 32;   //վ�����Ƴ���
const int LenOfMsgTime = 20;       //ʱ���ʽ����
const int LenOfPathFileName = 256;  //·�����ļ����Ƶĳ���

//�������Ŀͻ��˷�����
class CRsyncOct
{
public:        
        //�ϴ��ļ�������
        enum UploadFileType
        {
                FileName = 0x1,  //�ļ�����
                FileData = 0x2   //�ļ���
        };
        //֧�ֵ���Ϣ���
        enum ProtocolType
        {
                ProtocolUploadFile = 0x01, //�ϴ��ļ�
                ProtocolDelFile = 0x02,  //ɾ���ļ�
                ProtocolMvFile = 0x15,  //�ƶ�
                ProtocolCpFile = 0x16,   //����
                ProtocolPackFile = 0x17   //����ϴ�
        };
        //��Ϣ�汾
        enum VersionType
        {
                Version1 = 0x01
        };
        enum UploadPackFileType
        {
                FILE_ERROR = 0,  //����
                FILE_DIR = 1, //Ŀ¼
                FILE_TARGZ = 2, // *.tar.gz
                FILE_ZIP = 3,  // *.zip
                FILE_NORMAL = 4 //��ͨ�ļ�
        };

        //����ֵ�ú���
         enum ResponseCode
        {
                checkOK = 0x00,
                Success = 0x01, 
                Fail = 0x02, 
                HeadFail = 0x03,
                InfoFail = 0x04,
                DataFail = 0x05,
                MsgError = 0x06,
                ServicePause = 0x07,
                CancelDuty = 0x08,
                UnknownCode = 0x09
        };
        
        //�������������
        CRsyncOct();
        ~CRsyncOct();
        
        /**
         *  \brief ����Ҫ���͵ķ�������IP�Ͷ˿ڣ���Ҫ���ڷ����Ҫ��Ȩ�������Ҫ�ѱ���IP�ڷ�����������
         *  \param sIP ��������IP
         *  \param iPort ����˶˿�
         *  \return void
         */
        void setServer(const string& sIP, int iPort);
        
        /**
         *  \brief �����ϴ���Ϣ������
         *  \param cType �ϴ��ļ�������
         *  \param sSiteName վ������
         *  \param sRemotePathName �ļ���Զ�˵������վĿ¼�����Ŀ¼���ļ�����
         *  \param sLocalPathName �ļ��ڱ��صľ���Ŀ¼������
         *  \return void
         */
        void setUploadInfo(UploadFileType cType, const char* sSiteName, const char* sRemotePathName, const char* sLocalPathName);

        /**
         *  \brief ���ô���ϴ���Ϣ������
         *  \param cType �ϴ��ļ�������
         *  \param sSiteName վ������
         *  \param sRemotePath �ļ���Զ�˵������վĿ¼�����Ŀ¼
         *  \param sLocalPathName �ļ��ڱ��صľ���Ŀ¼������
         *  \return -100��ʾʧ��,0�ɹ�
         */
        int setUploadPackInfo(UploadFileType cType, const char* sSiteName, const char* sRemotePathName, const char* sLocalPathName);        
        
        /**
         *  \brief �����ϴ���Ϣ������
         *  \param sSiteName վ������
         *  \param sRemotePathName �ļ���Զ�˵������վĿ¼�����Ŀ¼���ļ�����
         *  \return void
         */
        void setDeleteInfo(const char* sSiteName, const char* sRemotePathName);
        
        /**
         *  \brief �ƶ��Ϳ����ļ�
         *  \param sdestSiteName Ŀ��վ������
         *  \param sDestPathName Ŀ��Ŀ¼�ļ�,�����վ�����Ŀ¼
         *  \param sSrcSiteName Դվ������
         *  \param sSrcPathName ԴĿ¼�ļ�,�����վ�����Ŀ¼
         *  \return void
         */
        void setMvInfo(const char* sDestSiteName, const char* sDestPathName, const char* sSrcSiteName, const char* sSrcPathName);
        void setCpInfo(const char* sDestSiteName, const char* sDestPathName, const char* sSrcSiteName, const char* sSrcPathName);
        
        /**
         *  \brief ǰ�����óɹ��󣬷�����Ϣ
         *  \return �ɹ���Ӧ�룬0�ɹ�������ʧ��
         */
        int execute();
        int uploadFile(const string& sIP, int iPort,UploadFileType cType, const char* sSiteName, const char* sRemotePathName, const char* sLocalPathName);
        int deleteFile(const string& sIP, int iPort,const char* sSiteName, const char* sRemotePathName);
        int mvFile(const string& sIP, int iPort, const char* sDestSiteName, const char* sDestPathName, const char* sSrcSiteName, const char* sSrcPathName);
        int cpFile(const string& sIP, int iPort, const char* sDestSiteName, const char* sDestPathName, const char* sSrcSiteName, const char* sSrcPathName);
        /*(1) Դ�ļ�
                @����Ǹ�.tar.gz��β���ļ������ϴ�����
                @����Ǹ�.zip��β���ļ���Ҳ���ϴ�����
                @����Ǹ���ͨ���ļ�����ѹ����.tar.gz���ϴ���Ӱ� 
                @����Ǹ�Ŀ¼���������Ŀ¼�£���Ŀ¼�����е��ļ�������ϴ�����
             (2)Ŀ���ļ�
                @Ŀ���ļ�����һ��·����������һ��������ļ�����,��������ڴ�Ŀ¼
             (3)����APIֻ��������������ɾ����ʱ�ļ�(û�п���),�������̿ռ����⣬�����а���ɾ��
                ��Կռ���Ҫ��,�����Ӧ���Լ����,������ɺ�,���п���ɾ���ķ���(����ȷ�����з����Ѿ����������)
                @���д������ʱ,�ļ���������ʽΪ:
                oct_ftok($srcfilename,0).tar.gz�ķ�ʽ
                ��������������Ҫ��ʡ�ռ䣬��ֹ��ͬ����������ɶ����ͬ����ʱ�ļ������
                */
        int uploadPackFile(const string& sIP, int iPort,UploadFileType cType, const char* sSiteName, const char* sRemotePath, const char* sLocalPathName);

private:
        /**
         *  \brief ���ݵ�ǰʱ��������Ϣ�е�ʱ���ʽ
         *  \return ��Ϣ�е�ʱ���ʽ
         */
        string getMsgTime();

        //����ϴ��ļ�������
        UploadPackFileType getTypeOfUploadPackFile(string sFile);
        //�ָ��ļ����ƺ�·��
        void divPathFile(string sPathFile, string& sPath, string& sFileName);
        
        //������IP�Ͷ˿�
        string m_sServerIP;
        int m_iServerPort;
        //��Ϣ����
        ProtocolType m_eProtocolType;
        //�ļ�����
        UploadFileType m_eFileType;
        //վ����
        char m_sSiteName[LenOfSiteName];
        //Զ��Ŀ¼�ļ�������
        char m_sRemotePathName[LenOfPathFileName];
        //����Ŀ¼������
        char m_sLocalPathName[LenOfPathFileName]; 
        //����ʱ��
        char m_sSendTime[LenOfMsgTime];               
};

#endif
