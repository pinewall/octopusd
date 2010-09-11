/**
 * \file SocketAddr.hh 
 * \brief  decare socket address wrapper facade.
 */
#ifndef  __SOCKETADDR_HH__
#define  __SOCKETADDR_HH__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

/**
 * Magic_SocketAddr suppies socket address
 * wrapper interface.
 */
class Magic_SocketAddr {

public:
  
  //default constructor.
  Magic_SocketAddr(void);

  Magic_SocketAddr(const std::string& host ,
		unsigned short port 
    );
 
  //copy constructor
  Magic_SocketAddr(const Magic_SocketAddr& addr);
  
  Magic_SocketAddr(const sockaddr_in &addr);
 
  //evaluate constructor
  Magic_SocketAddr& operator= (const Magic_SocketAddr& addr);

  virtual ~Magic_SocketAddr(void);

  void setHost(const std::string& host);
  
  std::string getHost(void) ;

  void setPort(unsigned short port);

  unsigned short getPort(void) ;

  inline struct sockaddr_in getAddress(void) const
  {
	return m_stAddr;
  }

private:
  
  struct in_addr getNetIp(const std::string& host);
 
  struct sockaddr_in m_stAddr;

};

#endif // __SOCKETADDR_HH__

