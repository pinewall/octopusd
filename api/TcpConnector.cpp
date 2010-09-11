//TcpConnector.cpp 
#include "TcpConnector.h"
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>

using namespace CGIMAGIC;


static void (*SigHandle)(int);

static sigjmp_buf m_buf;

static void AlarmHandle(int)
{
   siglongjmp(m_buf,1);
}

Magic_TcpConnector::Magic_TcpConnector(void):m_iBlock(1)
{
}

Magic_TcpConnector::~Magic_TcpConnector(void)
{
}

int Magic_TcpConnector::setBlock(int block)
{

        int iFlag = fcntl(m_sockfd, F_GETFL);
        if (iFlag < 0)
		return -1;

        if (block)
                iFlag &= ~O_NONBLOCK;
        else
                iFlag |= O_NONBLOCK;

        if (fcntl(m_sockfd, F_SETFL, iFlag) < 0)
		return -1;

        m_iBlock = block;
        return 0;

}


int Magic_TcpConnector::Connect(
	const Magic_SocketAddr& addr
 )
{

   if(Socket(AF_INET,SOCK_STREAM,0)!=0) {
        return -1;
   }

   struct sockaddr_in address = addr.getAddress();

   int res = connect(m_sockfd,(struct sockaddr*)&address,sizeof(address));
    if(res < 0)
    {
        if ( (m_iBlock && errno == EINTR)
             || (!m_iBlock && errno == EINPROGRESS) )
        {
	     Close();
             return 1;
        }
	Close();
        return -1;
    }

    return 0;
}


int Magic_TcpConnector::Connect_tm(
	const Magic_SocketAddr& addr,
	int timeout
    )
{

 if(Socket(AF_INET,SOCK_STREAM,0)!=0) {
	return -1;
 }
 
 struct sockaddr_in address = addr.getAddress();

 if(timeout== -1) {
    int res = connect(m_sockfd,(struct sockaddr*)&address,sizeof(address));
    if(res < 0) 
    {
        if ( (m_iBlock && errno == EINTR) 
	     || (!m_iBlock && errno == EINPROGRESS) )
        {
	     Close();
	     return 1;
        }
	Close();
        return -1;
    } 
 }     
 else 
 {
   SigHandle = signal(SIGALRM,&AlarmHandle);
   if(sigsetjmp(m_buf,1))
   {
       alarm(0);
       signal(SIGALRM,SigHandle);
       Close();
       return -2; //time out 
   }
  alarm(timeout);
  int res = connect(m_sockfd,(struct sockaddr*)&address,sizeof(address));
  if(res <0) 
  {
      alarm(0);
      signal(SIGALRM,SigHandle);
      Close();
      return -1;
  }
  alarm(0);
  signal(SIGALRM,SigHandle);
 }
 
 return 0;
}

int Magic_TcpConnector::Read(void* pBuf, int iLen)
{
#if 0
  if(timeout !=-1) {     
   SigHandle = signal(SIGALRM,&AlarmHandle);
   if(sigsetjmp(m_buf,1))
   {
       alarm(0);
       signal(SIGALRM,SigHandle);
       return -2; //time out
   }
  alarm(timeout);
 }
#endif
  return read(m_sockfd,pBuf,iLen); 

}


int Magic_TcpConnector::Read_n(void *pBuf, int iLen,int timeout) 
{

  if(timeout !=-1) {
   SigHandle = signal(SIGALRM,&AlarmHandle);
   if(sigsetjmp(m_buf,1))
   {
       alarm(0);
       signal(SIGALRM,SigHandle);
       return -2; //time out
   }
    alarm(timeout);
  }

        char *pMsg =(char*)pBuf; 
   	int bytes_read = 0;

	int this_read;
	while( bytes_read < iLen )
	{
              do
    	       {
		this_read = read(m_sockfd, pMsg, iLen - bytes_read);
	       }
		while ( ( this_read < 0 )  && ( errno == EINTR ) );
		
		if (this_read < 0)
		{
			return this_read;
    	        }
    	         else if(this_read == 0) {
      			return bytes_read;
         	}
    	
    	   bytes_read += this_read;
    
    	   pMsg += this_read;
  	}
        
       alarm(0);
       signal(SIGALRM,SigHandle);
        return iLen;
}
int Magic_TcpConnector::Write(const void *pBuf,int iLen)
{
#if 0
  if(timeout !=-1) {
   SigHandle = signal(SIGALRM,&AlarmHandle);
   if(sigsetjmp(m_buf,1))
   {
       alarm(0);
       signal(SIGALRM,SigHandle);
       return -2; //time out
   }
  alarm(timeout);
 }
#endif 

   return write(m_sockfd,pBuf,iLen);
}

