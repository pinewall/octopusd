/************************************************************************
 *  This is the source of an commercial software; you can't redistribute 
 *  it and/or modify it without the author's permit. All rights reserved.
 *  You may get support by contact bud@tencent.com if you really need.	 
 *                                                                       
 *    $RCSfile: Mobileinterface.h,v $
 *    $Revision: 1.0 $
 *    $Author: kevinliu $
 *    $Name:  $
 *    $Date: 2005/09/09 11:52:29 $
 ***********************************************************************/

#ifndef __MOBILE_INTERFACE_H__
#define __MOBILE_INTERFACE_H__

#include <iostream>
#include <string>
#include "LogMng.h"
#include "TCPSocket.h"

#define STX                     0x02
#define SMS_AGENT_OK            0
#define SMS_AGENT_ERRORPACKET   0x01
#define SMS_AGENT_ERRORLQMODE   0x02

typedef struct
{
        char cSTX;
        char cCMD;
        char Length[4];
        char cMODE;
        char sMobile[12];
        char sContent[141];
}TSmsCMDPacket;

typedef struct
{
        char cSTX;
        char cCMD;
        char length[4];
        char cResult;
        char sInfo[128];
}TSmsAgentResponse;

class CMobileInterface
{

public:
        CMobileInterface(const string& /*sSMSGWIP="172.16.19.55"*/, const unsigned short /*sSMSGWPort=1024*/)
        {
        }
        
        ~CMobileInterface()
        {
        }
        
        /**
         *  �����ֻ�����Ϣ
         *  \param   sMobile    ����Ŀ���ֻ���
         *  \param   sContent   ����Ϣ����
         *  \param   sSMSGWIP   �������ص�ַ
         *  \param   sSMSGWPort �������ض˿ں�
         *  \return   int  ������� 0:�ɹ�  ��0:ʧ��
         */
        static int SendSMSMsg(const string& sMobile, const string& sContent,
                const string& sSMSGWIP="172.16.19.55", const unsigned short sSMSGWPort=1024);
};

#endif
