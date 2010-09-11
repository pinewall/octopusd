//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ProtocolMng.h,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef PROTOCOLMNG_H_HEADER_INCLUDED_BCE1B9DA
#define PROTOCOLMNG_H_HEADER_INCLUDED_BCE1B9DA

#include "ComFun.h"
#include "QueueMng.h"
#include "LogMng.h"
#include "RecvingDuty.h"
#include "SendDuty.h"
#include "TCPSocket.h"
#include "FileUploadMsg.h"
#include "FileDelMsg.h"
#include "ServerMng.h"
#include "ServerAddMsg.h"
#include "ServerDelMsg.h"

const int MsgHeadLen = 2;
const int MsgBufferSize = 1024 * 10;   //10K
const int WriteToFileSize = 1024 * 1000;   //500K ��ʼдһ���ļ�,����MsgBufferSize


//Э�������
class CProtocolMng
{
public:

        enum ProtocolType
        {
                ProtocolUploadFile = 0x01,
                ProtocolDelFile = 0x02,
                ProtocolAddServer = 0x03,
                ProtocolDelServer = 0x04,
                ProtocolHeartBeat = 0x06,
                ProtocolCheckStatus = 0x07,
                ProtocolAddSubServer = 0x13,  //����
                ProtocolDelSubServer = 0x14,    //����
                //BEGIN kevinliu add 2006-04-21
                ProtocolMvFile = 0x15,
                ProtocolCpFile = 0x16,
                //END kevinliu add 2006-04-21
                //BEGIN kevinliu add 2007-01-10                
                ProtocolPackFile = 0x17,   //����ϴ�
                //END kevinliu add 2007-01-10

                //kevinliu add for 2.1�汾 - 2007-04-29
                //Э���ʽ�������ƫ������ͬ,����TLV�Ľṹ,�ṹΪ
                // Version(char) + MsgType(char) + TotalLen(int) + TLV(1...n)
                // ��Ӧ��Ϣ,Ҳ������Ľṹ
                ProtocolSearchTree = 0x20, //��÷ַ����ṹ
                ProtocolCommonCfg = 0x21,  //�޸Ļ���������Ϣ
                ProtocolSiteCfg = 0x22,      //�޸�վ��������Ϣ
                ProtocolMobileCfg = 0x23,  //�޸ĸ澯�ļ�
                ProtocolLogTrace = 0x24,    //�쿴��־���
                ProtocolShowQInfo = 0x25  //�鿴�������
        };
	
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

        enum VersionType
        {
                Version1 = 0x01,
                Version2 = 0x02  //kevinliu add for 2.1�汾 - 2007-04-29
        };

        enum StatusType
        {
                Critical = 2,    /*���ع���*/
                Warning = 1,   /*�澯*/
                OK = 0,             /*����*/
                Unknown =  -1 /*��������*/
        };
        /**
         * \brief ������Ϣ����
         * \param insTcpServer SocketServer��ʵ��
         * \return void
         */        
        static void processRecv(CTCPSocket& insTcpServer);

        /**
         * \brief ������Ϣ����
         * \param int iType 0������������1������ʧ������
         * \return void
         */        
        static void processSend(int iType);  // 2007-05-21 add itype

        /**
         * \brief ����������״̬
         * \param ip��ַ port�˿� iTimeOut�ȴ�ʱ��,���Ϊ0����Ϊ�����ļ���ʱ�䣬���Ϊ������������Ϊ������ȡ��
         * \return 0 ok -1 not ok
         */   
        static int checkServerStatus(string sIP, int iPort, int iTimeOut = 0);

        /**
         * \brief ���ͺͽ�����Ӧ��Ϣ
         * \param insSocket Ҫ�����socket����Ӧ��
         * \return 0�ɹ�����Ӧ�� ������ʧ�ܣ�û�л����Ӧ��
         */
        static int sendResponse(CTCPSocket&  insSocket, ResponseCode resp);
        static int recvResponse(CTCPSocket&  insSocket, ResponseCode& resp);     

        //kevinliu add for 2.1�汾 - 2007-04-29
        //�����̨���������
        static void processVersion2(CTCPSocket & insTcpServer, char cProtocolType);
        static void setVersion2Resp(vector<string>& vecmsgContent, const string& sCode, 
                const string& sDesc, const string& value);        
        static int recvVersion2Msg(CTCPSocket & insTcpServer, vector<string>& vecRecvContent);
        static int sendVersion2Msg(CTCPSocket & insTcpServer, char cMsgType, vector<string>& vecSendContent);
        //����Ϣת��������ķַ���������sitenameΪ�գ���ʾ���еķ�������Ҫת����Ϣ����Ϊ�գ����ʾֻ����ڵ÷ַ�����������Ϣ
        static int forwardVersion2Msg(char cMsgType, vector<string>& vecmsgContent,  const string& sitename);        

        static int cloneSiteCfg(const string& sBySiteName, const string& sAddSiteName);
};

#endif /* PROTOCOLMNG_H_HEADER_INCLUDED_BCE1B9DA */


