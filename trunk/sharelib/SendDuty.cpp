//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: SendDuty.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "SendDuty.h"

CSendDuty::CSendDuty()
{
        m_iState = Over;  
        m_iProtocolType = 0;
        m_sSiteName[0] = '\0';
        m_sDestPathFile[0] = '\0';
        m_sSendedTime[0]  = '\0';
        m_sLocalPathFile[0]  = '\0';
        m_iLocalTime = 0;  
        m_sServerIP[0]  = '\0';
        m_iServerPort = 0;
        m_iNrOfFail = 0;     
}

CSendDuty::CSendDuty(const CSendDuty& right)
{
        m_iState = right.m_iState;  
        m_iProtocolType = right.m_iProtocolType;
        strcpy(m_sSiteName, right.m_sSiteName);
        strcpy(m_sDestPathFile, right.m_sDestPathFile);
        strcpy(m_sSendedTime, right.m_sSendedTime);
        strcpy(m_sLocalPathFile, right.m_sLocalPathFile);
        m_iLocalTime = right.m_iLocalTime; 
        strcpy(m_sServerIP, right.m_sServerIP);
        m_iServerPort = right.m_iServerPort;
        m_iNrOfFail = right.m_iNrOfFail;   
}

CSendDuty::~CSendDuty()
{
}

CSendDuty& CSendDuty::operator=(const CSendDuty& right)
{
        if(this != &right)
        {
                m_iState = right.m_iState;  
                m_iProtocolType = right.m_iProtocolType;
                strcpy(m_sSiteName, right.m_sSiteName);
                strcpy(m_sDestPathFile, right.m_sDestPathFile);
                strcpy(m_sSendedTime, right.m_sSendedTime);
                strcpy(m_sLocalPathFile, right.m_sLocalPathFile);
                m_iLocalTime = right.m_iLocalTime; 
                strcpy(m_sServerIP, right.m_sServerIP);
                m_iServerPort = right.m_iServerPort;
                m_iNrOfFail = right.m_iNrOfFail;   
        }
        return *this;
}

string CSendDuty::getPrintInfo()
{
        char sTmp[2048];
        sprintf(sTmp, "STATUS=%d:PROTOCOL=%d:%s:%s:%s"
                ":%s:LOCALTIME=%d:%s:PORT=%d:NROFFAIL=%d",
                m_iState,
                m_iProtocolType,
                m_sSiteName,
                m_sDestPathFile,
                m_sSendedTime,
                m_sLocalPathFile,
                m_iLocalTime,
                m_sServerIP,
                m_iServerPort,
                m_iNrOfFail);
        return sTmp;
}

