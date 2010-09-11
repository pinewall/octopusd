//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: octopustool.cpp $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "ComFun.h"
#include "ConfigMng.h"
#include "LogMng.h"
#include "TCPSocket.h"
#include "Mobileinterface.h"
#include "Maintain.h"
#include "ProtocolMng.h"
#include "QueueMng.h"

string g_sHomeDir;
string g_sProgramName;
CLogMng g_insLogMng;
CMaintain g_insMaintain;
CConfigMng g_insConfigMng;
int g_iFileType;
//map<int, CSendDuty> g_mapthreadSendDuty; 
CQueueMng g_insQueueMng;

//连接池
CThreadMutex g_insConCtrl;  //锁控制
list<CTCPSocket> g_lstConnectPool;

map<int, int> g_mapThread2pid;


/**
 *  \brief 信号处理函数
 *  \brief 进程收到的信号
 */
void sighandler(int signum)
{
        sigset_t block_mask;

        g_insLogMng.debug("all", CLogMng::LogMode1, "[pid %d got a signal %d]", getpid(), signum);

        if (signum == SIGCHLD)
        {
                int errno2, result;
                pid_t deadpid;
                errno2 = errno;
                while((deadpid = waitpid(0, &result, WNOHANG)) > 0)
                {
                }

                errno = errno2;
                signal(SIGCHLD, sighandler);
        }
        else if (signum == SIGUSR1)
        {
                signal(SIGUSR1, sighandler);
        }
        else if ( signum == SIGINT || signum == SIGTERM)
        {
                /*屏蔽所有可能的信号*/
                sigemptyset( &block_mask );
                sigaddset( &block_mask, SIGINT );
                sigaddset( &block_mask, SIGQUIT );
                sigaddset( &block_mask, SIGTERM );
                sigaddset( &block_mask, SIGCHLD );
                sigaddset( &block_mask, SIGIO );
                sigaddset( &block_mask, SIGPIPE );
                sigaddset( &block_mask, SIGHUP );
                sigprocmask( SIG_BLOCK, &block_mask, NULL );
                g_insLogMng.normal("Process is killed by signal %d.", signum );
                exit(0);
        }
}


void printMenu0_M()
{
        printf("\n\nnewctopusd Admin Tool Function Menu:\n");
        printf("\t [1] Log Control Manage...\n");
        printf("\t [2] Config Info Manage...\n");
        printf("\t [3] Send Duty Queue Manage...\n");
        printf("\t [4] Emulate Wizardwebdev Program...\n");
        printf("\t [5] Stop Newoctopusd Program...\n");
        printf("\t [6] Odditional Function ...\n");
        printf("\t [0] Exit\n");
        printf("Please input your choice ===>");
        return;
};
void printMenu0_1()
{
        printf("\n\nLog Control Manage list:\n");
        printf("\t [1] Set Log Level...\n");
        printf("\t [2] Set Log Module...\n");
        printf("\t [0] Return\n");
        printf("Please input your choice ===>");
        return;
};

void printMenu0_1_1(int iCur)
{
        printf("\n\nSet Log Level list:\n");
        printf("\t [1] None debug\n");
        printf("\t [2] Primary Level\n");
        printf("\t [3] Senior Level\n");
        printf("\t [4] high-level Level\n");
        printf("\t [0] Return\n");
        printf("Cur the level is %d, Please input your choice ===>", iCur+1);
        return;
};

void printMenu0_1_2(int iCur, map<string, string>& insMapModule)
{
        printf("\n\nSet Log Module list:\n");
        map<string, string>::iterator it = insMapModule.begin();
        for(; it != insMapModule.end();it++)
        {
                printf("\t [%d] %s\n", atoi(it->second.c_str()) + 1, it->first.c_str());
        }
        printf("\t [0] Return\n");
        printf("Cur the level is %d, Please input your choice ===>", iCur+1);
        return;
};

void printMenu0_2()
{
        printf("\n\nConfig Info Manage:\n");
        printf("\t [1] Print Config Info to File\n");
        printf("\t [2] Reload Common Config File to Memory\n");
        printf("\t [3] Reload Site Info Config File to Memory\n");
        printf("\t [4] Reload Module Config File to Memory\n");
        printf("\t [0] Return\n");
        printf("Please input your choice ===>");
        return;
};

void printMenu0_3()
{
        printf("\n\nSend Duty Queue Manage:\n");
        printf("\t [1] Reload FailedDutyFile to sendQueue...\n");
        printf("\t [2] Print SendQueue status to file\n");
        printf("\t [3] Print Distribute Servers Run Status\n");
        printf("\t [4] Convert FailedDutyFile from data to text...\n");
        printf("\t [5] Clear Disabled Duty by sitecfg.cfg...\n");
        printf("\t [6] Queue Safe Mode(0: check locate time And dont mv form error to normal queue).\n");  //危险，慎用
        printf("\t [0] Return\n");
        printf("Please input your choice ===>");
        return;
};

void printMenu0_4()
{
        printf("\n\nEmulate Wizardwebdev Program:\n");
        printf("\t [1] Input Simple-Duty to Newoctopusd...\n");
        printf("\t [2] Send mult-Duties from DutyFile(dat/testduty.list)...\n");
        printf("\t [3] Send a statuscheck to Newoctopusd\n");
        printf("\t [4] Send a statuscheck to Distribute Server...\n");
        printf("\t [5] Send a ServerAdd to Newoctopusd...\n");
        printf("\t [6] Send a ServerDel to Newoctopusd...\n");
        printf("\t [0] Return\n");
        printf("Please input your choice ===>");
        return;
};

void printMenu0_4_2()
{
        printf("\n\nSend many Duties File Type:\n");
        printf("\t [1] times...\n");
        printf("\t [2] cycle\n");
        printf("\t [0] Return\n");
        printf("Please input your choice ===>");
        return;
};

void printMenu0_6()
{
        printf("\n\nOdditional Function:\n");
        printf("\t [1] Print all share memory var value\n");
        printf("\t [2] Delete Share memory\n");
        printf("\t [3] Send a moblie short message\n");
        printf("\t [4] Print ThreadID to PID\n");
        printf("\t [5] Close All Accept Socket Connection\n");
        printf("\t [6] Clear All Error IPPort record\n");
        printf("\t [7] Close All Socket Connection(FD:10~1024) to reback fd.\n");  //危险，慎用
        printf("\t [0] Return\n");
        printf("Please input your choice ===>");
        return;
};


int sendstatuscheck(string sIP, int iPort)
{
        int iMsgTimeout = g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout;  //消息超时时间
        if (0 == strcmp(sIP.c_str(), "ALL") || 0 == strcmp(sIP.c_str(), "ALL"))
        {
                sIP = "127.0.0.1";
        }  
        CTCPSocket  insClientSocket(sIP.c_str(), iPort);
        if ( 0 != insClientSocket.connectServer())
        {
                g_insLogMng.error("Connect to server(%s %d) failed when sending statuscheck.",
                        sIP.c_str(), iPort);
                return -1;
        }

        //发送消息
        char sMsgBuff[MsgHeadLen];
        sMsgBuff[0] = CProtocolMng::Version1;
        sMsgBuff[1] = CProtocolMng::ProtocolCheckStatus;
        
        if (0 != insClientSocket.sendMsg(sMsgBuff, MsgHeadLen, iMsgTimeout))  //  send 1.1
        {
                 g_insLogMng.error("Write msg statuscheck to socket failed when statuscheck.");
                 insClientSocket.closeSocket();
                return -2;
        }

        //接受响应消息
        char sResponseCode;
        char sTmpStr[1024];
        if (0 == insClientSocket.receiveMsg(sTmpStr, 1, 2*iMsgTimeout))  //为了确保一下成功,等待时间用2倍
        {
                sResponseCode = sTmpStr[0];
                if (CProtocolMng::Success == sResponseCode || CProtocolMng::checkOK == sResponseCode)
                {
                }
                else
                {
                         g_insLogMng.error("Recv the failed response");
                         insClientSocket.closeSocket();
                         return -3;

                }
        }
        else
        {
                g_insLogMng.error("Recv the response failed.");
                insClientSocket.closeSocket();
                return -4;
        }
        g_insLogMng.normal("Check status ok.");
        insClientSocket.closeSocket();
        return 0;

#if 0
        Magic_TcpConnector TcpConn;
        Magic_SocketAddr SockAddr(sIP, (unsigned short)iPort);
        char sMsgBuff[2];
        sMsgBuff[0] = CProtocolMng::Version1;
        sMsgBuff[1] = CProtocolMng::ProtocolCheckStatus;
        if(TcpConn.Connect_tm(SockAddr,10) < 0)
        {
                printf("connect error\n");
                return -1;   
        }     
        if(TcpConn.Write_n(sMsgBuff, 2, 10) != 2)
        {
                        printf("write error\n");
                 TcpConn.Close();
        	return -1;
        }

        if(TcpConn.Read_n(sMsgBuff,1,10) != 1)
        {
                printf("read error\n");
         TcpConn.Close();
        	return -1;
        }

        printf("Result:%x\n", sMsgBuff[0]);
        TcpConn.Close();
        
        return 0;
#endif        

}

