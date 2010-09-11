//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *
//  *    $RCSfile: TCPSocket.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "TCPSocket.h"

extern CConfigMng g_insConfigMng;
extern CLogMng g_insLogMng;

CTCPSocket::CTCPSocket()
{
        m_iFd = 0;
        m_sIP[0] = '\0';
        m_iPort = 0;
        m_iBlockMode = 0;
}

CTCPSocket::CTCPSocket(const char* sIP, int iPort)
{
        m_iFd = 0;
        strcpy(m_sIP, sIP);
        m_iPort = iPort;
        m_iBlockMode = 0;
}

CTCPSocket::CTCPSocket(int iFd, const char* sIP, int iPort)
{
        m_iFd = iFd;
        strcpy(m_sIP, sIP);
        m_iPort = iPort;
        m_iBlockMode = 0;
}

CTCPSocket::~CTCPSocket()
{
}

CTCPSocket::CTCPSocket(const CTCPSocket& right)
{
        m_iFd = right.m_iFd;
        strcpy(m_sIP, right.m_sIP);
        m_iPort = right.m_iPort;
        m_iBlockMode = right.m_iBlockMode;
}

CTCPSocket& CTCPSocket::operator=(const CTCPSocket& right)
{
        if (this != &right)
        {
                m_iFd = right.m_iFd;
                strcpy(m_sIP, right.m_sIP);
                m_iPort = right.m_iPort;
                m_iBlockMode = right.m_iBlockMode;
        }
        return *this;
}

void CTCPSocket::init(const char* sIP, int iPort)
{
        m_iFd = 0;
        strcpy(m_sIP, sIP);
        m_iPort = iPort;
        m_iBlockMode = 0;
}

void CTCPSocket::init(int iFd, const char* sIP, int iPort)
{
        m_iFd = iFd;
        strcpy(m_sIP, sIP);
        m_iPort = iPort;
        m_iBlockMode = 0;
}

int CTCPSocket::createSocket()
{
        g_insLogMng.normal("Begin to create socket server(%s, %d)......", m_sIP, m_iPort);
        int iRet = 0;
        int iReuseAddr = 1;

        struct sockaddr_in insAddress;
        memset((char *) &insAddress, 0, sizeof(insAddress));
        insAddress.sin_family = AF_INET;
        insAddress.sin_port = htons(m_iPort);
        if (0 != strcmp(m_sIP,  "ALL") && 0 != strcmp(m_sIP,  "all") )
        {
                inet_aton(m_sIP, &insAddress.sin_addr);
        }
        else
        {
                insAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        }

        // 生成 fd
        m_iFd = socket(AF_INET, SOCK_STREAM, 0);
        if (m_iFd < 0)
        {
                g_insLogMng.error("Create: socket() failed, errno = %d, reason = %s", errno, strerror(errno));
                return -1;
        }
        g_insLogMng.debug("socket", CLogMng::LogMode1, "Create socket id successfully, fd = %d", m_iFd);

        //可复用
        setsockopt(m_iFd, SOL_SOCKET, SO_REUSEADDR, (void*)(&(iReuseAddr)),  sizeof(iReuseAddr));
        g_insLogMng.debug("socket", CLogMng::LogMode3, "Setsockopt successfully.");

        if (bind(m_iFd, (struct sockaddr *) &insAddress, sizeof(insAddress)) < 0)
        {
                g_insLogMng.error("Create: bind() failed, errno = %d, reason = %s", errno, strerror(errno));
                closeSocket();
                return -1;
        }
        g_insLogMng.debug("socket", CLogMng::LogMode3, "Bind successfully.");

        /* Queue up to Max connections before having them automatically rejected. */
        int iTmpNrOfCnt;
        if ( 0 != g_insConfigMng.m_tComCfg.m_iMaxNrOfCnt)
        {
                iTmpNrOfCnt = 10000;
        }
        else
        {
                iTmpNrOfCnt = g_insConfigMng.m_tComCfg.m_iMaxNrOfCnt + 10; //多余１０个缓冲
        }
        // listen
        iRet = listen(m_iFd, iTmpNrOfCnt);
        if (iRet == -1)
        {
                g_insLogMng.error("Create: listen() failed, errno = %d, reason = %s", errno, strerror(errno));
                closeSocket();
                return -1;
        }
        g_insLogMng.debug("socket", CLogMng::LogMode3, "Listen(maxcnt = %d) successfully.", iTmpNrOfCnt);

        //non block
        if (0 != setBlockMode(NonBlock))
        {
                g_insLogMng.error("Create: setNonBlocking() failed, errno = %d, reason = %s", errno, strerror(errno));
                closeSocket();
                return -1;
        }
        
         //设置socket句柄的tcpcork属性
         int on = 1;
         setsockopt (m_iFd, SOL_TCP, TCP_CORK, &on, sizeof (on));

         int  nSendBuf=100*1024;//设置为100K
         setsockopt(m_iFd, SOL_SOCKET, SO_SNDBUF, (const  char*)&nSendBuf, sizeof(int));
         
        g_insLogMng.debug("socket", CLogMng::LogMode3, "Set nonblock successfully.");

        g_insLogMng.normal("Create socket server(%s, %d) successfully", m_sIP, m_iPort);

        return 0;
}


