//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ServerInfo.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef SERVERINFO_H_HEADER_INCLUDED_BCE185FE
#define SERVERINFO_H_HEADER_INCLUDED_BCE185FE

#include "ComFun.h"

class CServerInfo
{
      public:
        CServerInfo();
        CServerInfo(const CServerInfo& right);
        virtual ~CServerInfo();
        CServerInfo& operator=(const CServerInfo& right);

        //服务器的名称
        char m_sName[LenOfNormal];

        //服务器的IP
        char m_sIP[LenOfIPAddr];

        //服务器的端口
        int m_iPort;
};

#endif /* SERVERINFO_H_HEADER_INCLUDED_BCE185FE */

