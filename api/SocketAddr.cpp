//SocketAddr.cpp 
#include "SocketAddr.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

using namespace std;

Magic_SocketAddr::Magic_SocketAddr(void)
{
    memset(&m_stAddr,0,sizeof(m_stAddr));
    m_stAddr.sin_family = AF_INET;
}

Magic_SocketAddr::Magic_SocketAddr(
  const string& host,
  unsigned short port
 )
{
    memset(&m_stAddr,0,sizeof(m_stAddr));
    m_stAddr.sin_family = AF_INET;
    m_stAddr.sin_port = htons(port);
    m_stAddr.sin_addr = getNetIp(host);
}

Magic_SocketAddr::Magic_SocketAddr(const sockaddr_in &addr)
{
    memset(&m_stAddr,0,sizeof(m_stAddr));
    m_stAddr = addr;

}


void Magic_SocketAddr::setPort(unsigned short port)
{
    m_stAddr.sin_port = htons(port);

}


unsigned short Magic_SocketAddr::getPort(void) 
{

    return ntohs(m_stAddr.sin_port);
}
void Magic_SocketAddr::setHost(const string& host)
{
    m_stAddr.sin_addr = getNetIp(host);

} 

string Magic_SocketAddr::getHost(void) 
{

    char *addr=  inet_ntoa(m_stAddr.sin_addr);
    if(addr !=0) return addr;
    else  return "";
}


struct in_addr  
Magic_SocketAddr::getNetIp(const string& host)
{
     struct in_addr stAddr;
     memset(&stAddr,0,sizeof(stAddr));
     //host is ip.      
     if (inet_aton(host.c_str(), &stAddr)!=0){ //valid
	 return stAddr;
     }
         
     // host is hostname.
     struct hostent  *pstHost;
   if((pstHost = gethostbyname(host.c_str()))==NULL)
    {
    return stAddr;
    }

     /*****************
      *****freebsd does  not support gethostbyname_r interface.
      
     struct hostent stHostBuf, *pstHost;
     size_t tBufLen = 1024;
     int iHErr;
     char *pchBuffer = new char [tBufLen];
     while (gethostbyname_r(host.c_str(), &stHostBuf, pchBuffer, tBufLen,
		 &pstHost,&iHErr) !=0 ||pstHost == NULL) 
    {
      if (iHErr != NETDB_INTERNAL || errno != ERANGE)
      {
                        delete [] pchBuffer;
			return stAddr;
      }
      else
      {
                     // Enlarge buffer.  
                        delete [] pchBuffer;
                        tBufLen *= 2;
                        pchBuffer = new char [tBufLen];
       }
    }
    ********************/
     stAddr = *(struct in_addr*)pstHost->h_addr_list[0];
 //    delete [] pchBuffer;
     return stAddr;
}


Magic_SocketAddr::Magic_SocketAddr(
   const Magic_SocketAddr& addr
  )
{
   *this = addr;
}

Magic_SocketAddr &
 Magic_SocketAddr::operator=(const Magic_SocketAddr& addr)
{
	if(this == &addr)
	{
		return *this;
	}

   	m_stAddr = addr.m_stAddr;
  	return *this;
}

Magic_SocketAddr::~Magic_SocketAddr(void)
{
}


 

