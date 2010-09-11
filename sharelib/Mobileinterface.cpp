/************************************************************************
 *  This is the source of an commercial software; you can't redistribute 
 *  it and/or modify it without the author's permit. All rights reserved.
 *  You may get support by contact bud@tencent.com if you really need.	 
 *                                                                       
 *    $RCSfile: Mobileinterface.cpp,v $
 *    $Revision: 1.0 $
 *    $Author: kevinliu $
 *    $Name:  $
 *    $Date: 2005/09/07 11:52:29 $
 ***********************************************************************/

#include "Mobileinterface.h"

extern CLogMng g_insLogMng;

int CMobileInterface::SendSMSMsg(const string& sMobile, const string& sContent,
        const string& sSMSGWIP, const unsigned short sSMSGWPort)
{
         CTCPSocket TcpConn(sSMSGWIP.c_str(), sSMSGWPort); 

         //首先连接短信网关
         if(TcpConn.connectServer() != 0)
         {
                   g_insLogMng.debug("socket", CLogMng::LogMode2, "connect to sms agent server failed.");
                   return -1;
         }
 
         //组织命令包
         int iLen = sizeof(TSmsCMDPacket) - 6;
         iLen = htonl(iLen);
         TSmsCMDPacket stCMDPacket;
         stCMDPacket.cSTX = STX;
         stCMDPacket.cCMD = 0x01;
         memcpy(stCMDPacket.Length,&iLen,4);
         stCMDPacket.cMODE = 0;
         strncpy(stCMDPacket.sMobile,sMobile.c_str(),11);
         stCMDPacket.sMobile[11] = 0;
         strncpy(stCMDPacket.sContent,sContent.c_str(),140);
         stCMDPacket.sContent[140] = 0;

         //向短信网关发送消息
         if(TcpConn.sendMsg((char*)&stCMDPacket,sizeof(stCMDPacket),5) < 0)
         {
                   g_insLogMng.debug("socket", CLogMng::LogMode1, "send cmd to agent failed.");
                   TcpConn.closeSocket();
                   return -1;
         }

         int iRecvLen;

         //接收从短信网关返回的响应
         char sRecvBuf[512];
         memset(sRecvBuf, 0, 512);
         if((iRecvLen = TcpConn.receiveMsg(sRecvBuf,sizeof(TSmsAgentResponse),5)) < 0)
         {
                   g_insLogMng.debug("socket", CLogMng::LogMode1, "recv from agent server failed.");
                   TcpConn.closeSocket();
                   return -1;
         }
         TcpConn.closeSocket();

         //判断响应是否正确 
         TSmsAgentResponse stAgentReponse;
         if(iRecvLen != sizeof(stAgentReponse))
         {
                   g_insLogMng.debug("socket", CLogMng::LogMode1, "received an incorrect response packet.");
                   return -1;
         } 
         memcpy(&stAgentReponse,sRecvBuf,sizeof(stAgentReponse));
         if(stAgentReponse.cResult != SMS_AGENT_OK)
         {
                   g_insLogMng.debug("socket", CLogMng::LogMode1, "Receive an error response result(%d)", stAgentReponse.cResult);
                   return -1;
         }
         return 0;
}

