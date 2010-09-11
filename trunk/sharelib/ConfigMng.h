//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ConfigMng.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************
#ifndef CONFIGMNG_H_HEADER_INCLUDED_BCE1C7C8
#define CONFIGMNG_H_HEADER_INCLUDED_BCE1C7C8

#include "ComFun.h"
#include "Config.h"

#include "ComCfg.h"
#include "SiteInfo.h"
#include "ServerGroupInfo.h"

#include "LogMng.h"

//配置文件管理类
class CConfigMng
{
public:
        CConfigMng();
        virtual ~CConfigMng();

        /**
         * \brief 初始化配置文件
         * \param void
         * \return 0 成功 -1 出错
         */
        int init();

        /**
         * \brief 打印配置信息到配置文件
         * \param void
         * \return void
         */
        void print();

        /**
         * \brief 打印配置信息到配置文件
         * \param sSiteName 站点名称 sServerIP服务器IP iPort端口
         * \return 0 存在 -1不存在
         */
        int checkServerExist(string sSiteName, string sServerIP, int iPort);

        /**
         * \brief 根据站点名称，获得站点的root目录
         * \param sSiteName 站点的名称
         * \return 站点的root目录，如果为""则表示查找失败
         */
        string getSiteRootDir(string sSiteName);

        /**
         * \brief 从数据库中获得数据库
         * \param void
         * \return 0 成功 -1 出错
         */
        int readInfoFromDB(string sSiteId);

        /**
         * \brief 获得站点信息, 由于线程不安全,所以要进行锁控制
         * \param void
         * \return 0 成功 -1 出错
         */
        void getSiteMap(map<string, CSiteInfo>& m_mapSite);
        void getSiteServer(vector<CServerInfo>& vecServerInfo, const string& sSiteName);
        
        //公共配置项
        CComCfg m_tComCfg;

        //锁控制
        CThreadMutex  m_insShareMutex;

        //服务器列表
        map<string, CServerInfo> m_mapServer;
        
        //服务器组列表
        map<string, CServerGroupInfo> m_mapGroup;
        
        //站点信息列表
        map<string, CSiteInfo> m_mapSite;
};

#endif /* CONFIGMNG_H_HEADER_INCLUDED_BCE1C7C8 */

