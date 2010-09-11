//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ServerMng.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: Kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "ServerMng.h"

extern CConfigMng g_insConfigMng;
extern CLogMng g_insLogMng;

int CServerMng::addServer(CServerAddMsg& insServerAddMsg)
{
        //站点是否存在
        map<string, CSiteInfo> insMapSite;
        g_insConfigMng.getSiteMap(insMapSite);
        if (insMapSite.end() == insMapSite.find(insServerAddMsg.m_sSiteName))
        {
                //不存在,添加
                CSiteInfo insSiteInfo;                
                CServerInfo insServerInfo;
                //服务器信息
                strcpy(insServerInfo.m_sName, insServerAddMsg.m_sServerIP);
                strcpy(insServerInfo.m_sIP, insServerAddMsg.m_sServerIP);
                insServerInfo.m_iPort = insServerAddMsg.m_iPort;
                //站点信息
                insSiteInfo.m_sName = insServerAddMsg.m_sSiteName;
                insSiteInfo.m_vecServerList.clear();
                insSiteInfo.m_vecServerList.push_back(insServerInfo);
                insSiteInfo.m_sRootDir = string("/usr/local/qq_") + insSiteInfo.m_sName + "/htdocs";
                //站点登记
                g_insConfigMng.m_insShareMutex.setLock(); //锁控制
                g_insConfigMng.m_mapSite[insServerAddMsg.m_sSiteName] = insSiteInfo;
                g_insConfigMng.m_insShareMutex.unLock(); //锁控制
                return 0;
        }

        //检查是否存在相同的服务器端口和IP
        if (0 == g_insConfigMng.checkServerExist(insServerAddMsg.m_sSiteName, 
                insServerAddMsg.m_sServerIP, insServerAddMsg.m_iPort))
        {               
                g_insLogMng.normal("Site:%s Server(%s, %d) exist, think adding it successfully.",
                        insServerAddMsg.m_sSiteName, 
                        insServerAddMsg.m_sServerIP, 
                        insServerAddMsg.m_iPort);
                return 0;
        }
        
        //不存在,添加
        CServerInfo insServerInfo;
        strcpy(insServerInfo.m_sName, insServerAddMsg.m_sServerIP);
        strcpy(insServerInfo.m_sIP, insServerAddMsg.m_sServerIP);
        insServerInfo.m_iPort = insServerAddMsg.m_iPort;

        g_insConfigMng.m_insShareMutex.setLock(); //锁控制
        g_insConfigMng.m_mapSite[insServerAddMsg.m_sSiteName].m_vecServerList.push_back(insServerInfo);
        g_insConfigMng.m_insShareMutex.unLock(); //锁控制

        return 0;
}

int CServerMng::delServer(CServerDelMsg& insServerDelMsg)
{
        //站点是否存在
        map<string, CSiteInfo> insMapSite;
        g_insConfigMng.getSiteMap(insMapSite);
        if (insMapSite.end() == insMapSite.find(insServerDelMsg.m_sSiteName))
        {
                //不存在,认为删除成功
                g_insLogMng.normal("Site:%s  not exist, think deleting Server(%s, %d) successfully.",
                        insServerDelMsg.m_sSiteName, 
                        insServerDelMsg.m_sServerIP, 
                        insServerDelMsg.m_iPort);
                return 0;
        }

        g_insConfigMng.m_insShareMutex.setLock(); //锁控制
         //遍历查找这个频道下所有数据库, 已经判断过站点是否存在,此处不再判断,使用时注意
        for (vector<CServerInfo>::iterator it =  g_insConfigMng.m_mapSite[insServerDelMsg.m_sSiteName].m_vecServerList.begin();
                it !=  g_insConfigMng.m_mapSite[insServerDelMsg.m_sSiteName].m_vecServerList.end(); it++)
        {        
                 if (0 == strcmp(insServerDelMsg.m_sServerIP,  (*it).m_sIP)  && insServerDelMsg.m_iPort == (*it).m_iPort)
                {
                        g_insConfigMng.m_mapSite[insServerDelMsg.m_sSiteName].m_vecServerList.erase(it);
                        g_insConfigMng.m_insShareMutex.unLock(); //锁控制
                        return 0;
                }
        } 
        g_insConfigMng.m_insShareMutex.unLock(); //锁控制

        //不存在        
        g_insLogMng.normal("Site:%s Server(%s, %d) not exist, think deleting it successfully.",
                        insServerDelMsg.m_sSiteName, 
                        insServerDelMsg.m_sServerIP, 
                        insServerDelMsg.m_iPort);

        return 0;
}

int CServerMng::addSubServer()
{
        return 0;
}

int CServerMng::delSubServer()
{
        return 0;
}

