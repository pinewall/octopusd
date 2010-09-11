//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: SiteInfo.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: Kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/06/01 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef SITEINFO_H_HEADER_INCLUDED_BCE1E1EF
#define SITEINFO_H_HEADER_INCLUDED_BCE1E1EF

#include "ComFun.h"
#include "ServerInfo.h"

//站点信息
class CSiteInfo
{
public:
        CSiteInfo();
        CSiteInfo(const CSiteInfo& right);
        virtual ~CSiteInfo();
        CSiteInfo& operator=(const CSiteInfo& right);

        //站点名称
        string m_sName;

        // 需要推送的服务器列表
        vector<CServerInfo> m_vecServerList;

        //本站点对外服务的文件根目录
        string m_sRootDir;

};

#endif /* SITEINFO_H_HEADER_INCLUDED_BCE1E1EF */

