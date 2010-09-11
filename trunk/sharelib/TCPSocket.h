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
const int MaxIgnoreSignal = 10;  //��

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

        //ɨ��ģʽ
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
         * \brief ��ʼ�����Ա����Ҫ������server��
         * \param sIP ����IP
         * \param iPort ����˿�
         * \return void
         */ 
        void init(const char* sIP, int iPort);

        /**
         * \brief ��ʼ�����Ա����Ҫ������client��
         * \param iFd ���Ӿ��
         * \param sIP ����IP
         * \param iPort ����˿�
         * \return void
         */ 
        void init(int iFd, const char* sIP, int iPort);

        /**
         * \brief ����socket server��
         * \param void
         * \return 0 �ɹ� ���� ʧ��
         */ 
        int createSocket();

        /**
         * \brief ���տͻ��˵�����
         * \param sIP �ͻ��˵�IP
         * \return -1��0ʧ�ܣ�����0���ɹ��ҷ���socketid 
         */ 
        int acceptClient(char* sIP);

        /**
         * \brief ����socket server
         * \param void
         * \return 0 �ɹ� ���� ʧ��
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
         * \brief ������Ϣ
         * \param pBuffer ��Ϣ����������Ϣ����
         * \param iLen �������ĳ���
         * \param iTimeout ������Ϣ�ĳ�ʱʱ�� -1�޳�ʱ
         * \return -1ʧ�ܣ�0�ɹ�
         */ 
        int sendMsg(char* pBuffer, int iLen, int iTimeout = -1);

        /**
         * \brief ������Ϣ
         * \param pBuffer ��Ϣ����������Ϣ����
         * \param iLen �������ĳ���
         * \param iTimeout ������Ϣ�ĳ�ʱʱ��
         * \return ���� ʧ�ܣ�0 �ɹ�
         */ 
        int receiveMsg(char* pBuffer, int iLen, int iTimeout = -1);

        /**
         * \brief ����ģʽ
         * \param iBlockFlag ����ģʽ 0 �� 1 ����
         * \return -1ʧ�ܣ����� �ɹ�
         */        
        int  setBlockMode(BlockMode iBlockFlag);

        /**
         * \brief �ر�socket
         * \param void
         * \return void
         */   
        void  closeSocket();

public:
        
        // ���Ӿ��
        int m_iFd;

        // IP��ַ
        char m_sIP[LenOfIPAddr];

        //����˿�
        int m_iPort;

        //����ģʽ 0 non block 1 block
        int m_iBlockMode;

};

#endif /* TCPSOCKET_H_HEADER_INCLUDED_BCE1BA0A */


