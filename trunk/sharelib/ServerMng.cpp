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
        //վ���Ƿ����
        map<string, CSiteInfo> insMapSite;
        g_insConfigMng.getSiteMap(insMapSite);
        if (insMapSite.end() == insMapSite.find(insServerAddMsg.m_sSiteName))
        {
                //������,���
                CSiteInfo insSiteInfo;                
                CServerInfo insServerInfo;
                //��������Ϣ
                strcpy(insServerInfo.m_sName, insServerAddMsg.m_sServerIP);
                strcpy(insServerInfo.m_sIP, insServerAddMsg.m_sServerIP);
                insServerInfo.m_iPort = insServerAddMsg.m_iPort;
                //վ����Ϣ
                insSiteInfo.m_sName = insServerAddMsg.m_sSiteName;
                insSiteInfo.m_vecServerList.clear();
                insSiteInfo.m_vecServerList.push_back(insServerInfo);
                insSiteInfo.m_sRootDir = string("/usr/local/qq_") + insSiteInfo.m_sName + "/htdocs";
                //վ��Ǽ�
                g_insConfigMng.m_insShareMutex.setLock(); //������
                g_insConfigMng.m_mapSite[insServerAddMsg.m_sSiteName] = insSiteInfo;
                g_insConfigMng.m_insShareMutex.unLock(); //������
                return 0;
        }

        //����Ƿ������ͬ�ķ������˿ں�IP
        if (0 == g_insConfigMng.checkServerExist(insServerAddMsg.m_sSiteName, 
                insServerAddMsg.m_sServerIP, insServerAddMsg.m_iPort))
        {               
                g_insLogMng.normal("Site:%s Server(%s, %d) exist, think adding it successfully.",
                        insServerAddMsg.m_sSiteName, 
                        insServerAddMsg.m_sServerIP, 
                        insServerAddMsg.m_iPort);
                return 0;
        }
        
        //������,���
        CServerInfo insServerInfo;
        strcpy(insServerInfo.m_sName, insServerAddMsg.m_sServerIP);
        strcpy(insServerInfo.m_sIP, insServerAddMsg.m_sServerIP);
        insServerInfo.m_iPort = insServerAddMsg.m_iPort;

        g_insConfigMng.m_insShareMutex.setLock(); //������
        g_insConfigMng.m_mapSite[insServerAddMsg.m_sSiteName].m_vecServerList.push_back(insServerInfo);
        g_insConfigMng.m_insShareMutex.unLock(); //������

        return 0;
}

int CServerMng::delServer(CServerDelMsg& insServerDelMsg)
{
        //վ���Ƿ����
        map<string, CSiteInfo> insMapSite;
        g_insConfigMng.getSiteMap(insMapSite);
        if (insMapSite.end() == insMapSite.find(insServerDelMsg.m_sSiteName))
        {
                //������,��Ϊɾ���ɹ�
                g_insLogMng.normal("Site:%s  not exist, think deleting Server(%s, %d) successfully.",
                        insServerDelMsg.m_sSiteName, 
                        insServerDelMsg.m_sServerIP, 
                        insServerDelMsg.m_iPort);
                return 0;
        }

        g_insConfigMng.m_insShareMutex.setLock(); //������
         //�����������Ƶ�����������ݿ�, �Ѿ��жϹ�վ���Ƿ����,�˴������ж�,ʹ��ʱע��
        for (vector<CServerInfo>::iterator it =  g_insConfigMng.m_mapSite[insServerDelMsg.m_sSiteName].m_vecServerList.begin();
                it !=  g_insConfigMng.m_mapSite[insServerDelMsg.m_sSiteName].m_vecServerList.end(); it++)
        {        
                 if (0 == strcmp(insServerDelMsg.m_sServerIP,  (*it).m_sIP)  && insServerDelMsg.m_iPort == (*it).m_iPort)
                {
                        g_insConfigMng.m_mapSite[insServerDelMsg.m_sSiteName].m_vecServerList.erase(it);
                        g_insConfigMng.m_insShareMutex.unLock(); //������
                        return 0;
                }
        } 
        g_insConfigMng.m_insShareMutex.unLock(); //������

        //������        
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

