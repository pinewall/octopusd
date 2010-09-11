//Connector.hh decare common net client iterface.
#ifndef __CONNECTOR__HH__
#define __CONNECTOR__HH__

#include "SocketAddr.h"

/**
 * \brief
 * Used to declare abstract connector 
 * interface.
 */
class Magic_Connector {
 public:
  //constructor
  Magic_Connector(void);
  
  //destructor.
  virtual ~Magic_Connector(void);

  /**
   * Used to establish connection to server.
   * @param addr indicates client  server addr.
   * @return 0 on success.
   *         -1 on failure.
   */
  
  virtual int Connect(const Magic_SocketAddr& addr) = 0; 

  /**
   * Used to close socket connection.
   * @return 0 on success.
   *         -1 on failure.
   */
  virtual int Close(void);

  /**
   * Used to read data from socket .
   * @param pMsg indicates recieved data.
   * @param iLen indicates length of recieved data.
   * @return num bytes read on success.
   *         -1 on failure.
   */
  virtual int Read(void *pMsg, int iLen) = 0; 

  /**
   * Used to write data to socket.
   * @param pMsg indicates send data.
   * @param iLen indicates length of send data.
   * @return num bytes write on success.
   *         -1 on failure.
   */
  virtual int Write(const void *pMsg,int iLen) = 0;

  /**
   * Used to get socket descriptor.
   * @return socket descriptor.
   */
  inline int getSocket(void) const {

	return m_sockfd;
  }

  /**
   * Used to handle handle status.
   */
  int handle_ready(
           int fd,       
           struct timeval& timeout,
           int read_ready,
           int write_ready,
           int exception_ready   
  );
 
 protected:
 
 
 /**
  * Wrapper socket function .
  * @return 0 on success.
  *  return -1 on failure.
  */
 int  Socket(int iDomain,int iType,int iProtocol = 0);

 /**
  * Wrapper socket bind function.
  * @return 0 on success.
  *  return -1 on failure.
  * This is server side function.
  */
// int  Bind(const  Magic_SocketAddr& addr);

 
 int m_sockfd;
 //socket descriptor

};

#endif  //__SOCKETCONNECTOR__HH__
 
