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
const int WriteToFileSize = 1024 * 1000;   //500K 开始写一次文件,大于MsgBufferSize
const int LenOfSiteName = 32;   //站点名称长度
const int LenOfMsgTime = 20;       //时间格式长度
const int LenOfPathFileName = 256;  //路径和文件名称的长度

//章鱼服务的客户端发送类
class CRsyncOct
{
public:        
        //上传文件的类型
        enum UploadFileType
        {
                FileName = 0x1,  //文件名称
                FileData = 0x2   //文件流
        };
        //支持的消息类别
        enum ProtocolType
        {
                ProtocolUploadFile = 0x01, //上传文件
                ProtocolDelFile = 0x02,  //删除文件
                ProtocolMvFile = 0x15,  //移动
                ProtocolCpFile = 0x16,   //拷贝
                ProtocolPackFile = 0x17   //打包上传
        };
        //消息版本
        enum VersionType
        {
                Version1 = 0x01
        };
        enum UploadPackFileType
        {
                FILE_ERROR = 0,  //错误
                FILE_DIR = 1, //目录
                FILE_TARGZ = 2, // *.tar.gz
                FILE_ZIP = 3,  // *.zip
                FILE_NORMAL = 4 //普通文件
        };

        //返回值得含义
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
        
        //构造和析构函数
        CRsyncOct();
        ~CRsyncOct();
        
        /**
         *  \brief 设置要发送的服务器的IP和端口，主要由于服务端要鉴权，因此需要把本机IP在服务器端配置
         *  \param sIP 服务器端IP
         *  \param iPort 服务端端口
         *  \return void
         */
        void setServer(const string& sIP, int iPort);
        
        /**
         *  \brief 设置上传消息的内容
         *  \param cType 上传文件的类型
         *  \param sSiteName 站点名称
         *  \param sRemotePathName 文件在远端的相对主站目录的相对目录和文件名称
         *  \param sLocalPathName 文件在本地的绝对目录和名称
         *  \return void
         */
        void setUploadInfo(UploadFileType cType, const char* sSiteName, const char* sRemotePathName, const char* sLocalPathName);

        /**
         *  \brief 设置打包上传消息的内容
         *  \param cType 上传文件的类型
         *  \param sSiteName 站点名称
         *  \param sRemotePath 文件在远端的相对主站目录的相对目录
         *  \param sLocalPathName 文件在本地的绝对目录和名称
         *  \return -100表示失败,0成功
         */
        int setUploadPackInfo(UploadFileType cType, const char* sSiteName, const char* sRemotePathName, const char* sLocalPathName);        
        
        /**
         *  \brief 设置上传消息的内容
         *  \param sSiteName 站点名称
         *  \param sRemotePathName 文件在远端的相对主站目录的相对目录和文件名称
         *  \return void
         */
        void setDeleteInfo(const char* sSiteName, const char* sRemotePathName);
        
        /**
         *  \brief 移动和拷贝文件
         *  \param sdestSiteName 目的站点名称
         *  \param sDestPathName 目的目录文件,相对于站点的主目录
         *  \param sSrcSiteName 源站点名称
         *  \param sSrcPathName 源目录文件,相对于站点的主目录
         *  \return void
         */
        void setMvInfo(const char* sDestSiteName, const char* sDestPathName, const char* sSrcSiteName, const char* sSrcPathName);
        void setCpInfo(const char* sDestSiteName, const char* sDestPathName, const char* sSrcSiteName, const char* sSrcPathName);
        
        /**
         *  \brief 前面设置成功后，发送消息
         *  \return 成功相应码，0成功，负数失败
         */
        int execute();
        int uploadFile(const string& sIP, int iPort,UploadFileType cType, const char* sSiteName, const char* sRemotePathName, const char* sLocalPathName);
        int deleteFile(const string& sIP, int iPort,const char* sSiteName, const char* sRemotePathName);
        int mvFile(const string& sIP, int iPort, const char* sDestSiteName, const char* sDestPathName, const char* sSrcSiteName, const char* sSrcPathName);
        int cpFile(const string& sIP, int iPort, const char* sDestSiteName, const char* sDestPathName, const char* sSrcSiteName, const char* sSrcPathName);
        /*(1) 源文件
                @如果是个.tar.gz结尾的文件，则上传后解包
                @如果是个.zip结尾的文件，也是上传后解包
                @如果是个普通的文件，则压缩成.tar.gz，上传后接包 
                @如果是个目录，则在这个目录下，将目录下所有的文件打包，上传后解包
             (2)目标文件
                @目标文件，是一个路径，不能是一个具体的文件名称,解包将基于此目录
             (3)章鱼API只负责打包，不负责删除临时文件(没有控制),如果因磁盘空间问题，请自行安排删除
                如对空间有要求,请采用应用自己打包,发送完成后,自行控制删除的方法(必须确保所有服务都已经完成任务了)
                @进行打包操作时,文件的命名方式为:
                oct_ftok($srcfilename,0).tar.gz的方式
                这种命名方法主要节省空间，防止相同的任务会生成多个不同的临时文件的情况
                */
        int uploadPackFile(const string& sIP, int iPort,UploadFileType cType, const char* sSiteName, const char* sRemotePath, const char* sLocalPathName);

private:
        /**
         *  \brief 根据当前时间生成消息中的时间格式
         *  \return 消息中的时间格式
         */
        string getMsgTime();

        //获得上传文件的类型
        UploadPackFileType getTypeOfUploadPackFile(string sFile);
        //分割文件名称和路径
        void divPathFile(string sPathFile, string& sPath, string& sFileName);
        
        //服务器IP和端口
        string m_sServerIP;
        int m_iServerPort;
        //消息类型
        ProtocolType m_eProtocolType;
        //文件类型
        UploadFileType m_eFileType;
        //站点名
        char m_sSiteName[LenOfSiteName];
        //远端目录文件和名称
        char m_sRemotePathName[LenOfPathFileName];
        //本地目录和名称
        char m_sLocalPathName[LenOfPathFileName]; 
        //发送时间
        char m_sSendTime[LenOfMsgTime];               
};

#endif