//和SendOneDutyMsg的不同点是，这个函数不依赖配置文件，所有信息都是在外面赋值
//为了省事，暂时直接拷贝过来函数而已
int sendSingleDutyMsg(CSendDuty insSendingDuty)
{
        int iMsgTimeout = 10;  //消息超时时间
        int iFileTimeout = 20;  //文件超时时间
        
        CTCPSocket  insClientSocket(insSendingDuty.m_sServerIP, insSendingDuty.m_iServerPort);
        if ( 0 != insClientSocket.connectServer())
        {
                g_insLogMng.error("Connect to server failed when sending duty(%s).",
                        insSendingDuty.getPrintInfo().c_str());
                return -1;
        }
        //消息缓冲区和文件暂存缓冲区
        char sMsgBuff[MsgBufferSize + 1];
        char sFileBuff[WriteToFileSize + 1];

        //发送消息头信息
        sMsgBuff[0] = CProtocolMng::Version1;
        sMsgBuff[1] = insSendingDuty.m_iProtocolType;
        
        if (0 != insClientSocket.sendMsg(sMsgBuff, MsgHeadLen, iMsgTimeout))  //  send 1.1
        {
                 g_insLogMng.error("Write msg head data to socket failed when sending duty(%s).",
                        insSendingDuty.getPrintInfo().c_str());
                 insClientSocket.closeSocket();
                return -2;
        }

        //根据不同的协议，分别进行不同的处理
        CProtocolMng::ResponseCode eLastResponseCode = CProtocolMng::Success;
        switch(insSendingDuty.m_iProtocolType)
        {        
        case CProtocolMng::ProtocolUploadFile:
        {
                //拼装并发送消息的info信息
                CFileUploadMsg insFileUploadMsg;
                strcpy(insFileUploadMsg.m_sSiteName, insSendingDuty.m_sSiteName);
                strcpy(insFileUploadMsg.m_sDestPathFile, insSendingDuty.m_sDestPathFile);
                strcpy(insFileUploadMsg.m_sSendedTime,  insSendingDuty.m_sSendedTime);
                if (1 == g_iFileType)  //filename
                {
                        insFileUploadMsg.m_cDataType = CFileUploadMsg::FileName;
                        insFileUploadMsg.m_iDataLength = htonl(strlen(insSendingDuty.m_sLocalPathFile));
                        if (0 != insClientSocket.sendMsg((char*)(&insFileUploadMsg), sizeof(CFileUploadMsg), iMsgTimeout))
                        {
                                g_insLogMng.error("Write FileUploadMsg to socket failed when sending duty(%s).",
                                        insSendingDuty.getPrintInfo().c_str());
                                eLastResponseCode = CProtocolMng::ServicePause;
                                break;
                        }
                        else
                        {
                                if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                                {
                                        CProtocolMng::ResponseCode eResponseCode;
                                        if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
                                        {
                                                g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,but recv response timeout.",
                                                        insSendingDuty.getPrintInfo().c_str());                                        
                                                eLastResponseCode = CProtocolMng::ServicePause;
                                                break;
                                        }
                                        if (CProtocolMng::CancelDuty == eResponseCode)
                                        {
                                                eLastResponseCode = CProtocolMng::Success;
                                                g_insLogMng.normal("Send Duty(%s)'s FileUploadMsg to socket,recv CancelDuty responecode.",
                                                        insSendingDuty.getPrintInfo().c_str());
                                                break;
                                        }
                                        else if (CProtocolMng::Success == eResponseCode)
                                        {
                                                eLastResponseCode = CProtocolMng::Success;
                                                //继续
                                        }
                                        else //失败
                                        {
                                                eLastResponseCode = eResponseCode;
                                                g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,recv %x responecode.",
                                                                        insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                                break;
                                        }
                                }
                        }
                        
                        if (0 != insClientSocket.sendMsg(insSendingDuty.m_sLocalPathFile, strlen(insSendingDuty.m_sLocalPathFile), iMsgTimeout))
                        {
                                g_insLogMng.error("Write FileUploadMsg data to socket failed when sending duty(%s).",
                                        insSendingDuty.getPrintInfo().c_str());
                                eLastResponseCode = CProtocolMng::ServicePause;
                                break;
                        }
                        else 
                                if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                        {
                                CProtocolMng::ResponseCode eResponseCode;
                                if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
                                {
                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,but recv response timeout.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        eLastResponseCode = CProtocolMng::ServicePause;
                                        break;
                                }
                                if (CProtocolMng::CancelDuty == eResponseCode)
                                {
                                        eLastResponseCode = CProtocolMng::Success;
                                        g_insLogMng.normal("Send Duty(%s)'s FileUploadMsg to socket,recv CancelDuty responecode.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        break;
                                }
                                else if (CProtocolMng::Success == eResponseCode)
                                {
                                        eLastResponseCode = CProtocolMng::Success;
                                        //继续
                                }
                                else //失败
                                {
                                        eLastResponseCode = eResponseCode;
                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,recv %x responecode.",
                                                                insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                        break;
                                }
                        }
                        
                } //else 1 filedata
                else
                {
                        //生成保存的文件名称和路径(前面已经判断过了，站点到这儿已经存在)
                        if (1 == CComFun::isDirectory(insSendingDuty.m_sLocalPathFile))
                        {
                                 g_insLogMng.error("%s is a directory where send the duty(%s).",
                                        insSendingDuty.m_sLocalPathFile,
                                        insSendingDuty.getPrintInfo().c_str());
                                 eLastResponseCode = CProtocolMng::ServicePause;
                                 break;
                        }
                        FILE *fp = fopen(insSendingDuty.m_sLocalPathFile, "r");
                        if (NULL == fp)
                        {
                                 g_insLogMng.error("Open %s failed where send the duty(%s).",
                                        insSendingDuty.m_sLocalPathFile,
                                        insSendingDuty.getPrintInfo().c_str());
                                 eLastResponseCode = CProtocolMng::ServicePause;
                                 break;
                        }
                        
                        //计算一下文件的长度
                        fseek(fp,0,SEEK_END); 
                        long iFileSize = ftell(fp);
                        fseek(fp,0,SEEK_SET);

                        //拼装并发送消息的info信息
                        insFileUploadMsg.m_cDataType = CFileUploadMsg::FileData;
                        insFileUploadMsg.m_iDataLength = htonl(iFileSize);
                        if (0 != insClientSocket.sendMsg((char*)(&insFileUploadMsg), sizeof(CFileUploadMsg), iMsgTimeout))
                        {
                                g_insLogMng.error("Write FileUploadMsg to socket failed when sending duty(%s).",
                                       insSendingDuty.getPrintInfo().c_str());
                                fclose(fp);
                                eLastResponseCode = CProtocolMng::ServicePause;
                                break;
                         }
                        else if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                        {
                                CProtocolMng::ResponseCode eResponseCode;
                                if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
                                {
                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,but recv response timeout.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        eLastResponseCode = CProtocolMng::ServicePause;
                                        break;
                                }
                                if (CProtocolMng::CancelDuty == eResponseCode)
                                {
                                        eLastResponseCode = CProtocolMng::Success;
                                        g_insLogMng.normal("Send Duty(%s)'s FileUploadMsg to socket,recv CancelDuty responecode.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        break;
                                }
                                else if (CProtocolMng::Success == eResponseCode)
                                {
                                        eLastResponseCode = CProtocolMng::Success;
                                        //继续
                                }
                                else //失败
                                {
                                        eLastResponseCode = eResponseCode;
                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,recv %x responecode.",
                                                                insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                        break;
                                }
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
                                        g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode3, 
                                                "Begin to send the msg data stream block");        
                                        if( 0 !=  insClientSocket.sendMsg(sMsgBuff, iSendLen, iFileTimeout))
                                        {
                                                g_insLogMng.error("Send the duty(%s) data block failed.", 
                                                                insSendingDuty.getPrintInfo().c_str());
                                                ix = 1;
                                                break;
                                        }
                                        else if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                                        {
                                                CProtocolMng::ResponseCode eResponseCode;
                                                if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
                                                {
                                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,but recv response timeout.",
                                                                insSendingDuty.getPrintInfo().c_str());
                                                        ix = 1;
                                                        eLastResponseCode = CProtocolMng::ServicePause;
                                                        break;
                                                }
                                                if (CProtocolMng::CancelDuty == eResponseCode)
                                                {
                                                        eLastResponseCode = CProtocolMng::Success;
                                                        g_insLogMng.normal("Send Duty(%s)'s FileUploadMsg to socket,recv CancelDuty responecode.",
                                                                insSendingDuty.getPrintInfo().c_str());
                                                         ix = 1;
                                                        break;
                                                }
                                                else if (CProtocolMng::Success == eResponseCode)
                                                {
                                                        eLastResponseCode = CProtocolMng::Success;
                                                        //继续
                                                }
                                                else //失败
                                                {
                                                        eLastResponseCode = eResponseCode;
                                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,recv %x responecode.",
                                                                                insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                                         ix = 1;
                                                        break;
                                                }
                                        }
                                        iHadSendLen = iHadSendLen + iSendLen;
                                        iReadLen = iReadLen - iSendLen;
                                }  
                                if ( 1== ix)
                                {
                                        break;
                                }
                        }
                        fclose(fp);
                }
                break;
        }
        case CProtocolMng::ProtocolDelFile:
        {
                //拼装并发送消息的info信息
                CFileDelMsg insFileDelMsg;
                strcpy(insFileDelMsg.m_sSiteName,  insSendingDuty.m_sSiteName);
                strcpy(insFileDelMsg.m_sDestPathFile, insSendingDuty.m_sDestPathFile);
                strcpy(insFileDelMsg.m_sSendedTime, insSendingDuty.m_sSendedTime);
                if (0 != insClientSocket.sendMsg((char*)(&insFileDelMsg), sizeof(CFileDelMsg), iMsgTimeout))
                {
                        g_insLogMng.error("Write FileUploadMsg to socket failed when sending duty(%s).",
                               insSendingDuty.getPrintInfo().c_str());
                        eLastResponseCode = CProtocolMng::ServicePause;
                        break;
                }
                else if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                {
                        CProtocolMng::ResponseCode eResponseCode;
                        if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
                        {
                                g_insLogMng.error("Send Duty(%s)'s FileDelMsg to socket,but recv response timeout.",
                                        insSendingDuty.getPrintInfo().c_str());
                                eLastResponseCode = CProtocolMng::ServicePause;
                                break;
                        }
                        if (CProtocolMng::CancelDuty == eResponseCode)
                        {
                                eLastResponseCode = CProtocolMng::Success;
                                g_insLogMng.normal("Send Duty(%s)'s FileDelMsg to socket,recv CancelDuty responecode.",
                                        insSendingDuty.getPrintInfo().c_str());
                                break;
                        }
                        else if (CProtocolMng::Success == eResponseCode)
                        {
                                eLastResponseCode = CProtocolMng::Success;
                                break;
                        }
                        else //失败
                        {
                                eLastResponseCode = eResponseCode;
                                g_insLogMng.error("Send Duty(%s)'s FileDelMsg to socket,recv %x responecode.",
                                                        insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                break;
                        }
                }
                
                break;
        }
        default:
        {
                break;
        }
        }

        //当采用最后确认协议时,需要最后接收一下消息
        if (0 == g_insConfigMng.m_tComCfg.m_iProtocolType)
        {
                CProtocolMng::ResponseCode eResponseCode;
                if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
                {
                        g_insLogMng.error("Send Duty(%s) recv response msg timeout.",
                                insSendingDuty.getPrintInfo().c_str());
                        eLastResponseCode = CProtocolMng::ServicePause;
                }
                else
                {
                        if (CProtocolMng::CancelDuty == eResponseCode)
                        {
                                eLastResponseCode = CProtocolMng::Success;
                                g_insLogMng.normal("Send Duty(%s) recv CancelDuty responecode.",
                                        insSendingDuty.getPrintInfo().c_str());
                        }
                        else if (CProtocolMng::Success == eResponseCode)
                        {
                                eLastResponseCode = CProtocolMng::Success;
                        }
                        else //失败
                        {
                                eLastResponseCode = eResponseCode;
                                g_insLogMng.error("Send Duty(%s) recv %x responecode.",
                                                        insSendingDuty.getPrintInfo().c_str(), eLastResponseCode);
                        }
                }
        }

        //接受响应消息
        if (CProtocolMng::Success != eLastResponseCode)
        {
                //g_insLogMng.error("Send the duty(%s) failed, because recv %x reponsecode",
                //         insSendingDuty.getPrintInfo().c_str(), eLastResponseCode);
                 insClientSocket.closeSocket();
                 return -4;

        }        
        //测试环境,就写个normal日志
        g_insLogMng.normal("Deal with Duty(%s) successfully.",  insSendingDuty.getPrintInfo().c_str());
        insClientSocket.closeSocket();
        return 0;
}


