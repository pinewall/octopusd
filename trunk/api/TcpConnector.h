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

  //�г�ʱ����
  virtual int Connect_tm(const Magic_SocketAddr& addr,int timeout=-1); 

  //�ظ���ȡ��ֱ����ȡ��n���ֽ�
  virtual int Read_n(void *pMsg, int iLen,int timeout = -1) ;

  //����readһ�Σ�����ȡiLen���ֽ�
  virtual int Read(void *pMsg, int iLen);

  //�ظ�д�룬���д��iLen���ֽ�
  virtual int Write_n(const void *pMsg,int iLen,int timeout=-1);

  //д��һ�Σ����д��iLen���ֽ�
  virtual int Write(const void *pMsg,int iLen);
        
	int getSocket(void) const 
	{
		return m_sockfd;
	}

    int ReceiveMessage(cClientMsg& ClientMsg);
    int SendMessage(cClientMsg& ClientMsg);

  //�����������λ
  virtual int setBlock(int block);
 
private:

  // socket are block or not.
  // 0 indicates no block.
  // 1 indicates block.
  int   m_iBlock;
};


}

#endif  //__TCPSOCKETCONNECTOR__HH__
 