int Magic_TcpConnector::Write_n(const void *pBuf,int iLen,int timeout) 
{

  if(timeout !=-1) {
   SigHandle = signal(SIGALRM,&AlarmHandle);
   if(sigsetjmp(m_buf,1))
   {
       alarm(0);
       signal(SIGALRM,SigHandle);
       return -2; //time out
   }
   alarm(timeout);
  }

     int bytes_sent = 0;
     int this_write;
     const char* pMsg = (const char*)pBuf;
     while (bytes_sent < iLen)
     {
		do
		{
		  this_write = write(m_sockfd, pMsg, iLen - bytes_sent);
		}
		while ( (this_write < 0) && ( errno == EINTR ));
		
		if( this_write <= 0 )
		{
			  return this_write;
		}
		
		bytes_sent += this_write;
		
		pMsg += this_write;
	}     
     
       alarm(0);
       signal(SIGALRM,SigHandle);

     return iLen;
}

int Magic_TcpConnector::ReceiveMessage(cClientMsg& ClientMsg)
{
        char sTmpBuff[nMsgHeadLen];
        //MAGIC_LOG(("Begin to receive HeadMessage"));     
        int iRet = read(m_sockfd, sTmpBuff, nMsgHeadLen);
        if (0 > iRet)
        {           
                //MAGIC_LOG(("Receive HeadMessage Fail becauseof network error"));     
                return -1;
        }
        else if(0 == iRet) //初始状态，不应该能收到断连消息
        {         
                //MAGIC_LOG(("Receive HeadMessage Fail because current is initial status"));     
                return -1;
        }
        else if (iRet < nMsgHeadLen)
        {         
                //MAGIC_LOG(("Receive HeadMessage Fail because received bytes %d is not equal to needed %d", iRet, nMsgHeadLen));     
                return -1;
        }
        //消息内容的处理
        char *pXX = sTmpBuff;
        memcpy(&ClientMsg.m_iTotalLen, pXX + 1,  nMsgHeadLen -1);
        ClientMsg.m_iTotalLen = ntohl(ClientMsg.m_iTotalLen);  //获得实际消息体长度
        //检查版本信息是否正确
        if (cMsgVersion != sTmpBuff[0])
        {
                //MAGIC_LOG(("ReceiveMessage Fail because Received Message Version is %c", sTmpBuff[0]));     
                return -1;
        }

        //验证消息长度是否合法
        if ( ClientMsg.m_iTotalLen > nMaxMsgLen || ClientMsg.m_iTotalLen <= nMsgHeadLen)
        {
                //MAGIC_LOG(("ReceiveMessage Fail because Received Message Length is invalid: %d", ClientMsg.m_iTotalLen));     
                return -1;
        }
        
        //分配空间
        int iRemain = ClientMsg.m_iTotalLen - nMsgHeadLen;
        char *pMessage = new char[iRemain];
        memset(pMessage, 0, iRemain)        ;

        //数据接受不完整
        if(iRemain != Read_n(pMessage, iRemain))
        {
                delete[] pMessage;
                //MAGIC_LOG(("ReceiveMessage Fail because Received Body Message is not completed"));
                return -1;
        }
        //消息格式有错误
        iRet = ClientMsg.decode(pMessage, iRemain);
        delete[] pMessage;
        
        if(-1 == iRet)        
        {
                //MAGIC_LOG(("ReceiveMessage Fail because decode Message Fail"));     
                return -1;        
        }
        else
        {
                //MAGIC_LOG(("ReceiveMessage Success"));     
                return 0;
        }
}

int Magic_TcpConnector::SendMessage(cClientMsg& ClientMsg)
{
        //MAGIC_LOG(("Begin to SendMessage"));     
        //消息编码失败
        if(0 != ClientMsg.encode())
        {
                //MAGIC_LOG(("SendMessage Fail because encode Message Fail"));     
                return -1;
        }

        //MAGIC_LOG(("Encode Success"));     
        if(ClientMsg.m_iTotalLen != Write_n(ClientMsg.m_pszMessage, ClientMsg.m_iTotalLen))
        {
                //MAGIC_LOG(("SendMessage Fail because Message sent length is not equal to realMessage length"));     
                return -1;
        }
        else
        {
                //MAGIC_LOG(("SendMessage Success"));     
                return 0;
        }
}
