/**
 * \file TcpConnector.hh 
 * \brief declare common net client iterface for tcp app.
 */
#ifndef __TCPCONNECTOR__HH__
#define __TCPCONNECTOR__HH__

#include "Connector.h"
#include "ClientMsg.h"

namespace CGIMAGIC {

/**
 * Used to establish tcp connection to server and
 * supplied read /write data interface.
 */ 
class Magic_TcpConnector:public Magic_Connector {
 public:
  //constructor
  Magic_TcpConnector(void);
  
  //destructor.
  virtual ~Magic_TcpConnector(void);

  /**
   * Used to establish connection to server.
   * @param addr indicates client  server addr.
   * @return  1 partial success.
   *     connection is not established for interrupted 
   *      or in progress status.
   * 
   *          0 on success.
   *         -1 on failure.
   */
  
  virtual int Connect(const Magic_SocketAddr& addr); 

  //有超时链接
  virtual int Connect_tm(const Magic_SocketAddr& addr,int timeout=-1); 

  //重复读取，直到读取到n个字节
  virtual int Read_n(void *pMsg, int iLen,int timeout = -1) ;

  //调用read一次，最多读取iLen个字节
  virtual int Read(void *pMsg, int iLen);

  //重复写入，最多写入iLen个字节
  virtual int Write_n(const void *pMsg,int iLen,int timeout=-1);

  //写入一次，最多写入iLen个字节
  virtual int Write(const void *pMsg,int iLen);
        
	int getSocket(void) const 
	{
		return m_sockfd;
	}

    int ReceiveMessage(cClientMsg& ClientMsg);
    int SendMessage(cClientMsg& ClientMsg);

  //设置阻塞标记位
  virtual int setBlock(int block);
 
private:

  // socket are block or not.
  // 0 indicates no block.
  // 1 indicates block.
  int   m_iBlock;
};


}

#endif  //__TCPSOCKETCONNECTOR__HH__
 
