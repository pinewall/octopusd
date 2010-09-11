//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ServerAddMsg.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "ServerAddMsg.h"

CServerAddMsg::CServerAddMsg()
{
}

CServerAddMsg::~CServerAddMsg()
{
}

void CServerAddMsg::sureValidSrt()
{
        m_sSiteName[LenOfSiteName - 1] = '\0';
        m_sServerIP[LenOfIPAddr - 1] = '\0';
        m_sTime[LenOfTime - 1] = '\0';
}

string CServerAddMsg::getPrintInfo()
{
        char sTmp[2048];
        sprintf(sTmp, "%s:%s:%d:%s",
                m_sSiteName, m_sServerIP, m_iPort, m_sTime);
        return sTmp;
}

