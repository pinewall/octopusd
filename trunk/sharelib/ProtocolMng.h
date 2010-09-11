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
const int WriteToFileSize = 1024 * 1000;   //500K 开始写一次文件,大于MsgBufferSize


//协议调度类
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
                ProtocolAddSubServer = 0x13,  //保留
                ProtocolDelSubServer = 0x14,    //保留
                //BEGIN kevinliu add 2006-04-21
                ProtocolMvFile = 0x15,
                ProtocolCpFile = 0x16,
                //END kevinliu add 2006-04-21
                //BEGIN kevinliu add 2007-01-10                
                ProtocolPackFile = 0x17,   //打包上传
                //END kevinliu add 2007-01-10

                //kevinliu add for 2.1版本 - 2007-04-29
                //协议格式和上面的偏移量不同,采用TLV的结构,结构为
                // Version(char) + MsgType(char) + TotalLen(int) + TLV(1...n)
                // 响应消息,也是上面的结构
                ProtocolSearchTree = 0x20, //获得分发数结构
                ProtocolCommonCfg = 0x21,  //修改基础配置信息
                ProtocolSiteCfg = 0x22,      //修改站点配置信息
                ProtocolMobileCfg = 0x23,  //修改告警文件
                ProtocolLogTrace = 0x24,    //察看日志情况
                ProtocolShowQInfo = 0x25  //查看队列情况
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
                Version2 = 0x02  //kevinliu add for 2.1版本 - 2007-04-29
        };

        enum StatusType
        {
                Critical = 2,    /*严重故障*/
                Warning = 1,   /*告警*/
                OK = 0,             /*正常*/
                Unknown =  -1 /*其他错误*/
        };
        /**
         * \brief 接受消息处理
         * \param insTcpServer SocketServer的实例
         * \return void
         */        
        static void processRecv(CTCPSocket& insTcpServer);

        /**
         * \brief 发送消息处理
         * \param int iType 0代表处理常规任务，1代表处理失败任务
         * \return void
         */        
        static void processSend(int iType);  // 2007-05-21 add itype

        /**
         * \brief 检查服务器的状态
         * \param ip地址 port端口 iTimeOut等待时间,如果为0，则为配置文件的时间，如果为其他正数，则为除数，取商
         * \return 0 ok -1 not ok
         */   
        static int checkServerStatus(string sIP, int iPort, int iTimeOut = 0);

        /**
         * \brief 发送和接收响应消息
         * \param insSocket 要处理的socket，响应码
         * \return 0成功有响应码 其他，失败，没有获得响应码
         */
        static int sendResponse(CTCPSocket&  insSocket, ResponseCode resp);
        static int recvResponse(CTCPSocket&  insSocket, ResponseCode& resp);     

        //kevinliu add for 2.1版本 - 2007-04-29
        //处理后台管理的任务
        static void processVersion2(CTCPSocket & insTcpServer, char cProtocolType);
        static void setVersion2Resp(vector<string>& vecmsgContent, const string& sCode, 
                const string& sDesc, const string& value);        
        static int recvVersion2Msg(CTCPSocket & insTcpServer, vector<string>& vecRecvContent);
        static int sendVersion2Msg(CTCPSocket & insTcpServer, char cMsgType, vector<string>& vecSendContent);
        //将消息转发到下面的分发服务器，sitename为空，表示所有的服务器都要转发消息，不为空，则表示只向对于得分发服务器发消息
        static int forwardVersion2Msg(char cMsgType, vector<string>& vecmsgContent,  const string& sitename);        

        static int cloneSiteCfg(const string& sBySiteName, const string& sAddSiteName);
};

#endif /* PROTOCOLMNG_H_HEADER_INCLUDED_BCE1B9DA */