int sendOneDutyMsg(CSendDuty insSendingDuty)
{
        //在insSendDuty 中取出IP和端口，进行连接和发送
        //注意每次要判断一下，是否进行了取消操作
        int iMsgTimeout = g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout;  //消息超时时间
        int iFileTimeout = g_insConfigMng.m_tComCfg.m_iRecvFileTimeout;  //文件超时时间

        strcpy(insSendingDuty.m_sServerIP, g_insConfigMng.m_tComCfg.m_sServerIP);
        if (0 == strcmp(insSendingDuty.m_sServerIP, "ALL") || 0 == strcmp(insSendingDuty.m_sServerIP, "ALL"))
        {
                strcpy(insSendingDuty.m_sServerIP, "127.0.0.1");
        }
        insSendingDuty.m_iServerPort = g_insConfigMng.m_tComCfg.m_iServerPort;
        
        CTCPSocket  insClientSocket(insSendingDuty.m_sServerIP, insSendingDuty.m_iServerPort);
        if ( 0 != insClientSocket.connectServer())
        {
                g_insLogMng.error("Connect to server failed when sending duty(%s).",
                        insSendingDuty.getPrintInfo().c_str());
                return -1;
        }
        //消息缓冲区和文件暂存缓冲区
        char sMsgBuff[MsgBufferSize + 1];
        char sFileBuff[WriteToFileSize + 1];

        //发送消息头信息
        sMsgBuff[0] = CProtocolMng::Version1;
        sMsgBuff[1] = insSendingDuty.m_iProtocolType;
        
        if (0 != insClientSocket.sendMsg(sMsgBuff, MsgHeadLen, iMsgTimeout))  //  send 1.1
        {
                 g_insLogMng.error("Write msg head data to socket failed when sending duty(%s).",
                        insSendingDuty.getPrintInfo().c_str());
                 insClientSocket.closeSocket();
                return -2;
        }

        //根据不同的协议，分别进行不同的处理
        CProtocolMng::ResponseCode eLastResponseCode = CProtocolMng::Success;
        switch(insSendingDuty.m_iProtocolType)
        {        
        case CProtocolMng::ProtocolUploadFile:
        {
                //拼装并发送消息的info信息
                CFileUploadMsg insFileUploadMsg;
                strcpy(insFileUploadMsg.m_sSiteName, insSendingDuty.m_sSiteName);
                strcpy(insFileUploadMsg.m_sDestPathFile, insSendingDuty.m_sDestPathFile);
                strcpy(insFileUploadMsg.m_sSendedTime,  insSendingDuty.m_sSendedTime);
                if (1 == g_iFileType)  //filename
                {
                        insFileUploadMsg.m_cDataType = CFileUploadMsg::FileName;
                        insFileUploadMsg.m_iDataLength = htonl(strlen(insSendingDuty.m_sLocalPathFile));
                        if (0 != insClientSocket.sendMsg((char*)(&insFileUploadMsg), sizeof(CFileUploadMsg), iMsgTimeout))
                        {
                                g_insLogMng.error("Write FileUploadMsg to socket failed when sending duty(%s).",
                                        insSendingDuty.getPrintInfo().c_str());
                                eLastResponseCode = CProtocolMng::ServicePause;
                                break;
                        }
                        else
                        {
                                if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                                {
                                        CProtocolMng::ResponseCode eResponseCode;
                                        if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
                                        {
                                                g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,but recv response timeout.",
                                                        insSendingDuty.getPrintInfo().c_str());                                        
                                                eLastResponseCode = CProtocolMng::ServicePause;
                                                break;
                                        }
                                        if (CProtocolMng::CancelDuty == eResponseCode)
                                        {
                                                eLastResponseCode = CProtocolMng::Success;
                                                g_insLogMng.normal("Send Duty(%s)'s FileUploadMsg to socket,recv CancelDuty responecode.",
                                                        insSendingDuty.getPrintInfo().c_str());
                                                break;
                                        }
                                        else if (CProtocolMng::Success == eResponseCode)
                                        {
                                                eLastResponseCode = CProtocolMng::Success;
                                                //继续
                                        }
                                        else //失败
                                        {
                                                eLastResponseCode = eResponseCode;
                                                g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,recv %x responecode.",
                                                                        insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                                break;
                                        }
                                }
                        }
                        
                        if (0 != insClientSocket.sendMsg(insSendingDuty.m_sLocalPathFile, strlen(insSendingDuty.m_sLocalPathFile), iMsgTimeout))
                        {
                                g_insLogMng.error("Write FileUploadMsg data to socket failed when sending duty(%s).",
                                        insSendingDuty.getPrintInfo().c_str());
                                eLastResponseCode = CProtocolMng::ServicePause;
                                break;
                        }
                        else 
                                if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                        {
                                CProtocolMng::ResponseCode eResponseCode;
                                if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
                                {
                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,but recv response timeout.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        eLastResponseCode = CProtocolMng::ServicePause;
                                        break;
                                }
                                if (CProtocolMng::CancelDuty == eResponseCode)
                                {
                                        eLastResponseCode = CProtocolMng::Success;
                                        g_insLogMng.normal("Send Duty(%s)'s FileUploadMsg to socket,recv CancelDuty responecode.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        break;
                                }
                                else if (CProtocolMng::Success == eResponseCode)
                                {
                                        eLastResponseCode = CProtocolMng::Success;
                                        //继续
                                }
                                else //失败
                                {
                                        eLastResponseCode = eResponseCode;
                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,recv %x responecode.",
                                                                insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                        break;
                                }
                        }
                        
                } //else 1 filedata
                else
                {
                        //生成保存的文件名称和路径(前面已经判断过了，站点到这儿已经存在)
                        if (1 == CComFun::isDirectory(insSendingDuty.m_sLocalPathFile))
                        {
                                 g_insLogMng.error("%s is a directory where send the duty(%s).",
                                        insSendingDuty.m_sLocalPathFile,
                                        insSendingDuty.getPrintInfo().c_str());
                                 eLastResponseCode = CProtocolMng::ServicePause;
                                 break;
                        }
                        FILE *fp = fopen(insSendingDuty.m_sLocalPathFile, "r");
                        if (NULL == fp)
                        {
                                 g_insLogMng.error("Open %s failed where send the duty(%s).",
                                        insSendingDuty.m_sLocalPathFile,
                                        insSendingDuty.getPrintInfo().c_str());
                                 eLastResponseCode = CProtocolMng::ServicePause;
                                 break;
                        }
                        
                        //计算一下文件的长度
                        fseek(fp,0,SEEK_END); 
                        long iFileSize = ftell(fp);
                        fseek(fp,0,SEEK_SET);

                        //拼装并发送消息的info信息
                        insFileUploadMsg.m_cDataType = CFileUploadMsg::FileData;
                        insFileUploadMsg.m_iDataLength = htonl(iFileSize);
                        if (0 != insClientSocket.sendMsg((char*)(&insFileUploadMsg), sizeof(CFileUploadMsg), iMsgTimeout))
                        {
                                g_insLogMng.error("Write FileUploadMsg to socket failed when sending duty(%s).",
                                       insSendingDuty.getPrintInfo().c_str());
                                fclose(fp);
                                eLastResponseCode = CProtocolMng::ServicePause;
                                break;
                         }
                        else if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                        {
                                CProtocolMng::ResponseCode eResponseCode;
                                if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
                                {
                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,but recv response timeout.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        eLastResponseCode = CProtocolMng::ServicePause;
                                        break;
                                }
                                if (CProtocolMng::CancelDuty == eResponseCode)
                                {
                                        eLastResponseCode = CProtocolMng::Success;
                                        g_insLogMng.normal("Send Duty(%s)'s FileUploadMsg to socket,recv CancelDuty responecode.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        break;
                                }
                                else if (CProtocolMng::Success == eResponseCode)
                                {
                                        eLastResponseCode = CProtocolMng::Success;
                                        //继续
                                }
                                else //失败
                                {
                                        eLastResponseCode = eResponseCode;
                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,recv %x responecode.",
                                                                insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                        break;
                                }
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
                                        g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode3, 
                                                "Begin to send the msg data stream block");        
                                        if( 0 !=  insClientSocket.sendMsg(sMsgBuff, iSendLen, iFileTimeout))
                                        {
                                                g_insLogMng.error("Send the duty(%s) data block failed.", 
                                                                insSendingDuty.getPrintInfo().c_str());
                                                ix = 1;
                                                break;
                                        }
                                        else if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                                        {
                                                CProtocolMng::ResponseCode eResponseCode;
                                                if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
                                                {
                                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,but recv response timeout.",
                                                                insSendingDuty.getPrintInfo().c_str());
                                                        ix = 1;
                                                        eLastResponseCode = CProtocolMng::ServicePause;
                                                        break;
                                                }
                                                if (CProtocolMng::CancelDuty == eResponseCode)
                                                {
                                                        eLastResponseCode = CProtocolMng::Success;
                                                        g_insLogMng.normal("Send Duty(%s)'s FileUploadMsg to socket,recv CancelDuty responecode.",
                                                                insSendingDuty.getPrintInfo().c_str());
                                                         ix = 1;
                                                        break;
                                                }
                                                else if (CProtocolMng::Success == eResponseCode)
                                                {
                                                        eLastResponseCode = CProtocolMng::Success;
                                                        //继续
                                                }
                                                else //失败
                                                {
                                                        eLastResponseCode = eResponseCode;
                                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg to socket,recv %x responecode.",
                                                                                insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                                         ix = 1;
                                                        break;
                                                }
                                        }
                                        iHadSendLen = iHadSendLen + iSendLen;
                                        iReadLen = iReadLen - iSendLen;
                                }  
                                if ( 1== ix)
                                {
                                        break;
                                }
                        }
                        fclose(fp);
                }
                break;
        }
        case CProtocolMng::ProtocolDelFile:
        {
                //拼装并发送消息的info信息
                CFileDelMsg insFileDelMsg;
                strcpy(insFileDelMsg.m_sSiteName,  insSendingDuty.m_sSiteName);
                strcpy(insFileDelMsg.m_sDestPathFile, insSendingDuty.m_sDestPathFile);
                strcpy(insFileDelMsg.m_sSendedTime, insSendingDuty.m_sSendedTime);
                if (0 != insClientSocket.sendMsg((char*)(&insFileDelMsg), sizeof(CFileDelMsg), iMsgTimeout))
                {
                        g_insLogMng.error("Write FileUploadMsg to socket failed when sending duty(%s).",
                               insSendingDuty.getPrintInfo().c_str());
                        eLastResponseCode = CProtocolMng::ServicePause;
                        break;
                }
                else if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                {
                        CProtocolMng::ResponseCode eResponseCode;
                        if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
                        {
                                g_insLogMng.error("Send Duty(%s)'s FileDelMsg to socket,but recv response timeout.",
                                        insSendingDuty.getPrintInfo().c_str());
                                eLastResponseCode = CProtocolMng::ServicePause;
                                break;
                        }
                        if (CProtocolMng::CancelDuty == eResponseCode)
                        {
                                eLastResponseCode = CProtocolMng::Success;
                                g_insLogMng.normal("Send Duty(%s)'s FileDelMsg to socket,recv CancelDuty responecode.",
                                        insSendingDuty.getPrintInfo().c_str());
                                break;
                        }
                        else if (CProtocolMng::Success == eResponseCode)
                        {
                                eLastResponseCode = CProtocolMng::Success;
                                break;
                        }
                        else //失败
                        {
                                eLastResponseCode = eResponseCode;
                                g_insLogMng.error("Send Duty(%s)'s FileDelMsg to socket,recv %x responecode.",
                                                        insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                break;
                        }
                }
                
                break;
        }
        default:
        {
                break;
        }
        }

        //当采用最后确认协议时,需要最后接收一下消息
        if (0 == g_insConfigMng.m_tComCfg.m_iProtocolType)
        {
                CProtocolMng::ResponseCode eResponseCode;
                if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
                {
                        g_insLogMng.error("Send Duty(%s) recv response msg timeout.",
                                insSendingDuty.getPrintInfo().c_str());
                        eLastResponseCode = CProtocolMng::ServicePause;
                }
                else
                {
                        if (CProtocolMng::CancelDuty == eResponseCode)
                        {
                                eLastResponseCode = CProtocolMng::Success;
                                g_insLogMng.normal("Send Duty(%s) recv CancelDuty responecode.",
                                        insSendingDuty.getPrintInfo().c_str());
                        }
                        else if (CProtocolMng::Success == eResponseCode)
                        {
                                eLastResponseCode = CProtocolMng::Success;
                        }
                        else //失败
                        {
                                eLastResponseCode = eResponseCode;
                                g_insLogMng.error("Send Duty(%s) recv %x responecode.",
                                                        insSendingDuty.getPrintInfo().c_str(), eLastResponseCode);
                        }
                }
        }

        //接受响应消息
        if (CProtocolMng::Success != eLastResponseCode)
        {
                //g_insLogMng.error("Send the duty(%s) failed, because recv %x reponsecode",
                //         insSendingDuty.getPrintInfo().c_str(), eLastResponseCode);
                 insClientSocket.closeSocket();
                 return -4;

        }        
        //测试环境,就写个normal日志
        g_insLogMng.normal("Deal with Duty(%s) successfully.",  insSendingDuty.getPrintInfo().c_str());
        insClientSocket.closeSocket();
        return 0;
}

