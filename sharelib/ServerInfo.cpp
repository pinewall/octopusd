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

#include "ServerInfo.h"

CServerInfo::CServerInfo()
{
}

CServerInfo::CServerInfo(const CServerInfo& right)
{
        strcpy(m_sName, right.m_sName);
        strcpy(m_sIP, right.m_sIP);
        m_iPort = right.m_iPort;
}

CServerInfo::~CServerInfo()
{
}

CServerInfo& CServerInfo::operator=(const CServerInfo& right)
{
        if (this != &right)
        {
                strcpy(m_sName, right.m_sName);
                strcpy(m_sIP, right.m_sIP);
                m_iPort = right.m_iPort;
        }
        return *this;
}

