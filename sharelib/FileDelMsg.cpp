//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: FileDelMsg.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "FileDelMsg.h"

CFileDelMsg::CFileDelMsg()
{
        m_sSiteName[0] = '\0';
        m_sDestPathFile[0] = '\0';
        m_sSendedTime[0] = '\0';
}

CFileDelMsg:: ~CFileDelMsg()
{
}

void CFileDelMsg::sureValidSrt()
{
        m_sSiteName[LenOfSiteName - 1] = '\0';
        m_sDestPathFile[LenOfPathFileName - 1] = '\0';
        m_sSendedTime[LenOfTime - 1] = '\0';
}

string CFileDelMsg::getPrintInfo()
{
        char sTmp[2048];
        sprintf(sTmp, "%s:%s:%s",
                m_sSiteName, m_sDestPathFile, m_sSendedTime);
        return sTmp;
}

//================

CFileMvCpMsg::CFileMvCpMsg()
{
        m_sSiteName[0] = '\0';
        m_sDestPathFile[0] = '\0';
        m_sSrcPathFile[0] = '\0';
        m_sSendedTime[0] = '\0';
}

CFileMvCpMsg:: ~CFileMvCpMsg()
{
}

void CFileMvCpMsg::sureValidSrt()
{
        m_sSiteName[LenOfSiteName - 1] = '\0';
        m_sDestPathFile[LenOfPathFileName - 1] = '\0';
        m_sSrcPathFile[LenOfPathFileName - 1] = '\0';
        m_sSendedTime[LenOfTime - 1] = '\0';
}

string CFileMvCpMsg::getPrintInfo()
{
        char sTmp[2048];
        sprintf(sTmp, "Dest=%s:%s;Src=%s:%s",
                m_sSiteName, m_sDestPathFile,  m_sSrcPathFile,m_sSendedTime);
        return sTmp;
}