int sendServerAddMsg(CServerAddMsg& serverinfo)
{
        //在insSendDuty 中取出IP和端口，进行连接和发送
        //注意每次要判断一下，是否进行了取消操作
        int iMsgTimeout = g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout;  //消息超时时间
        if (0 == strcmp(g_insConfigMng.m_tComCfg.m_sServerIP, "ALL") || 0 == strcmp(g_insConfigMng.m_tComCfg.m_sServerIP, "ALL"))
        {
                strcpy(g_insConfigMng.m_tComCfg.m_sServerIP, "127.0.0.1");
        }        
        CTCPSocket  insClientSocket(g_insConfigMng.m_tComCfg.m_sServerIP, 
                g_insConfigMng.m_tComCfg.m_iServerPort);
        if ( 0 != insClientSocket.connectServer())
        {
                g_insLogMng.error("Connect to server failed when AddServer(%s).",
                        serverinfo.getPrintInfo().c_str());
                return -1;
        }
        //消息缓冲区和文件暂存缓冲区
        char sMsgBuff[2 + 1];

        //发送消息头信息
        sMsgBuff[0] = CProtocolMng::Version1;
        sMsgBuff[1] = CProtocolMng::ProtocolAddServer;
        
        if (0 != insClientSocket.sendMsg(sMsgBuff, MsgHeadLen, iMsgTimeout))  //  send 1.1
        {
                 g_insLogMng.error("Write msg head data to socket failed when AddServer(%s).",
                        serverinfo.getPrintInfo().c_str());
                 insClientSocket.closeSocket();
                return -2;
        }
       
       //拼装并发送消息的info信息
       serverinfo.m_iPort = htonl(serverinfo.m_iPort);
       if (0 != insClientSocket.sendMsg((char*)(&serverinfo), sizeof(CServerAddMsg), iMsgTimeout))
       {
                g_insLogMng.error("Write ServerAddMsg to socket failed when AddServer(%s).",
                      serverinfo.getPrintInfo().c_str());
                insClientSocket.closeSocket();
                return -3;
       }       

       CProtocolMng::ResponseCode eResponseCode;
       if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
       {
               g_insLogMng.error("AddServer(%s) recv response msg timeout.",
                       serverinfo.getPrintInfo().c_str());
       }
       else
       {
               if (CProtocolMng::Success == eResponseCode)
               {
                        g_insLogMng.normal("Deal with AddServer(%s) successfully.",  serverinfo.getPrintInfo().c_str());
               }
               else //失败
               {
                        g_insLogMng.error("AddServer(%s) recv %x responecode.",
                                               serverinfo.getPrintInfo().c_str(), eResponseCode);
               }
       }
        
        insClientSocket.closeSocket();
        return 0;
}