int CTCPSocket::acceptClient(char* sIP)
{        
        g_insLogMng.debug("socket", CLogMng::LogMode2, "Begin to accept client connection......");
        struct sockaddr_in clientAddr;
        int iClientSocketSize = sizeof(clientAddr);
        memset(&clientAddr, 0, iClientSocketSize);

        int iClientFd = accept(m_iFd, (struct sockaddr *)&clientAddr, (socklen_t*)&iClientSocketSize);
        if (0 >= iClientFd)
        {
                g_insLogMng.debug("socket", CLogMng::LogMode2, "Accept client connection failed.");
                return -1;
        }
        else
        {
                struct in_addr  tmpInaddr;
                tmpInaddr.s_addr = clientAddr.sin_addr.s_addr;
                strcpy(sIP,  (char *)inet_ntoa(tmpInaddr));
                g_insLogMng.debug("socket", CLogMng::LogMode1, "Accept client(%s) connection successfully, fd = %d", 
                        sIP, iClientFd);
                //g_insLogMng.runlog("Accept:fd=%d;ip=%s", iClientFd, m_sIP);
                return iClientFd;
        }

}

int CTCPSocket::connectServer()
{
        g_insLogMng.debug("socket", CLogMng::LogMode2, "Begin to connect to server......");

        m_iFd = socket(AF_INET, SOCK_STREAM, 0);
        if( m_iFd <= 0)
        {
                g_insLogMng.error( "socket(AF_INET, SOCK_STREAM, 0) for %s failed, m_iFd = %d", m_sIP);
                return -1;
        }
        //g_insLogMng.runlog("Create:fd=%d;ip=%s", m_iFd, m_sIP);
        g_insLogMng.debug("socket", CLogMng::LogMode2, "Create fd = %d for connect (%s) successfully.", 
                m_iFd, m_sIP);

        struct sockaddr_in stInAddr;
        memset(&stInAddr,0,sizeof(stInAddr));
        stInAddr.sin_family = AF_INET;   //family
        stInAddr.sin_port = htons(m_iPort); //port
        struct in_addr tmpAddr;

        //inet_aton() converts the Internet host address cp from the  standard  num-
       //bers-and-dots  notation  into  binary  data and stores it in the structure
       //that inp points to. inet_aton returns nonzero if  the  address  is  valid,
       //zero if not.        
        if (0 == inet_aton(m_sIP, &tmpAddr))
        {
                g_insLogMng.debug("socket", CLogMng::LogMode2, "Connect to server failed, server IP(%s) is invalid.", m_sIP);
                closeSocket();  // kevinliu add 06-06
                return -1;
        }
        stInAddr.sin_addr = tmpAddr;  //ip

        //建立非阻塞方式，目的是connect超时可控制，完成后恢复
        if (0 != setBlockMode(NonBlock))
        {
                g_insLogMng.debug("socket", CLogMng::LogMode2, "Connect to server failed, setBlockMode NonBlock failed.");
                closeSocket(); // kevinliu add 06-06
                return -1;
        }

        int iRet = connect(m_iFd, (struct sockaddr*)&stInAddr, sizeof(stInAddr));
        if(iRet < 0)
        {
                 g_insLogMng.debug("socket", CLogMng::LogMode2, "Connect to server(%s) error.errno=%d, %s", 
                                m_sIP, errno, strerror(errno));
                if (errno != EINPROGRESS &&  errno != EALREADY  //  in process  //delete && errno != EINTR
                    && errno != EAGAIN)   // try again
                {
                        g_insLogMng.error( "Connect to server(%s) error.errno=%d, %s", 
                                m_sIP, errno, strerror(errno));
                        closeSocket();
                        return -1;
                }
                else  // 其他的再尝试检查端口的状态
                {
                        if ( 0 > checkSocket(g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout, CTCPSocket::WriteMode))
                        {
                                g_insLogMng.error("Connect to Server(%s) failed, checksocket timeout.", m_sIP);
                                closeSocket();
                                return -1;
                        }
                }
        }
        
        //恢复回阻塞方式
        if (0 != setBlockMode(Block))
        {
                g_insLogMng.debug("socket", CLogMng::LogMode2, "Connect to server failed, recover setBlockMode Block failed.");
                closeSocket();
                return -1;
        }

        // Check if there were any error  这一块应该没有太大作用,去掉
        /*
        int err;
        socklen_t err_len = sizeof(int);
        int ret =  getsockopt(m_iFd, SOL_SOCKET,SO_ERROR,&err,&err_len);
        if(ret < 0) 
        {
                g_insLogMng.error("getsockopt return -1");
        	closeSocket();
                return -1;
        }
        if(err > 0)
        {
                g_insLogMng.error("getsockopt return error");
        	 closeSocket();
                return -1;
        }
        */

        g_insLogMng.debug("socket", CLogMng::LogMode1, "Connect to server(%s) successfully, fd = %d", 
                m_sIP, m_iFd);

        return 0;

}

