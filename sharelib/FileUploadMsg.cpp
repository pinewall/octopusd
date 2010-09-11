//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: FileUploadMsg.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "FileUploadMsg.h"

CFileUploadMsg::CFileUploadMsg()
{
        m_sSiteName[0] = '\0';
        m_sDestPathFile[0] = '\0';
        m_sSendedTime[0] = '\0';
        m_cDataType = '\0';
        m_sReserve[0] = '\0';
        m_iDataLength = 0;
}

CFileUploadMsg::~CFileUploadMsg()
{
}

void CFileUploadMsg::sureValidSrt()
{
        m_sSiteName[LenOfSiteName - 1] = '\0';
        m_sDestPathFile[LenOfPathFileName - 1] = '\0';
        m_sSendedTime[LenOfTime - 1] = '\0';
        m_sReserve[2] = '\0';
}

string CFileUploadMsg::getPrintInfo()
{
        char sTmp[2048];
        sprintf(sTmp, "%s:%s:%s:DATA=%02x:LENGTH=%d",
                m_sSiteName, m_sDestPathFile, m_sSendedTime, m_cDataType, m_iDataLength);
        return sTmp;
}

