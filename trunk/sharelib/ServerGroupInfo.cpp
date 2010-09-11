//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ServerGroupInfo.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: Kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07$
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "ServerGroupInfo.h"

CServerGroupInfo::CServerGroupInfo()
{
}

CServerGroupInfo::CServerGroupInfo(const CServerGroupInfo& right)
{
        m_sName = right.m_sName;
        m_vecServerList = right.m_vecServerList;
}

CServerGroupInfo::~CServerGroupInfo()
{
}

CServerGroupInfo& CServerGroupInfo::operator=(const CServerGroupInfo& right)
{
        if(this != &right)
        {
                m_sName = right.m_sName;
                m_vecServerList = right.m_vecServerList;
        }
        return *this;
}