int CTCPSocket::checkSocket(const int iTimeOut, SelectMode eSelectMode)
{
        int iRet = 0;
        fd_set  rd_set, wt_set;
        struct timeval time_val;

        int iRetry = 0;
        unsigned int uiBeginTime = time(NULL);

        //g_insLogMng.debug("socket", CLogMng::LogMode3, "CheckSocket(%s): Begin to select socket.", m_sIP);

        while(1)
        {
                time_val.tv_usec = 0;
                if(iTimeOut <= 0)
                {
                        time_val.tv_sec = DefaultSelectTimeOut;
                }
                else
                {
                        time_val.tv_sec = iTimeOut;
                }

                if(ReadMode == eSelectMode)
                {
                        FD_ZERO(&rd_set);
                        FD_SET((unsigned int)m_iFd, &rd_set);

                        iRet = select(m_iFd + 1, &rd_set, NULL, NULL, &time_val);
                }
                else if(WriteMode == eSelectMode)
                {
                        FD_ZERO(&wt_set);
                        FD_SET((unsigned int)m_iFd, &wt_set);

                        iRet = select(m_iFd+1, NULL, &wt_set, NULL, &time_val);
                }
                else if(ReadAndWriteMode == eSelectMode)
                {
                        FD_ZERO(&rd_set);
                        FD_SET((unsigned int)m_iFd, &rd_set);
                        FD_ZERO(&wt_set);
                        FD_SET((unsigned int)m_iFd, &wt_set);

                        iRet = select(m_iFd + 1, &rd_set, &wt_set, NULL, &time_val);
                }

                if(iRet == 0)  //超时
                {
                        return -1;
                }                
                else if(iRet > 0)
                {
                        break;
                }
                else //if(iRet == -1)
                {
                        if(errno == EINTR && iRetry < MaxIgnoreSignal)
                        {
                                iRetry++;
                                continue;
                        }
                        g_insLogMng.debug("socket", CLogMng::LogMode3, "CheckSocket(%s): select return -1, errno = %s, reason = %s, NrOfretry = %d",
                                         m_sIP, errno, strerror(errno), iRetry);
                        return -1;
                }
        }

        g_insLogMng.debug("socket", CLogMng::LogMode3, "CheckSocket select socket OK, mode = %d(0=W 1=R 2=RW).",  eSelectMode);

        return(time(NULL) - uiBeginTime);

}


int CTCPSocket::sendMsg(char* pBuffer, int iLen, int iTimeout)
{
        const char  *pTemp = pBuffer;
        int iRemainSize = iLen;
        int iSendSize;

        //int iCounter = 0;

         g_insLogMng.debug("socket", CLogMng::LogMode3, "Begin to send message block.");

        while(iRemainSize > 0)
        {
                // wait until the socket is writable
                if(checkSocket(iTimeout, WriteMode) == -1)
                {
                        g_insLogMng.error( "Send message failed as check socket timeout.");
                        return -1;  //等待出错
                }

                iSendSize = send(m_iFd, pTemp, iRemainSize, 0);
                if(iSendSize < 0)
                {
                        if(errno == ENOBUFS || errno == EAGAIN)
                        {
                                iSendSize = 0;
                        }
                        /* ignore signal */
                        //kevinliu modify temp 10-04
                        //else if(errno == EINTR && nCounter < MaxIgnoreSignal && iSendSize < 0)
                        else if(errno == EINTR && iSendSize < 0)
                        {
                                //iCounter++;
                                iSendSize = 0;
                                CComFun::sleep_msec(0, 10);
                        }
                        else
                        {
                                g_insLogMng.error( "Send message failed because send to socket error.");
                                return -2;  //发送出错
                        }

                }

                iRemainSize = iRemainSize - iSendSize;
                if(iRemainSize > 0)
                {
                        pTemp = pTemp + iSendSize;
                }
        }

        // code stream
        g_insLogMng.bin("socket", CLogMng::LogMode3, pBuffer, iLen);

        g_insLogMng.debug("socket", CLogMng::LogMode3, "End to send message block.");

        return 0;
}