int sendServerDelMsg(CServerDelMsg& serverinfo)
{
        //在insSendDuty 中取出IP和端口，进行连接和发送
        //注意每次要判断一下，是否进行了取消操作
        int iMsgTimeout = g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout;  //消息超时时间
        if (0 == strcmp(g_insConfigMng.m_tComCfg.m_sServerIP, "ALL") || 0 == strcmp(g_insConfigMng.m_tComCfg.m_sServerIP, "ALL"))
        {
                strcpy(g_insConfigMng.m_tComCfg.m_sServerIP, "127.0.0.1");
        }  
        CTCPSocket  insClientSocket(g_insConfigMng.m_tComCfg.m_sServerIP, 
                g_insConfigMng.m_tComCfg.m_iServerPort);
        if ( 0 != insClientSocket.connectServer())
        {
                g_insLogMng.error("Connect to server failed when DelServer(%s).",
                        serverinfo.getPrintInfo().c_str());
                return -1;
        }
        //消息缓冲区和文件暂存缓冲区
        char sMsgBuff[2 + 1];

        //发送消息头信息
        sMsgBuff[0] = CProtocolMng::Version1;
        sMsgBuff[1] = CProtocolMng::ProtocolDelServer;
        
        if (0 != insClientSocket.sendMsg(sMsgBuff, MsgHeadLen, iMsgTimeout))  //  send 1.1
        {
                 g_insLogMng.error("Write msg head data to socket failed when DelServer(%s).",
                        serverinfo.getPrintInfo().c_str());
                 insClientSocket.closeSocket();
                return -2;
        }
       
       //拼装并发送消息的info信息
       serverinfo.m_iPort = htonl(serverinfo.m_iPort);
       if (0 != insClientSocket.sendMsg((char*)(&serverinfo), sizeof(CServerDelMsg), iMsgTimeout))
       {
                g_insLogMng.error("Write ServerDelMsg to socket failed when DelServer(%s).",
                      serverinfo.getPrintInfo().c_str());
                insClientSocket.closeSocket();
                return -3;
       }       

       CProtocolMng::ResponseCode eResponseCode;
       if (0 != CProtocolMng::recvResponse(insClientSocket, eResponseCode))
       {
               g_insLogMng.error("DelServer(%s) recv response msg timeout.",
                       serverinfo.getPrintInfo().c_str());
       }
       else
       {
               if (CProtocolMng::Success == eResponseCode)
               {
                        g_insLogMng.normal("Deal with DelServer(%s) successfully.",  serverinfo.getPrintInfo().c_str());
               }
               else //失败
               {
                        g_insLogMng.error("DelServer(%s) recv %x responecode.",
                                               serverinfo.getPrintInfo().c_str(), eResponseCode);
               }
       }
        
        insClientSocket.closeSocket();
        return 0;
}


void* pthread_sendOneDutyMsg(void* p)
{
/*        int iPD = (int)getpid();
        CSendDuty insSendDuty;
        while(1)
        {
                map<int, CSendDuty>::iterator it = g_mapthreadSendDuty.find(iPD);
                if(it != g_mapthreadSendDuty.end())
                {
                        insSendDuty = it->second;
                        g_mapthreadSendDuty.erase(it);
                        break;
                }
                CComFun::sleep_msec(0, 1);
        }
*/
        g_insLogMng.debug("tool", CLogMng::LogMode2, "Child thread begin to work, deal with the duty(%s)",
               ( *((CSendDuty *)p)).getPrintInfo().c_str());
        sendOneDutyMsg(*((CSendDuty *)p));
        return (void*)NULL;
}


void sendMultDutyMsg(int iTimes)
{
        vector<CSendDuty> vecSendDuty;
        string sPathFile = g_sHomeDir + "/dat/testduty.list";
        //---File Format
        //---Head---
        //filetype 
        //null
        //---body mult---
        //Line1:ProtocolType
        //Line2:SiteName
        //Line3:opr Dest Name
        //Line4:local file name
        //Line5: null
        CSendDuty insSendDuty;        
        
        FILE *fp = fopen(sPathFile.c_str(), "r");
        if (NULL == fp)
        {
                g_insLogMng.error("Open file %s failed.", sPathFile.c_str());
                return;
        }
        char sTmpLong[1024 + 1];
        fgets(sTmpLong, 1024, fp);
        CComFun::trimString(sTmpLong, sTmpLong, 0);
        g_iFileType = atoi(sTmpLong);
        fgets(sTmpLong, 1024, fp); //空行
        while(!feof(fp))  //只是测试工具，不在判断格式的正确性，人为保证
        {
                if (NULL == fgets(sTmpLong, 1024, fp))
                        break;
                CComFun::trimString(sTmpLong, sTmpLong, 0);
                int iTmp = atoi(sTmpLong);
                if (1 == iTmp)
                {
                        insSendDuty.m_iProtocolType = CProtocolMng::ProtocolUploadFile;
                }
                else
                {
                        insSendDuty.m_iProtocolType = CProtocolMng::ProtocolDelFile;
                }
                fgets(sTmpLong, 1024, fp);
                CComFun::trimString(sTmpLong, sTmpLong, 0);
                sTmpLong[strlen(sTmpLong) - 1] = '\0';
                strcpy(insSendDuty.m_sSiteName, sTmpLong);

                fgets(sTmpLong, 1024, fp);
                CComFun::trimString(sTmpLong, sTmpLong, 0);
                sTmpLong[strlen(sTmpLong) - 1] = '\0';
                strcpy(insSendDuty.m_sDestPathFile, sTmpLong);

                fgets(sTmpLong, 1024, fp);
                CComFun::trimString(sTmpLong, sTmpLong, 0);
                sTmpLong[strlen(sTmpLong) - 1] = '\0';
                strcpy(insSendDuty.m_sLocalPathFile, sTmpLong);

                fgets(sTmpLong, 1024, fp); //空行

                //add cur datttime
                strcpy(insSendDuty.m_sSendedTime, CComFun::GetMsgCurDateTime().c_str());

                vecSendDuty.push_back(insSendDuty);
        }
        fclose(fp);

        if ( -1 == iTimes)
        {
                iTimes = 10000; //循环也不是无限制的,1万次
        }
                
        while(iTimes > 0)
        {
                for (vector<CSendDuty>::iterator it = vecSendDuty.begin(); it != vecSendDuty.end(); it++)
                {
                        //多线程
                        //pthread_create((pthread_t*)&piD, NULL, pthread_sendOneDutyMsg, (void *)NULL);
                        //g_mapthreadSendDuty[piD] = *it;
                        CSendDuty insSendDuty = *it;
                        g_insLogMng.debug("tool", CLogMng::LogMode2, "Begin to send duty(%s)",
                                insSendDuty.getPrintInfo().c_str());
                        //pthread_create((pthread_t*)&piD, NULL, pthread_sendOneDutyMsg, (void *)(&insSendDuty));
                        CComFun::sleep_msec(0, 50);
                        pid_t childpid = fork();
                        if( childpid < 0 )              // failed
                        {
                                g_insLogMng.error("Fork child process failed.");
                                continue;
                        }
                        else if( childpid == 0 )  // child
                        {
                                sendOneDutyMsg(insSendDuty);
                                exit(0);
                        }    // else parent   
                }
                iTimes--;
        }
        return;
}


void convertFailDutyFileD2T(int iSeq)
{
        char sTmp[10];
        sprintf(sTmp, "%04d", iSeq);
        string sPathName1 = g_sHomeDir+"/dat/failduty." + sTmp;
        string sPathName2 = sPathName1 + ".txt";

        //写入文件
        FILE *fp1 = fopen(sPathName1.c_str(), "r");
        FILE *fp2 = fopen(sPathName2.c_str(), "w");
        if (NULL == fp1 || NULL == fp2)
        {
                return; 
        }

        CSendDuty insFailDuty;
        unsigned int iTmpSize = sizeof(CSendDuty);
        while(!feof(fp1))
        {
                if (1 != fread((char*)&insFailDuty, iTmpSize, 1, fp1))
                {
                        break;
                }
                fputs((insFailDuty.getPrintInfo() + "\n").c_str(), fp2);  //写入文件
        }

        fclose(fp1);
        fclose(fp2);
        
        return;
}


