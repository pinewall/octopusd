//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: SiteInfo.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "SiteInfo.h"

CSiteInfo::CSiteInfo()
{
}

CSiteInfo::CSiteInfo(const CSiteInfo& right)
{
        m_sName = right.m_sName;
        m_sRootDir = right.m_sRootDir;
        m_vecServerList = right.m_vecServerList;
}

CSiteInfo::~CSiteInfo()
{
}

CSiteInfo& CSiteInfo::operator=(const CSiteInfo& right)
{
        if(this != &right)
        {
                m_sName = right.m_sName;
                m_sRootDir = right.m_sRootDir;
                m_vecServerList = right.m_vecServerList;
        }
        return *this;
}

