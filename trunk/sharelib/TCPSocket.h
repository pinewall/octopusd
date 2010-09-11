//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: TCPSocket.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: Kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef TCPSOCKET_H_HEADER_INCLUDED_BCE1BA0A
#define TCPSOCKET_H_HEADER_INCLUDED_BCE1BA0A

#include "ComFun.h"
#include "ConfigMng.h"
#include "LogMng.h"

const int DefaultSelectTimeOut = 10; //second
const int MaxIgnoreSignal = 10;  //次

//TCP Socket
class CTCPSocket
{
      public:
        CTCPSocket();
        CTCPSocket(const char* sIP, int iPort);
        CTCPSocket(int iFd, const char* sIP, int iPort);
        CTCPSocket(const CTCPSocket& right);
        CTCPSocket& operator=(const CTCPSocket& right);
        ~CTCPSocket();

        //扫描模式
        enum SelectMode
        {
                WriteMode = 0,
                ReadMode = 1,
                ReadAndWriteMode = 2
        };

        enum BlockMode
        {
                NonBlock = 0,
                Block = 1
        };

        /**
         * \brief 初始化类成员，主要用在做server端
         * \param sIP 服务IP
         * \param iPort 服务端口
         * \return void
         */ 
        void init(const char* sIP, int iPort);

        /**
         * \brief 初始化类成员，主要用在做client端
         * \param iFd 连接句柄
         * \param sIP 服务IP
         * \param iPort 服务端口
         * \return void
         */ 
        void init(int iFd, const char* sIP, int iPort);

        /**
         * \brief 建立socket server端
         * \param void
         * \return 0 成功 其他 失败
         */ 
        int createSocket();

        /**
         * \brief 接收客户端的连接
         * \param sIP 客户端的IP
         * \return -1和0失败，大于0，成功且返回socketid 
         */ 
        int acceptClient(char* sIP);

        /**
         * \brief 连接socket server
         * \param void
         * \return 0 成功 其他 失败
         */ 
        int connectServer();

        /**
         * \brief select socket
         * \param iTimeOut: The max waiting time, in second. 0 means to wait forever.
         * \param eSelectMode: 1=wait for reading, 0=wait for writing, 2=both write and read.
         * \return The waiting time, in second, -1=FAIL
         */         
        int checkSocket(const int iTimeOut, SelectMode eSelectMode = ReadMode);

        /**
         * \brief 发送消息
         * \param pBuffer 消息缓冲区，消息内容
         * \param iLen 缓冲区的长度
         * \param iTimeout 发送消息的超时时间 -1无超时
         * \return -1失败；0成功
         */ 
        int sendMsg(char* pBuffer, int iLen, int iTimeout = -1);

        /**
         * \brief 接收消息
         * \param pBuffer 消息缓冲区，消息内容
         * \param iLen 缓冲区的长度
         * \param iTimeout 发送消息的超时时间
         * \return 负数 失败；0 成功
         */ 
        int receiveMsg(char* pBuffer, int iLen, int iTimeout = -1);

        /**
         * \brief 阻塞模式
         * \param iBlockFlag 阻塞模式 0 非 1 阻塞
         * \return -1失败；其他 成功
         */        
        int  setBlockMode(BlockMode iBlockFlag);

        /**
         * \brief 关闭socket
         * \param void
         * \return void
         */   
        void  closeSocket();

public:
        
        // 连接句柄
        int m_iFd;

        // IP地址
        char m_sIP[LenOfIPAddr];

        //服务端口
        int m_iPort;

        //阻塞模式 0 non block 1 block
        int m_iBlockMode;

};

#endif /* TCPSOCKET_H_HEADER_INCLUDED_BCE1BA0A */


