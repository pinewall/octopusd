//Connector.cpp 
#include "Connector.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

Magic_Connector::Magic_Connector(void)
{
}

Magic_Connector::~Magic_Connector(void)
{
}

int Magic_Connector::handle_ready(
           int fd,
           struct timeval& timeout,
           int read_ready,
           int write_ready,
           int exception_ready
  )
{
     // Wait for data or for the timeout to elapse. 
   int select_width = fd +1;

   fd_set fdset;
   FD_ZERO(&fdset);
   FD_SET(fd, &fdset);
   int  retval = select(
                     select_width,
                     read_ready? &fdset:0 ,
                     write_ready? &fdset:0,
                     exception_ready? &fdset:0,
                     &timeout);

    if(retval == 0) 
    {
      errno = ETIME;
      return 0;
    }
   
    return retval;

}


int Magic_Connector::Close(void)
{
   return close(m_sockfd);

}

int Magic_Connector::Socket(int iDomain,int iType,int iProtocol )
{

    m_sockfd = socket(iDomain,iType,iProtocol);
    if(m_sockfd < 0 ) return -1;
 
    return 0;
}

/*** 
int Magic_Connector::Bind(const Magic_SocketAddr& addr)
{
     struct sockaddr_in address;
     address = addr.getAddress();
     int res = bind(m_sockfd,(struct sockaddr*)&address,sizeof(address));
     if(res < 0) return -1;

     return 0;
}
*/