int main(int argc, char* argv[])
{
        //程序名称 newoctopusd
        g_sProgramName = "octopustool";

        // [2]octopusd的主目录
        if (NULL == getenv("NEWOCTOPUSD_HOME"))
        {
                g_sHomeDir = "/usr/local/newoctopusd";
        }
        else
        {
                g_sHomeDir =  getenv("NEWOCTOPUSD_HOME");
        }

        //检查目录是否存在
        struct stat st1;
        if(!((stat(g_sHomeDir.c_str(), &st1)== 0) && S_ISDIR(st1.st_mode)))
        {
                printf("home dir %s does not exist, %s start failed.\n", g_sHomeDir.c_str(), g_sProgramName.c_str());
                exit(0);
        }

        // [3]日志模块
        if (0 != g_insLogMng.init(g_sProgramName))
        {
                printf( "Create log file failed, %s start failed.\n", g_sProgramName.c_str());
                exit(0);
        }

        //[4] 其实这个并不是所有功能都需要，但是所有的写debug日志都需要，所以暂且连上
        if (0 != g_insMaintain.connect())
        {
               printf("\n\nError:Connect to share memory failed.\n\n");
               exit(-1);
        }

        signal( SIGIO, SIG_IGN);
        signal( SIGPIPE, SIG_IGN);
        signal( SIGTERM, sighandler);
        signal( SIGHUP, sighandler);
        signal( SIGCHLD, sighandler);
        signal( SIGUSR1, sighandler);
        signal( SIGINT, sighandler);
        signal(SIGALRM, sighandler);  
        

        //命令行，支持2层
        int iArg1;
        int iArg2;
        int iArg3;
        if (2 == argc) //none
        {
                if (0 == strcmp(argv[1], "-h") || 0 == strcmp(argv[1], "--help") ) 
                {
                        printf("octopustool:\n\tmenu\n\tArg1 Arg2 ...[command line,according to menu]\n\t-h\n\t--help\n");
                        exit(0);
                }
                else if (0 == strcmp(argv[1], "menu")) 
                {                        
                }
                else
                {
                        exit(0);
                }
        }
        else if (3 == argc) 
        {
                iArg1 = atoi(argv[1]);
                iArg2 = atoi(argv[2]);
                if (6 == iArg1)  //odditional function
                {
                        if (1 == iArg2)
                        {
                                if (0 != g_insMaintain.connect())
                                {
                                        printf("\n\nError:Connect to share memory failed.\n\n");
                                        exit(-1);
                                }
                                g_insMaintain.printScreen();
                        }
                        //else if (2 == iArg2)  比较危险,暂不提供
                        //{
                        //       if (0 != g_insMaintain.delShareMem())
                        //        {
                        //                printf("\n\nError:Delete share memory failed.\n\n");
                        //                exit(-1);
                        //        }
                        //}
                        else if (4 == iArg2)
                        {
                                if (0 != g_insMaintain.connect())
                                {
                                        printf("\n\nError:Connect to share memory failed.\n\n");
                                        exit(-1);
                                }
                                g_insMaintain.setFlag(CMaintain::PrintThread2Pid, 1);    
                        }
                        else if (7 == iArg2)  //有可能用在socket强制关闭功能里面
                        {
                                if (0 != g_insMaintain.connect())
                                {
                                        printf("\n\nError:Connect to share memory failed.\n\n");
                                        exit(-1);
                                }
                                g_insMaintain.setFlag(CMaintain::ForceCloseAllSocketFD, 1);    
                        }
                        
                }
                else if ( 2 == iArg1) //config
                {
                        if (0 != g_insMaintain.connect())
                        {
                                printf("\n\nError:Connect to share memory failed.\n\n");
                                exit(-1);
                        }
                        if (1 == iArg2)
                        {
                                g_insMaintain.setFlag(CMaintain::PrintCfg, 1);                                
                        }
                        else if (2 == iArg2)
                        {
                                 g_insMaintain.setFlag(CMaintain::RefreshCommonCfg, 1); 
                        }
                        else if (3 == iArg2)
                        {
                                g_insMaintain.setFlag(CMaintain::RefreshSiteInfo, 1); 
                        }
                        else if (4 == iArg2)
                        {
                                g_insMaintain.setFlag(CMaintain::RefreshLogModule, 1); 
                        }
                        
                        exit(0);
                }
                else if (5 == iArg1)  //exit system
                {
                        int iRet = 0;
                        if ('Y' != argv[2][0] && 'y' != argv[2][0] )
                        {
                                iRet = 0;
                        }
                        else
                        {
                                iRet = 1;
                        }
                        if (0 != g_insMaintain.connect())
                        {
                                printf("\n\nError:Connect to share memory failed.\n\n");
                                exit(-1);
                        }
                        g_insMaintain.setFlag(CMaintain::SaveDutyQueueWhenExit, iRet);
                        g_insMaintain.setFlag(CMaintain::ExitState, 1);                       
                        
                }
                if (3 == iArg1)  //queue
                {
                        if (0 != g_insMaintain.connect())
                        {
                                printf("\n\nError:Connect to share memory failed.\n\n");
                                exit(-1);
                        }
                        if (3 == iArg2)
                        {
                                g_insMaintain.setFlag(CMaintain::PrintServerStatus, 1);                                
                        }
                        else if (2 == iArg2)
                        {      
                                g_insMaintain.setFlag(CMaintain::PrintQCurInfo, 1); 
                        }
                        else if (1 == iArg2)
                        {
                                g_insMaintain.setFlag(CMaintain::ReloadFailDuty, 1); 
                        }
                        else if (6 == iArg2)
                        {
                                g_insMaintain.setFlag(CMaintain::QueueSafeMode, 1);  //直接置成1
                        }
                }                
                exit(0);
        }
        else if (4 == argc)
        {
                iArg1 = atoi(argv[1]);
                iArg2 = atoi(argv[2]); 
                iArg3 = atoi(argv[3]);
                if (1 == iArg1)  //log
                {                        
                        if (0 != g_insMaintain.connect())
                        {
                                printf("\n\nError:Connect to share memory failed.\n\n");
                                exit(-1);
                        }
                        if (1 == iArg2)
                        {
                                g_insMaintain.setFlag(CMaintain::LogMode, iArg3 -1);
                        }
                        else if (2 == iArg2)
                        {
                                g_insMaintain.setFlag(CMaintain::LogModule, iArg3 - 1);
                        }
                }
                else if (6 == iArg1)
                {
                        if (3 == iArg2)
                        {
                                CMobileInterface::SendSMSMsg(argv[3], argv[4]);
                        }
                }
                exit(0);
        }
        else if (5 == argc)
        {
                iArg1 = atoi(argv[1]);
                iArg2 = atoi(argv[2]); 
                if (6 == iArg1)
                {
                        if (3 == iArg2)
                        {
                                CMobileInterface::SendSMSMsg(argv[3], argv[4]);
                        }
                }
                exit(0);
        }
        else if (6 == argc)
        {
                iArg1 = atoi(argv[1]);
                iArg2 = atoi(argv[2]); 
                iArg3 = atoi(argv[3]);
                if (4 == iArg1)
                {
                        if (0 != g_insConfigMng.init())
                        {
                                printf("\n\nOK::read config file failed.\n\n");
                                exit(0);
                        }
                        if (1 == iArg2)
                        {                        
                                if (2 == iArg3)
                                {
                                        CSendDuty insSendDuty;
                                        insSendDuty.m_iProtocolType = CProtocolMng::ProtocolDelFile;
                                        strcpy(insSendDuty.m_sSiteName, argv[4]);
                                        strcpy(insSendDuty.m_sDestPathFile, argv[5]);
                                        strcpy(insSendDuty.m_sSendedTime, CComFun::GetMsgCurDateTime().c_str());
                                        int iRet = sendOneDutyMsg(insSendDuty);
                                        printf("\n\nOK::send msg finnished, the result is %d\n\n", iRet);
                                }
                        }
                }
                exit(0);
        }
        else if (7 == argc)   // 2006-05-22 add 参数不做校验，请自行保证
        {
                //octopustool ip port site oppdir_name data/name localdir_name
                CSendDuty insSendDuty;                                                
                insSendDuty.m_iProtocolType = CProtocolMng::ProtocolUploadFile;
                strcpy(insSendDuty.m_sSiteName, argv[3]);
                strcpy(insSendDuty.m_sDestPathFile, argv[4]);
                strcpy(insSendDuty.m_sSendedTime, CComFun::GetMsgCurDateTime().c_str());
                g_iFileType = atoi(argv[5]);
                strcpy(insSendDuty.m_sLocalPathFile, argv[6]);
                strcpy(insSendDuty.m_sServerIP, argv[1]);
                insSendDuty.m_iServerPort = atoi(argv[2]);
                sendSingleDutyMsg(insSendDuty);
                exit(0);
        }
        else if (8 == argc)
        {
                iArg1 = atoi(argv[1]);
                iArg2 = atoi(argv[2]); 
                iArg3 = atoi(argv[3]);
                if (4 == iArg1)
                {
                        if (0 != g_insConfigMng.init())
                        {
                                 printf("\n\nOK::read config file failed.\n\n");
                                 exit(0);
                        }
                        if (1 == iArg2)
                        {                        
                                if (1 == iArg3)
                                {
                                        CSendDuty insSendDuty;
                                        insSendDuty.m_iProtocolType = CProtocolMng::ProtocolUploadFile;
                                        strcpy(insSendDuty.m_sSiteName, argv[4]);
                                        strcpy(insSendDuty.m_sDestPathFile, argv[5]);
                                        strcpy(insSendDuty.m_sSendedTime, CComFun::GetMsgCurDateTime().c_str());
                                        g_iFileType = atoi(argv[6]);
                                        strcpy(insSendDuty.m_sLocalPathFile, argv[7]);
                                        int iRet = sendOneDutyMsg(insSendDuty);
                                        printf("\n\nOK::send msg finnished, the result is %d\n\n", iRet);
                                }
                        }
                }
                exit(0);
        }

        char sTmpStr[128 + 1];
        int iInput;
        // 菜单
        while(1)
        {
                printMenu0_M();
                fgets(sTmpStr, 128, stdin);
                iInput = atoi(sTmpStr);
                if (0 > iInput || 6 < iInput)
                {
                        continue;
                }
                // ========日志管理========
                if (1 == iInput)
                {
                        if (0 != g_insMaintain.connect())
                        {
                                printf("\n\nError:Connect to share memory failed.\n\n");
                                continue;
                        }
                        while(1)
                        {
                                printMenu0_1();
                                fgets(sTmpStr, 128, stdin);
                                iInput = atoi(sTmpStr);
                                if (0 > iInput || 2 < iInput)
                                {
                                        continue;
                                }
                                if (1 == iInput)
                                {
                                        while(1)
                                        {
                                                printMenu0_1_1(g_insMaintain.getFlag(CMaintain::LogMode));
                                                fgets(sTmpStr, 128, stdin);
                                                iInput = atoi(sTmpStr);
                                                if (0 > iInput || 4 < iInput)
                                                {
                                                        continue;
                                                }
                                                if (0 == iInput)
                                                {
                                                        break;
                                                }
                                                else
                                                {
                                                        g_insMaintain.setFlag(CMaintain::LogMode, iInput-1);
                                                }
                                        }
                                }
                                else if  (2 == iInput)
                                {                                        
                                        while(1)
                                        {
                                                printMenu0_1_2(g_insMaintain.getFlag(CMaintain::LogModule), 
                                                        g_insLogMng.m_mapLogModule);
                                                fgets(sTmpStr, 128, stdin);
                                                iInput = atoi(sTmpStr);
                                                if (0 == iInput)
                                                {
                                                        break;
                                                }
                                                else
                                                {
                                                        g_insMaintain.setFlag(CMaintain::LogModule, iInput-1);
                                                }
                                        }
                                }                                
                                else if (0 == iInput)
                                {
                                        break;
                                }
                        }
                }
                // ========配置管理========
                else if (2 == iInput)
                {
                        if (0 != g_insMaintain.connect())
                        {
                                printf("\n\nError:Connect to share memory failed.\n\n");
                                continue;
                        }
                        while(1)
                        {                                
                                printMenu0_2();
                                fgets(sTmpStr, 128, stdin);
                                iInput = atoi(sTmpStr);
                                if (0 > iInput || 4 < iInput)
                                {
                                        continue;
                                }
                                if (0 == iInput)
                                {
                                        break;
                                }
                                else
                                {
                                        printf("This operation will affect the running system, are you sure(Y/N)?");
                                        fgets(sTmpStr, 128, stdin);
                                        if (sTmpStr[0] != 'y' && sTmpStr[0] != 'Y')
                                        {
                                                break;
                                        }
                                        if (1 == iInput)
                                        {
                                                g_insMaintain.setFlag(CMaintain::PrintCfg, 1);
                                                printf("\n\nOK:Please find the result file in dir log/prt\n\n");
                                        }
                                        else if (2 == iInput)
                                        {
                                                g_insMaintain.setFlag(CMaintain::RefreshCommonCfg, 1);
                                        }
                                        else if (3 == iInput)
                                        {
                                                g_insMaintain.setFlag(CMaintain::RefreshSiteInfo, 1);
                                        }
                                        else if (4 == iInput)
                                        {
                                                g_insMaintain.setFlag(CMaintain::RefreshLogModule, 1);
                                                printf("\n\nOK:Please find the result file in dir log/prt\n\n");
                                        }
                                }
                        }
                }
                // ======== 发送任务队列 ========
                else if (3 == iInput)
                {
                        if (0 != g_insMaintain.connect())
                        {
                                printf("\n\nError:Connect to share memory failed.\n\n");
                                continue;
                        }
                        while(1)
                        {                                
                                printMenu0_3();
                                fgets(sTmpStr, 128, stdin);
                                iInput = atoi(sTmpStr);
                                if (0 > iInput || 6 < iInput)
                                {
                                        continue;
                                }
                                if (0 == iInput)
                                {
                                        break;
                                }
                                else
                                {
                                        printf("This operation will affect the running system, are you sure(Y/N)?");
                                        fgets(sTmpStr, 128, stdin);
                                        if (sTmpStr[0] != 'y' && sTmpStr[0] != 'Y')
                                        {
                                                break;
                                        }
                                        if (1 == iInput)
                                        {
                                                printf("Please input the FailDutyFile seq:(-1 ALL, 0~9999)===>");
                                                fgets(sTmpStr, 128, stdin);
                                                int iInput2 = atoi(sTmpStr);
                                                if (-1 > iInput2 || iInput2 > 9999)
                                                {
                                                        printf("\n\nERROR:Input invalid.\n\n");
                                                        break;
                                                }
                                                
                                                if (0 == iInput2)
                                                {
                                                        iInput2 = -2;
                                                }
                                                printf("Your input is %d.\n", iInput2);
                                                g_insMaintain.setFlag(CMaintain::ReloadFailDuty, iInput2);
                                        }
                                        else if (2 == iInput)
                                        {
                                                g_insMaintain.setFlag(CMaintain::PrintQCurInfo, 1);
                                                printf("\n\nOK:Please find the result file in dir log/prt\n\n");
                                        }
                                        else if (3 == iInput)
                                        {
                                                g_insMaintain.setFlag(CMaintain::PrintServerStatus, 1);
                                                printf("\n\nOK:Please find the result file in dir log/prt\n\n");
                                        }
                                        else if (4 == iInput)
                                        {
                                                printf("Please input the FailDutyFile seq:(0~9999)===>");
                                                fgets(sTmpStr, 128, stdin);
                                                int iInput2 = atoi(sTmpStr);
                                                if (0 > iInput2 || iInput2 > 9999)
                                                {
                                                        printf("\n\nERROR:Input invalid.\n\n");
                                                        break;
                                                }
                                                convertFailDutyFileD2T(iInput2);
                                                printf("\n\nOK:Please find the result file in dir /dat\n\n");
                                        }
                                        else if (5 == iInput)
                                        {
                                                int iValue = g_insMaintain.getFlag(CMaintain::ClearDisabledDuty);
                                                int nValue = iValue^1;  //取反
                                                printf("Current ClearDisabledDuty value is %d, Are you change to %d ? (Y/N)\n", 
                                                        iValue, nValue);
                                                fgets(sTmpStr, 128, stdin);
                                                if (sTmpStr[0] != 'y' && sTmpStr[0] != 'Y')
                                                {
                                                        break;
                                                }
                                                //确认
                                                g_insMaintain.setFlag(CMaintain::ClearDisabledDuty, nValue);
                                        }
                                        else if (6 == iInput)
                                        {
                                                int iValue = g_insMaintain.getFlag(CMaintain::QueueSafeMode);
                                                int nValue = iValue^1;  //取反
                                                printf("Current QueueSafeMode value is %d, Are you change to %d ? (Y/N)\n", 
                                                        iValue, nValue);
                                                fgets(sTmpStr, 128, stdin);
                                                if (sTmpStr[0] != 'y' && sTmpStr[0] != 'Y')
                                                {
                                                        break;
                                                }
                                                //确认
                                                g_insMaintain.setFlag(CMaintain::QueueSafeMode, nValue);
                                        }
                                }

                        }
                }
                // ======== 模拟Sitewizard ========
                else if (4 == iInput)
                {
                        if (0 != g_insConfigMng.init())
                        {
                                continue;
                        }
                        while(1)
                        {                                
                                printMenu0_4();
                                fgets(sTmpStr, 128, stdin);
                                iInput = atoi(sTmpStr);
                                if (0 > iInput || 6 < iInput)
                                {
                                        continue;
                                }
                                if (0 == iInput)
                                {
                                        break;
                                }
                                else
                                {
                                        if (1 == iInput)
                                        {
                                                CSendDuty insSendDuty;
                                                
                                                char sTmpLong[256];
                                                printf("Input Msg Type(1 Upload 2 Delete)==>");
                                                fgets(sTmpLong, 256, stdin);
                                                iInput = atoi(sTmpLong);
                                                if (1 == iInput)
                                                {
                                                        insSendDuty.m_iProtocolType = CProtocolMng::ProtocolUploadFile;
                                                        printf("Input site name==>");
                                                        fgets(sTmpLong, 256, stdin);
                                                        sTmpLong[strlen(sTmpLong) - 1] = '\0';
                                                        strcpy(insSendDuty.m_sSiteName, sTmpLong);
                                                        printf("Input opposite Dest file==>");
                                                        fgets(sTmpLong, 256, stdin);
                                                        sTmpLong[strlen(sTmpLong) - 1] = '\0';
                                                        strcpy(insSendDuty.m_sDestPathFile, sTmpLong);
                                                        strcpy(insSendDuty.m_sSendedTime, CComFun::GetMsgCurDateTime().c_str());
                                                        printf("Input File Type (1 Name 2 Data)=>");
                                                        fgets(sTmpLong, 256, stdin);
                                                        g_iFileType = atoi(sTmpLong);
                                                        printf("Input File Local Name with the whole dir =>");
                                                        fgets(sTmpLong, 256, stdin);
                                                        sTmpLong[strlen(sTmpLong) - 1] = '\0';
                                                        strcpy(insSendDuty.m_sLocalPathFile, sTmpLong);
                                                }
                                                else
                                                {
                                                        insSendDuty.m_iProtocolType = CProtocolMng::ProtocolDelFile;
                                                        printf("Input site name==>");
                                                        fgets(sTmpLong, 256, stdin);
                                                        sTmpLong[strlen(sTmpLong) - 1] = '\0';
                                                        strcpy(insSendDuty.m_sSiteName, sTmpLong);
                                                        printf("Input opposite Dest file==>");
                                                        fgets(sTmpLong, 256, stdin);
                                                        sTmpLong[strlen(sTmpLong) - 1] = '\0';
                                                        strcpy(insSendDuty.m_sDestPathFile, sTmpLong);
                                                        strcpy(insSendDuty.m_sSendedTime, CComFun::GetMsgCurDateTime().c_str());
                                                }
                                                int iRet = sendOneDutyMsg(insSendDuty);
                                                printf("\n\nOK::send msg finnished, the result is %d\n\n", iRet);
                                        }
                                        else if (2 == iInput)
                                        {
                                                while(1)
                                                {
                                                        printMenu0_4_2();
                                                        fgets(sTmpStr, 128, stdin);
                                                        iInput = atoi(sTmpStr);
                                                        if (0 > iInput || 2 < iInput)
                                                        {
                                                                continue;
                                                        }
                                                        if (0 == iInput)
                                                        {
                                                                break;
                                                        }
                                                        else
                                                        {      
                                                                int iTimes = 0;
                                                                if (2 == iInput)
                                                                {
                                                                        printf("\n\nWARNING::Enter Loop send... if you want to end sendduty, please exit this tool.\n\n");
                                                                        iTimes = -1;
                                                                }
                                                                else
                                                                {
                                                                        printf("\n\nPlease input send times==>");
                                                                        fgets(sTmpStr, 128, stdin);
                                                                        iTimes = atoi(sTmpStr);
                                                                }                                                                
                                                                sendMultDutyMsg(iTimes);
                                                        }
                                                }
                                        }
                                        else if (3 == iInput)
                                        {
                                                int iRet = sendstatuscheck(g_insConfigMng.m_tComCfg.m_sServerIP, g_insConfigMng.m_tComCfg.m_iServerPort);
                                                printf("\n\nSend status check result is %d\n\n", iRet);
                                        }
                                        else if (4 == iInput)
                                        {
                                                char sTmpLong1[256 + 1];
                                                char sTmpLong2[256 + 1];
                                                printf("Input Server IP==>");
                                                fgets(sTmpLong1, 256, stdin);
                                                sTmpLong1[strlen(sTmpLong1) - 1] = '\0';
                                                printf("Input Sever Port==>");
                                                fgets(sTmpLong2, 256, stdin);
                                                int iPort = atoi(sTmpLong2);
                                                int iRet = sendstatuscheck(sTmpLong1, iPort);
                                                printf("\n\nSend status check result is %d\n\n", iRet);
                                        }
                                        else  if (5 == iInput)
                                        {
                                                CServerAddMsg insServerAdd;
                                                char sTmpLong[256];
                                                printf("Input site name==>");
                                                fgets(sTmpLong, 256, stdin);
                                                sTmpLong[strlen(sTmpLong) - 1] = '\0';
                                                strcpy(insServerAdd.m_sSiteName, sTmpLong);
                                                printf("Input Server IP==>");
                                                fgets(sTmpLong, 256, stdin);
                                                sTmpLong[strlen(sTmpLong) - 1] = '\0';
                                                strcpy(insServerAdd.m_sServerIP, sTmpLong);
                                                printf("Input Server Port==>");
                                                fgets(sTmpLong, 256, stdin);
                                                insServerAdd.m_iPort = atoi(sTmpLong);
                                                strcpy(insServerAdd.m_sTime ,CComFun::GetMsgCurDateTime().c_str());
                                                sendServerAddMsg(insServerAdd);
                                        }
                                        else if (6 == iInput)
                                        {
                                                CServerDelMsg insServerDel;
                                                char sTmpLong[256];
                                                printf("Input site name==>");
                                                fgets(sTmpLong, 256, stdin);
                                                sTmpLong[strlen(sTmpLong) - 1] = '\0';
                                                strcpy(insServerDel.m_sSiteName, sTmpLong);
                                                printf("Input Server IP==>");
                                                fgets(sTmpLong, 256, stdin);
                                                sTmpLong[strlen(sTmpLong) - 1] = '\0';
                                                strcpy(insServerDel.m_sServerIP, sTmpLong);
                                                printf("Input Server Port==>");
                                                fgets(sTmpLong, 256, stdin);
                                                insServerDel.m_iPort = atoi(sTmpLong);
                                                strcpy(insServerDel.m_sTime ,CComFun::GetMsgCurDateTime().c_str());
                                                sendServerDelMsg(insServerDel);
                                        }
                                }
                        }
                }
                // ======== 退出系统 ========
                else if (5 == iInput)
                {
                        //危险性确认
                        printf("This operation will stop the newoctopusd and all child process, are you sure(Y/N)?");
                        fgets(sTmpStr, 128, stdin);
                        if (sTmpStr[0] != 'y' && sTmpStr[0] != 'Y')
                        {
                                continue;
                        }

                        //是否保存队列中的任务
                        printf("do you want to save the duty of the sendQueue to the faildutyFile(Y/N)?");
                        fgets(sTmpStr, 128, stdin);
                        int iRet = 0;
                        if (sTmpStr[0] != 'y' && sTmpStr[0] != 'Y')
                        {
                                iRet = 0;
                        }
                        else
                        {
                                iRet = 1;
                        }
                        
                        if (0 != g_insMaintain.connect())
                        {
                                printf("\n\nError:Connect to share memory failed.\n\n");
                                continue;
                        }
                        g_insMaintain.setFlag(CMaintain::SaveDutyQueueWhenExit, iRet);
                        g_insMaintain.setFlag(CMaintain::ExitState, 1);                        
                }
                else if (6 == iInput)
                {
                        while(1)
                        {
                                printMenu0_6();
                                fgets(sTmpStr, 128, stdin);
                                iInput = atoi(sTmpStr);
                                if (0 > iInput ||7 < iInput)
                                {
                                        continue;
                                }
                                if (0 == iInput)
                                {
                                        break;
                                }
                                else if (1 == iInput)
                                {
                                        if (0 != g_insMaintain.connect())
                                        {
                                                printf("\n\nError:Connect to share memory failed.\n\n");
                                                continue;
                                        }
                                        g_insMaintain.printScreen();
                                }
                                else if (2 == iInput)
                                {
                                        //确认2次
                                        printf("This Operation must execute after newoctopusd exit, or newoctopusd will coredump, are you sure(Y/N)?");
                                        fgets(sTmpStr, 128, stdin);
                                        if (sTmpStr[0] != 'y' && sTmpStr[0] != 'Y')
                                        {
                                                continue;
                                        }
                                        
                                        printf("This Operation must execute after newoctopusd exit, or newoctopusd will coredump, are you sure really(Y/N)?");
                                        fgets(sTmpStr, 128, stdin);
                                        if (sTmpStr[0] != 'y' && sTmpStr[0] != 'Y')
                                        {
                                                continue;
                                        }
                                        
                                        if (0 != g_insMaintain.delShareMem())
                                        {
                                                printf("\n\nError:Delete share memory failed.\n\n");
                                                continue;
                                        }
                                }
                                else if (3 == iInput)
                                {
                                        char sTmpStrLong1[256+1];
                                        char sTmpStrLong2[256+1];
                                        printf("Please the moblie number==>");                                        
                                        fgets(sTmpStrLong1, 256, stdin);
                                        printf("Please the short message(max 70 word)==>");
                                        fgets(sTmpStrLong2, 256, stdin);
                                        CMobileInterface::SendSMSMsg(sTmpStrLong1, sTmpStrLong2);
                                        printf("\n\nOK:Send finished, please check.\n\n");
                                }
                                else if (4 == iInput)
                                {
                                        if (0 != g_insMaintain.connect())
                                        {
                                                printf("\n\nError:Connect to share memory failed.\n\n");
                                                continue;
                                        }
                                        g_insMaintain.setFlag(CMaintain::PrintThread2Pid, 1);
                                        printf("\n\nOK:Please find the result file in dir log/prt\n\n");
                                }
                                else if (5 == iInput)
                                {
                                        if (0 != g_insMaintain.connect())
                                        {
                                                printf("\n\nError:Connect to share memory failed.\n\n");
                                                continue;
                                        }
                                        g_insMaintain.setFlag(CMaintain::ForceCloseAllSocket, 1);
                                        printf("\n\nOK:Send finished, please check.\n\n");
                                }
                                else if (6 == iInput)
                                {
                                        printf("This Operation will clear all error ipport record, are you sure(Y/N)?");
                                        fgets(sTmpStr, 128, stdin);
                                        if (sTmpStr[0] != 'y' && sTmpStr[0] != 'Y')
                                        {
                                                continue;
                                        }
                                        if (0 != g_insMaintain.connect())
                                        {
                                                printf("\n\nError:Connect to share memory failed.\n\n");
                                                continue;
                                        }
                                        g_insMaintain.setFlag(CMaintain::ClearErrorIPPort, 1);
                                        printf("\n\nOK:Send finished, please check.\n\n");
                                }
                                else if (7 == iInput)
                                {
                                        printf("This Operation will close all socketfd from 10 to 1024, are you sure(Y/N)?");
                                        fgets(sTmpStr, 128, stdin);
                                        if (sTmpStr[0] != 'y' && sTmpStr[0] != 'Y')
                                        {
                                                continue;
                                        }
                                        if (0 != g_insMaintain.connect())
                                        {
                                                printf("\n\nError:Connect to share memory failed.\n\n");
                                                continue;
                                        }
                                        g_insMaintain.setFlag(CMaintain::ForceCloseAllSocketFD, 1);
                                        printf("\n\nOK:Send finished, please check.\n\n");
                                }
                        }
                }
                else if (0 == iInput)
                {
                        break;
                }                
        }

        return 0;
}



