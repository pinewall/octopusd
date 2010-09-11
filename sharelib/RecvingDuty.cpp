//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: RecvingDuty.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "RecvingDuty.h"

CRecvingDuty::CRecvingDuty()
{
        m_iState = Over;
        m_iProtocolType = 0;
        m_sSiteName[0] = '\0';
        m_sDestPathFile[0] = '\0';
        m_sSendedTime[0] = '\0';
}

CRecvingDuty::CRecvingDuty(const CRecvingDuty& right)
{
        m_iState = right.m_iState;
        m_iProtocolType = right.m_iProtocolType;
        strcpy(m_sSiteName, right.m_sSiteName);
        strcpy(m_sDestPathFile, right.m_sDestPathFile);
        strcpy(m_sSendedTime, right.m_sSendedTime);
}

CRecvingDuty::~CRecvingDuty()
{
}

CRecvingDuty& CRecvingDuty::operator=(const CRecvingDuty& right)
{
         if (this != &right)
        {
                m_iState = right.m_iState;
                m_iProtocolType = right.m_iProtocolType;
                strcpy(m_sSiteName, right.m_sSiteName);
                strcpy(m_sDestPathFile, right.m_sDestPathFile);
                strcpy(m_sSendedTime, right.m_sSendedTime);
        }
        return *this;
}

string CRecvingDuty::getPrintInfo()
{
        char sTmp[2048];
        sprintf(sTmp, "STATUS=%d:PROTOCOL=%d:%s:%s:%s",
                m_iState,
                m_iProtocolType,
                m_sSiteName,
                m_sDestPathFile,
                m_sSendedTime);
        return sTmp;
}