int CTCPSocket::receiveMsg(char* pBuffer, int iLen, int iTimeout)
{
        g_insLogMng.debug("socket", CLogMng::LogMode3, "Begin to Recv message block.");
        char *pTemp = pBuffer;

        int iRemainSize = iLen;
        int iRecvSize;
        int iRemainTime = iTimeout;
        if (0 >= iRemainTime)
        {
                iRemainTime = DefaultSelectTimeOut;
        }
        int iWaitTime;

        int iRetry = 0;

        while(iRemainSize > 0)
        {
                iWaitTime = checkSocket(iRemainTime, ReadMode);
                if(iWaitTime == -1)
                {
                        g_insLogMng.error( "Recv message failed because check socket timeout.");
                        return -1;  //错误
                }
                iRemainTime = iRemainTime - iWaitTime;
                if(iRemainTime <= 0)
                {
                        // if has received some data
                        if(iRemainSize != iLen)
                        {
                                g_insLogMng.error( "Recv message failed because no time left and some data finnished(All=%d, Remain=%d).",
                                        iLen, iRemainSize);
                                return -2;  // 收了一部分数据
                        }
                        else
                        {
                                g_insLogMng.error( "Recv message failed because no time left.");
                                return -2; 
                        }
                }

                while(1)
                {
                        iRecvSize = recv(m_iFd, pTemp, iRemainSize, 0);
                        if(iRecvSize <= 0)
                        {
                                /* ignore signal */
                                if(errno == EINTR && iRetry < MaxIgnoreSignal && iRecvSize < 0)
                                {
                                        iRetry++;
                                        CComFun::sleep_msec(0, 10);
                                        continue;
                                }
                                
                                if (0 == iRecvSize)
                                {
                                        g_insLogMng.normal( "Opposite side of socket itself closed this connection.");
                                        return 1;  //接收到了一部分数据
                                }
                                
                                if(iRemainSize != iLen)
                                {
                                        g_insLogMng.error( "Recv message failed because recv error but some data finnished(All=%d, Remain=%d).",
                                                 iLen, iRemainSize);
                                        return -3;  //接收到了一部分数据
                                }
                                else
                                {
                                        g_insLogMng.error( "Recv message failed because recv error(recv size=%d).", iRecvSize);
                                        return -3; 
                                }
                        }
                        break;
                }

                iRemainSize = iRemainSize - iRecvSize;
                if(iRemainSize > 0)
                {
                        pTemp = pTemp + iRecvSize;
                }
        }

        // code stream
        g_insLogMng.bin("socket", CLogMng::LogMode3, pBuffer, iLen);

        g_insLogMng.debug("socket", CLogMng::LogMode3, "End to Recv message block.");

        return 0;
}

void  CTCPSocket::closeSocket()
{        
        if (m_iFd > 0)
        {                
                int i = 0;
                while (0 != close(m_iFd))  //没有被成功关闭,为了安全，这块做严格的判断
                {
                        g_insLogMng.error( "Try again:close(%d) failed with %s, errno=%d,%s", 
                                        m_iFd, m_sIP, errno, strerror(errno));
                        i++;                        
                        if(i >= 100) //重试次数，也得有个限制
                        {
                                g_insLogMng.error( "Fatal:close(%d) failed with %s, errno=%d,%s", 
                                        m_iFd, m_sIP, errno, strerror(errno));
                                break;
                        }
                        sleep(1); //休眠一秒再关闭
                }
                //g_insLogMng.runlog("Close:fd=%d;ip=%s;i=%d", m_iFd, m_sIP, i);
                g_insLogMng.debug("socket", CLogMng::LogMode1, "Close Socket(fd = %d).", m_iFd);
                m_iFd = 0;  //关闭掉以后，将fd设置成0                
        }
        else
        {
                g_insLogMng.debug("socket", CLogMng::LogMode1, "Socket(fd = %d) invalid.", m_iFd);
        }
        return;
}

int  CTCPSocket::setBlockMode(BlockMode iBlockFlag)
{
        int opts;
        opts=fcntl(m_iFd, F_GETFL);  
        if(opts == -1)
        {
                g_insLogMng.error( "setBlockMode:fcntl(m_iFd, F_GETFL) failed.");
                return -1;
        }
        if ( NonBlock == iBlockFlag)
        {
                opts = opts|O_NONBLOCK;
                opts = opts|O_NDELAY;
                m_iBlockMode = 0;
        }
        else
        {
                opts = opts&~O_NONBLOCK;
                m_iBlockMode = 1;
        }

         if(fcntl(m_iFd, F_SETFL, opts) == -1)
         {
                g_insLogMng.error( "setBlockMode:fcntl(m_iFd, F_SETFL, opts) failed.");
                return -1;
         }

     return 0;
}

