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

#include "ConfigMng.h"

extern string g_sHomeDir;
extern CLogMng g_insLogMng;


CConfigMng::CConfigMng()
{
        //�����ʼ����������Ա�����Լ��ĳ�ʼ������
}

CConfigMng::~CConfigMng()
{
}

//ϵͳ���¶�ȡ����ˢ�¶�����ʱ�����в���,��˲�������̲߳���ȫ������,�ʶ�����������
int CConfigMng::init()
{
        //���һ�¿��ܴ��ڵ��ϵļ�¼
        m_mapServer.clear();
        m_mapGroup.clear();
        m_mapSite.clear();

        //��һ����
        // --- 1 --- common�����ļ���·�������ļ���
        string sCommonCfgName = g_sHomeDir + "/cfg/common.cfg";
        g_insLogMng.normal("Begin to read %s ......", sCommonCfgName.c_str());

        //��ʼ�������ļ�
        CConfig  insCfgCommon(sCommonCfgName);
        int iRet = insCfgCommon.Init();
        if(iRet != 0)
        {
                g_insLogMng.error("Read cfg file: %s failed, bad line = %d",
                        sCommonCfgName.c_str(), iRet);
                return -1;
        }

        //��ȡ����������
        char tmpstr[1024];
        int iLoop;
        char sTmpSec[64 + 1];
        // [LOGIN]
        //   LoginIpGroup = 172.16.19.25|172.16.19.57|219.133.41.67|172.18.8.83
        insCfgCommon.ReadItem("LOGIN","LoginIpGroup","", tmpstr);
        if ('\0' == tmpstr[0])
        {
                g_insLogMng.error("Get [LOGIN] LoginIpGroup(%s) failed.", tmpstr);
                return  -1;
        }
        strcpy(m_tComCfg.m_sLoginIP, tmpstr);

        //  ServerFlag = BJ78,�ɿ�
        insCfgCommon.ReadItem("LOGIN","ServerFlag","", tmpstr);
        strcpy(m_tComCfg.m_sServerFlag, tmpstr);

        // [COMMON]
        //   LogMode = 0 ~ 3
       insCfgCommon.ReadItem("COMMON","LogMode","", tmpstr);
        m_tComCfg.m_iLogMode = atoi(tmpstr);
        if (0 > m_tComCfg.m_iLogMode || 3 < m_tComCfg.m_iLogMode)
        {
                g_insLogMng.error("Get [COMMON] LogMode(%s) failed.", tmpstr);
                return  -1;
        }

        //  FileModeOption = 666,�ɿ�
        insCfgCommon.ReadItem("COMMON","FileModeOption","", tmpstr);
        //���ܳ���4λ
        if (4 < strlen(tmpstr))
        {
                g_insLogMng.error("Get [COMMON] FileModeOption(%s) failed.", tmpstr);
                return  -1;
        }
        strcpy(m_tComCfg.m_sFileModeOption, tmpstr);

         //   m_iDelFileAfterUpload = 0
        insCfgCommon.ReadItem("COMMON","DelFileAfterUpload","", tmpstr);
        m_tComCfg.m_iDelFileAfterUpload = atoi(tmpstr);
        if (0 > m_tComCfg.m_iDelFileAfterUpload || 1 < m_tComCfg.m_iDelFileAfterUpload)
        {
                g_insLogMng.error("Get [COMMON] DelFileAfterUpload(%s) failed.", tmpstr);
                return  -1;
        }

        //   m_iIfUploadZeroFile = 1  Ĭ��Ϊ��Ȼ�����ϴ�
        insCfgCommon.ReadItem("COMMON","IfUploadZeroFile","", tmpstr);
        if(strlen(tmpstr) == 0)
        {
                m_tComCfg.m_iIfUploadZeroFile = 1;
        }
        else
        {
                m_tComCfg.m_iIfUploadZeroFile = atoi(tmpstr);
        }
        if (0 > m_tComCfg.m_iIfUploadZeroFile || 1 < m_tComCfg.m_iIfUploadZeroFile)
        {
                g_insLogMng.error("Get [COMMON] m_iIfUploadZeroFile(%s) failed.", tmpstr);
                return  -1;
        }

        //MaxLimitOfUploadFile
        insCfgCommon.ReadItem("COMMON","MaxLimitOfUploadFile","", tmpstr);
         m_tComCfg.m_iMaxLimitOfUploadFile = atoi(tmpstr) * 1024;
        if (0 > m_tComCfg.m_iMaxLimitOfUploadFile)
        {
                g_insLogMng.error("Get [COMMON] MaxLimitOfUploadFile(%s) failed.", tmpstr);
                return  -1;
        }

        //   ProtocolType = 0~int
        insCfgCommon.ReadItem("COMMON","ProtocolType","", tmpstr);
        m_tComCfg.m_iProtocolType = atoi(tmpstr);
        if (0 > m_tComCfg.m_iProtocolType)
        {
                g_insLogMng.error("Get [COMMON] ProtocolType(%s) failed.", tmpstr);
                return  -1;
        }

        //   MaxNrOfRetry = 0~int
        insCfgCommon.ReadItem("COMMON","MaxNrOfRetry","", tmpstr);
        m_tComCfg.m_iMaxNrOfRetry = atoi(tmpstr);
        if (0 > m_tComCfg.m_iMaxNrOfRetry)
        {
                g_insLogMng.error("Get [COMMON] MaxNrOfRetry(%s) failed.", tmpstr);
                return  -1;
        }

        //   IfFailLogFlag = 0, 1
        insCfgCommon.ReadItem("COMMON","IfFailLogFlag","", tmpstr);
        m_tComCfg.m_iIfFailLogFlag = atoi(tmpstr);
        if (0 > m_tComCfg.m_iIfFailLogFlag || 1 < m_tComCfg.m_iIfFailLogFlag )
        {
                g_insLogMng.error("Get [COMMON] IfFailLogFlag(%s) failed.", tmpstr);
                return  -1;
        }

        //   MaxNrOfQueueDuty = 0,int
        insCfgCommon.ReadItem("COMMON","MaxNrOfQueueDuty","", tmpstr);
        m_tComCfg.m_iMaxNrOfQueueDuty = atoi(tmpstr);
        if (0 > m_tComCfg.m_iMaxNrOfQueueDuty)
        {
                g_insLogMng.error("Get [COMMON] MaxNrOfQueueDuty(%s) failed.", tmpstr);
                return  -1;
        }

        //   MaxNrOfSaveLastSucDuty = 0,int
        insCfgCommon.ReadItem("COMMON","MaxNrOfSaveLastSucDuty","", tmpstr);
        m_tComCfg.m_iMaxNrOfSaveLastSucDuty = atoi(tmpstr);
        if (0 > m_tComCfg.m_iMaxNrOfSaveLastSucDuty)
        {
                g_insLogMng.error("Get [COMMON] MaxNrOfSaveLastSucDuty(%s) failed.", tmpstr);
                return  -1;
        }

        //TimeOfFailDutyRetry
        insCfgCommon.ReadItem("COMMON","TimeOfFailDutyRetry","", tmpstr);
        m_tComCfg.m_iTimeOfFailDutyRetry = atoi(tmpstr);
        if (0 > m_tComCfg.m_iTimeOfFailDutyRetry)
        {
                g_insLogMng.error("Get [COMMON] TimeOfFailDutyRetry(%s) failed.", tmpstr);
                return  -1;
        }

         //TimeOfCheckRunStatus
        insCfgCommon.ReadItem("COMMON","TimeOfCheckRunStatus","", tmpstr);
        m_tComCfg.m_iTimeOfCheckRunStatus = atoi(tmpstr);
        if (0 > m_tComCfg.m_iTimeOfCheckRunStatus)
        {
                g_insLogMng.error("Get [COMMON] TimeOfCheckRunStatus(%s) failed.", tmpstr);
                return  -1;
        }

        insCfgCommon.ReadItem("COMMON","MaxNrOfDutyToAlarm","", tmpstr);
        m_tComCfg.m_iMaxNrOfDutyToAlarm= atoi(tmpstr);
        if (m_tComCfg.m_iMaxNrOfDutyToAlarm == 0)
        {
                m_tComCfg.m_iMaxNrOfDutyToAlarm = 5000; //��ʧ�ܶ��еĴ��ڣ�������Ĭ�����ƷŴ�5k
        }
        else if (20 > m_tComCfg.m_iMaxNrOfDutyToAlarm)
        {                
                g_insLogMng.error("Get [COMMON] MaxNrOfDutyToAlarm(%s) failed.", tmpstr);
                return  -1;
        }
        //TimeOfThreadStat
        insCfgCommon.ReadItem("COMMON","TimeOfThreadStat","", tmpstr);
        m_tComCfg.m_iTimeOfThreadStat = atoi(tmpstr);
        if (0 > m_tComCfg.m_iTimeOfThreadStat)
        {
                g_insLogMng.error("Get [COMMON] TimeOfThreadStat(%s) failed.", tmpstr);
                return  -1;
        }

        //   DistributeFlag = 0,1,2
        insCfgCommon.ReadItem("COMMON","DistributeFlag","", tmpstr);
        m_tComCfg.m_iDistributeFlag = atoi(tmpstr);
        if (0 > m_tComCfg.m_iDistributeFlag || 2 < m_tComCfg.m_iDistributeFlag )
        {
                g_insLogMng.error("Get [COMMON] DistributeFlag(%s) failed.", tmpstr);
                return  -1;
        }

        if (1 == m_tComCfg.m_iDistributeFlag)
        {
                //   DBHost = 172.16.19.145
                insCfgCommon.ReadItem("COMMON","DBHost","", tmpstr);
                if ('\0' == tmpstr[0])
                {
                        g_insLogMng.error("Get [COMMON] DBHost(%s) failed.", tmpstr);
                        return  -1;
                }
                if (0 != CComFun::checkIP(tmpstr))
                {
                         g_insLogMng.error("[COMMON] DBHost(%s) invalid IP.", tmpstr);
                         return  -1;
                }
                strcpy(m_tComCfg.m_sDBHost, tmpstr);

                //  DBUser = root
                insCfgCommon.ReadItem("COMMON","DBUser","", tmpstr);
                if ('\0' == tmpstr[0])
                {
                        g_insLogMng.error("Get [COMMON] DBUser(%s) failed.", tmpstr);
                        return  -1;
                }
                strcpy(m_tComCfg.m_sDBUser, tmpstr);

                // #DBPassword = root@club410.4
                insCfgCommon.ReadItem("COMMON","DBPassword","", tmpstr);
                strcpy(m_tComCfg.m_sDBPassword, tmpstr);

        }

        // [SERVER]
        //   ServerIP = 127.0.0.1
        insCfgCommon.ReadItem("SERVER","ServerIP","", tmpstr);
        if ('\0' == tmpstr[0])
        {
                g_insLogMng.error("Get [SERVER] ServerIP(%s) failed.", tmpstr);
                return  -1;
        }
        if (0 == strcmp("all", tmpstr) || 0 == strcmp("ALL", tmpstr)) //��ʾ�����ϵ�����IP
        {
        }
        else  //ʵ��IP
        {
                if (0 != CComFun::checkIP(tmpstr))
                {
                         g_insLogMng.error("[SERVER] ServerIP(%s) invalid.", tmpstr);
                         return  -1;
                }
                //�����IP��ӵ������ε�IP��
                strcat(m_tComCfg.m_sLoginIP, tmpstr);
        }
        strcpy(m_tComCfg.m_sServerIP, tmpstr);

        // ServerPort = 8123   1000~65535
        insCfgCommon.ReadItem("SERVER","ServerPort","", tmpstr);
        m_tComCfg.m_iServerPort = atoi(tmpstr);
        if (1000 > m_tComCfg.m_iServerPort || 65535 < m_tComCfg.m_iServerPort)
        {
                g_insLogMng.error("Get [SERVER] ServerPort(%s) failed.", tmpstr);
                return  -1;
        }

        //   RecvNrOfThread = 20   0~int
        insCfgCommon.ReadItem("SERVER","RecvNrOfThread","", tmpstr);
        m_tComCfg.m_iRecvNrOfThread = atoi(tmpstr);
        if (0 >= m_tComCfg.m_iRecvNrOfThread)
        {
                g_insLogMng.error("Get [SERVER] RecvNrOfThread(%s) failed.", tmpstr);
                return  -1;
        }

        //   SendNrOfThread = 40  ,0 ~int
        insCfgCommon.ReadItem("SERVER","SendNrOfThread","", tmpstr);
        m_tComCfg.m_iSendNrOfThread = atoi(tmpstr);
        if (0 >= m_tComCfg.m_iSendNrOfThread)
        {
                g_insLogMng.error("Get [SERVER] SendNrOfThread(%s) failed.", tmpstr);
                return  -1;
        }

        //   SendNrOfThread = 40  ,1 ~int
        insCfgCommon.ReadItem("SERVER","SendErrorNrOfThread","", tmpstr);
        m_tComCfg.m_iSendErrorNrOfThread = atoi(tmpstr);
        if (0 == m_tComCfg.m_iSendErrorNrOfThread)
        {
                m_tComCfg.m_iSendErrorNrOfThread = 5;  //Ĭ��
        }
        if (0 > m_tComCfg.m_iSendErrorNrOfThread)
        {
                g_insLogMng.error("Get [SERVER] SendNrOfThread(%s) failed.", tmpstr);
                return  -1;
        }

        //   MaxNrOfCnt = 1000
        insCfgCommon.ReadItem("SERVER","MaxNrOfCnt","", tmpstr);
        m_tComCfg.m_iMaxNrOfCnt= atoi(tmpstr);
        if (0 > m_tComCfg.m_iMaxNrOfCnt)
        {
                g_insLogMng.error("Get [SERVER] MaxNrOfCnt(%s) failed.", tmpstr);
                return  -1;
        }

        //   RecvMsgTimeout = 10
        insCfgCommon.ReadItem("SERVER","RecvMsgTimeout","", tmpstr);
        m_tComCfg.m_iRecvMsgTimeout = atoi(tmpstr);
        if (0 > m_tComCfg.m_iRecvMsgTimeout)
        {
                g_insLogMng.error("Get [SERVER] RecvMsgTimeout(%s) failed.", tmpstr);
                return  -1;
        }

        //   RecvFileTimeout = 10
        insCfgCommon.ReadItem("SERVER","RecvFileTimeout","", tmpstr);
        m_tComCfg.m_iRecvFileTimeout = atoi(tmpstr);
        if (0 > m_tComCfg.m_iRecvFileTimeout)
        {
                g_insLogMng.error("Get [SERVER] RecvFileTimeout(%s) failed.", tmpstr);
                return  -1;
        }

        //   SendFlowCtrl = 100K
        insCfgCommon.ReadItem("SERVER","SendFlowCtrl","", tmpstr);
        m_tComCfg.m_iSendFlowCtrl = atoi(tmpstr);
        if (0 > m_tComCfg.m_iSendFlowCtrl)
        {
                g_insLogMng.error("Get [SERVER] SendFlowCtrl(%s) failed.", tmpstr);
                return  -1;
        }

        g_insLogMng.normal("Read %s Successfully.", sCommonCfgName.c_str());


        //�ڶ�����
        if (1 == m_tComCfg.m_iDistributeFlag)
        {
                g_insLogMng.normal("Begin to read server info from DB......");
                if ( 0 != readInfoFromDB(""))
                {
                        g_insLogMng.error("Read site info from DB failed.");
                        return -1;
                }
                g_insLogMng.normal("Read server info from DB successfully.");
        }
        else if (2 == m_tComCfg.m_iDistributeFlag)
        {
                // --- 2 --- siteinfo�����ļ���·�������ļ���
                string sSiteInfoCfgName = g_sHomeDir + "/cfg/siteinfo.cfg";
                g_insLogMng.normal("Begin to read %s ......", sSiteInfoCfgName.c_str());

                //��ʼ�������ļ�
                CConfig  insCfgSiteInfo(sSiteInfoCfgName);
                iRet = insCfgSiteInfo.Init();
                if(iRet != 0)
                {
                        g_insLogMng.error("Init %s failed, bad line = %d",
                                sSiteInfoCfgName.c_str(), iRet);
                        return -1;
                }

                // [SERVERINFO]
                //   NrOfServer = 5
                int iNrOfServer = 0;
                insCfgSiteInfo.ReadItem("SERVERINFO","NrOfServer","", tmpstr);
                iNrOfServer= atoi(tmpstr);
                if (0 > iNrOfServer)
                {
                        g_insLogMng.error("Get [SERVERINFO] NrOfServer(%s) failed.", tmpstr);
                        return  -1;
                }

                // [SERVER001]
                CServerInfo insServerInfo;
                for (iLoop = 1; iLoop <= iNrOfServer; iLoop++)
                {
                        sprintf(sTmpSec, "SERVER%03d", iLoop);
                        // ServerName = bjweb1
                        insCfgSiteInfo.ReadItem(sTmpSec, "ServerName","", tmpstr);
                        if ('\0' == tmpstr[0])
                        {
                                g_insLogMng.error("Get [%s] ServerName(%s) failed.", sTmpSec, tmpstr);
                                return  -1;
                        }
                        strcpy(insServerInfo.m_sName, tmpstr);
                        //����Ƿ���ڣ�ͬ���ķ�����
                        if (m_mapServer.end() != m_mapServer.find(insServerInfo.m_sName))
                        {
                                 g_insLogMng.error("The same servername([%s], %s) exist.", sTmpSec, insServerInfo.m_sName);
                                 return  -1;
                        }

                        //IP = 172.18.8.61
                        insCfgSiteInfo.ReadItem(sTmpSec, "IP","", tmpstr);
                       if ('\0' == tmpstr[0])
                        {
                                g_insLogMng.error("Get [%s] IP(%s) failed.", sTmpSec, tmpstr);
                                return  -1;
                        }
                        if (0 != CComFun::checkIP(tmpstr))
                        {
                                 g_insLogMng.error("[%s] IP(%s) invalid.", sTmpSec, tmpstr);
                                 return  -1;
                        }
                        strcpy(insServerInfo.m_sIP, tmpstr);

                        //Port = 21800
                        insCfgSiteInfo.ReadItem(sTmpSec, "Port","", tmpstr);
                        insServerInfo.m_iPort = atoi(tmpstr);
                        if (1000 > insServerInfo.m_iPort || 65535 < insServerInfo.m_iPort )
                        {
                                g_insLogMng.error("Get [%s] Port(%s) failed.", sTmpSec, tmpstr);
                                return  -1;
                        }

                        //�����������Ϣ
                        m_mapServer[insServerInfo.m_sName] = insServerInfo;
                }

                // [SERVERGROUP]
                //   NrOfServerGroup = 5
                int iNrOfServerGroup = 0;
                insCfgSiteInfo.ReadItem("SERVERGROUP","NrOfServerGroup","", tmpstr);
                iNrOfServerGroup= atoi(tmpstr);
                if (0 > iNrOfServerGroup)
                {
                        g_insLogMng.error("Get [SERVERGROUP] NrOfServerGroup(%s) failed.", tmpstr);
                        return  -1;
                }

                // [SERVERGROUP001]
                CServerGroupInfo insServerGroupInfo;
                for (iLoop = 1; iLoop <= iNrOfServerGroup; iLoop++)
                {
                        sprintf(sTmpSec, "SERVERGROUP%03d", iLoop);
                        // GroupName = ChannelServerGroup
                        insCfgSiteInfo.ReadItem(sTmpSec, "GroupName","", tmpstr);
                        if ('\0' == tmpstr[0])
                        {
                                g_insLogMng.error("Get [%s] GroupName(%s) failed.", sTmpSec, tmpstr);
                                return  -1;
                        }
                        insServerGroupInfo.m_sName = tmpstr;
                        //����Ƿ���ڣ�ͬ���ķ�������
                        if (m_mapGroup.end() != m_mapGroup.find(insServerGroupInfo.m_sName))
                        {
                                 g_insLogMng.error("The same group name([%s], %s) exist.", sTmpSec, insServerGroupInfo.m_sName.c_str());
                                 return  -1;
                        }

                        insCfgSiteInfo.ReadItem(sTmpSec, "ServerList","", tmpstr);
                        if ('\0' == tmpstr[0])
                        {
                                g_insLogMng.normal("Warning:Get [%s] ServerList = null.", sTmpSec, tmpstr);
                        }
                        else
                        {
                                CComFun::divideupStr(tmpstr, insServerGroupInfo.m_vecServerList, '|');
                        }

                        //���������б��Ƿ��������ļ��д���
                        for(vector<string>::iterator it = insServerGroupInfo.m_vecServerList.begin();
                                it != insServerGroupInfo.m_vecServerList.end(); it++)
                        {
                                //����Ƿ���ڣ�ͬ���ķ�����
                                if (m_mapServer.end() == m_mapServer.find(*it))
                                {
                                         g_insLogMng.error("[%s] ServerList's %s server not exist.", sTmpSec, (*it).c_str());
                                         return  -1;
                                }
                        }

                        //�������������Ϣ
                        m_mapGroup[insServerGroupInfo.m_sName] = insServerGroupInfo;
                        insServerGroupInfo.m_vecServerList.clear();

                }
                // [SITEINFO]
                //   NrOfSite = 5
                int iNrOfSite = 0;
                insCfgSiteInfo.ReadItem("SITEINFO","NrOfSite","", tmpstr);
                iNrOfSite= atoi(tmpstr);
                if (0 > iNrOfSite)
                {
                        g_insLogMng.error("Get [SITEINFO] NrOfSite(%s) failed.", tmpstr);
                        return  -1;
                }

                // [SITE001]
                CSiteInfo  insSiteInfo;
                for (iLoop = 1; iLoop <= iNrOfSite; iLoop++)
                {
                        sprintf(sTmpSec, "SITE%03d", iLoop);
                        // SiteName = ent
                        insCfgSiteInfo.ReadItem(sTmpSec, "SiteName","", tmpstr);
                        if ('\0' == tmpstr[0])
                        {
                                g_insLogMng.error("Get [%s] SiteName(%s) failed.", sTmpSec, tmpstr);
                                return  -1;
                        }
                        insSiteInfo.m_sName = tmpstr;

                        //�����Ƿ����,ͬ����վ��
                        if (m_mapSite.end() != m_mapSite.find(insSiteInfo.m_sName))
                        {
                                 g_insLogMng.error("The same site name([%s], %s) exist.", sTmpSec, insSiteInfo.m_sName.c_str());
                                 return  -1;
                        }

                        //ServerGroup = ChannelServerGroup
                        insCfgSiteInfo.ReadItem(sTmpSec, "ServerGroup","", tmpstr);
                        if ('\0' == tmpstr[0])
                        {
                                g_insLogMng.error("Get [%s] ServerGroup(%s) failed.", sTmpSec, tmpstr);
                                return  -1;
                        }

                        //�������ڷ������б�
                        map<string, CServerGroupInfo>::iterator itx = m_mapGroup.find(tmpstr);
                        if (m_mapGroup.end() != itx)
                        {
                                for (vector<string>::iterator ity =  (itx->second).m_vecServerList.begin();
                                        ity !=  (itx->second).m_vecServerList.end(); ity++)
                                {
                                        insSiteInfo.m_vecServerList.push_back(m_mapServer[*ity]);
                                }
                        }
                        else
                        {
                                g_insLogMng.error("[%s] ServerGroup(%s) not exist.", sTmpSec, tmpstr);
                                return  -1;
                        }

                        //DocumentRoot = /data1/qq_joke/htdocs
                        insCfgSiteInfo.ReadItem(sTmpSec, "DocumentRoot","", tmpstr);
                        if ('\0' == tmpstr[0])
                        {
                                g_insLogMng.error("Get [%s] DocumentRoot(%s) failed.", sTmpSec, tmpstr);
                                return  -1;
                        }
                        insSiteInfo.m_sRootDir = tmpstr;

                        //����վ����Ϣ
                        m_mapSite[insSiteInfo.m_sName] = insSiteInfo;

                        //�������ʱ�����е�����
                        insSiteInfo.m_vecServerList.clear();

                }
                g_insLogMng.normal("Read %s successfully.", sSiteInfoCfgName.c_str());
        }
        //���ַ��ͷַ�����Ҫ��ȡroot_dir��Ϣ
        else if (0 == m_tComCfg.m_iDistributeFlag)
        {
                // --- 2 --- siteinfo�����ļ���·�������ļ���
                string sSiteInfoCfgName = g_sHomeDir + "/cfg/siteinfo.cfg";
                g_insLogMng.normal("Begin to read %s ......", sSiteInfoCfgName.c_str());

                //��ʼ�������ļ�
                CConfig  insCfgSiteInfo(sSiteInfoCfgName);
                iRet = insCfgSiteInfo.Init();
                if(iRet != 0)
                {
                        g_insLogMng.error("Init %s failed, bad line = %d",
                                sSiteInfoCfgName.c_str(), iRet);
                        return -1;
                }

                // [SITEINFO]
                //   NrOfSite = 5
                int iNrOfSite = 0;
                insCfgSiteInfo.ReadItem("SITEINFO","NrOfSite","", tmpstr);
                iNrOfSite= atoi(tmpstr);
                if (0 > iNrOfSite)
                {
                        g_insLogMng.error("Get [SITEINFO] NrOfSite(%s) failed.", tmpstr);
                        return  -1;
                }

                // [SITE001]
                CSiteInfo  insSiteInfo;
                for (iLoop = 1; iLoop <= iNrOfSite; iLoop++)
                {
                        sprintf(sTmpSec, "SITE%03d", iLoop);
                        // SiteName = ent
                        insCfgSiteInfo.ReadItem(sTmpSec, "SiteName","", tmpstr);
                        if ('\0' == tmpstr[0])
                        {
                                g_insLogMng.error("Get [%s] SiteName(%s) failed.", sTmpSec, tmpstr);
                                return  -1;
                        }
                        insSiteInfo.m_sName = tmpstr;

                        //�����Ƿ����,ͬ����վ��
                        if (m_mapSite.end() != m_mapSite.find(insSiteInfo.m_sName))
                        {
                                 g_insLogMng.error("The same site name([%s], %s) exist.", sTmpSec, insSiteInfo.m_sName.c_str());
                                 return  -1;
                        }

                        //DocumentRoot = /data1/qq_joke/htdocs
                        insCfgSiteInfo.ReadItem(sTmpSec, "DocumentRoot","", tmpstr);
                        if ('\0' == tmpstr[0])
                        {
                                g_insLogMng.error("Get [%s] DocumentRoot(%s) failed.", sTmpSec, tmpstr);
                                return  -1;
                        }
                        insSiteInfo.m_sRootDir = tmpstr;

                        //����վ����Ϣ
                        m_mapSite[insSiteInfo.m_sName] = insSiteInfo;
                }
                g_insLogMng.normal("Read %s successfully.", sSiteInfoCfgName.c_str());
        }

        return 0;

}

void CConfigMng::print()
{
        m_insShareMutex.setLock(); //������

        char sTmpStr[65535] = "";
        //Common
        string sPrtInfo = "[LOGIN]\n LoginIpGroup = ";
        //unsigned int iLoop;
        sprintf(sTmpStr,  "%s%s\nServerFlag=%s\n", sTmpStr, m_tComCfg.m_sLoginIP, m_tComCfg.m_sServerFlag);
        sPrtInfo = sPrtInfo + sTmpStr + "\n[COMMON]\n" ;

        sprintf(sTmpStr, " LogMode = %d\n FileModeOption=%s\n DelFileAfterUpload = %d\n IfUploadZeroFile = %d\n MaxLimitOfUploadFile = %d\n "
                "ProtocolType = %d\n MaxNrOfRetry = %d\n IfFailLogFlag = %d\n"
                " MaxNrOfQueueDuty = %d\n MaxNrOfSaveLastSucDuty = %d\n"
                " TimeOfThreadStat= %d\n TimeOfFailDutyRetry = %d\n TimeOfCheckRunStatus = %d\n"
                " MaxNrOfDutyToAlarm = %d\n"
                " DistributeFlag= %d\n DBHost = %s\n"
                " DBUser = %s\n DBPassword = %s\n [SERVER]\n ServerIP = %s\n"
                " ServerPort = %d\n RecvNrOfThread = %d\n SendNrOfThread = %d\n SendErrorNrOfThread = %d\n"
                " MaxNrOfCnt = %d\n RecvMsgTimeout = %d\n"
                " RecvFileTimeout = %d\n SendFlowCtrl = %dKB\n",
                m_tComCfg.m_iLogMode, m_tComCfg.m_sFileModeOption, m_tComCfg.m_iDelFileAfterUpload, m_tComCfg.m_iIfUploadZeroFile, m_tComCfg.m_iMaxLimitOfUploadFile,
                m_tComCfg.m_iProtocolType, m_tComCfg.m_iMaxNrOfRetry, m_tComCfg.m_iIfFailLogFlag,
                m_tComCfg.m_iMaxNrOfQueueDuty, m_tComCfg.m_iMaxNrOfSaveLastSucDuty,
                m_tComCfg.m_iTimeOfThreadStat, m_tComCfg.m_iTimeOfFailDutyRetry, m_tComCfg.m_iTimeOfCheckRunStatus,
                m_tComCfg.m_iMaxNrOfDutyToAlarm,
                m_tComCfg.m_iDistributeFlag, m_tComCfg.m_sDBHost,
                m_tComCfg.m_sDBUser, m_tComCfg.m_sDBPassword,  m_tComCfg.m_sServerIP,
                m_tComCfg.m_iServerPort, m_tComCfg.m_iRecvNrOfThread, m_tComCfg.m_iSendNrOfThread,m_tComCfg.m_iSendErrorNrOfThread,
                m_tComCfg.m_iMaxNrOfCnt, m_tComCfg.m_iRecvMsgTimeout,
                m_tComCfg.m_iRecvFileTimeout, m_tComCfg.m_iSendFlowCtrl);
        sPrtInfo = sPrtInfo + sTmpStr;

#if 0
        //Server info
        sTmpStr[0] = '\0';
        iLoop = 1;
        for (map<string, CServerInfo>::iterator itServer = m_mapServer.begin();
                itServer != m_mapServer.end(); itServer++)
        {
                sprintf(sTmpStr,"%s[SERVER%03d]\n ServerName = %s\n IP = %s\n Port = %d\n",
                        sTmpStr,
                        iLoop, itServer->second.m_sName.c_str(),
                        itServer->second.m_sIP.c_str(),
                        itServer->second.m_iPort);
                iLoop++;
        }
        sPrtInfo = sPrtInfo + sTmpStr;

        //Server Group
        sTmpStr[0] = '\0';
        iLoop = 1;
        for (map<string, CServerGroupInfo>::iterator itServerG = m_mapGroup.begin();
                itServerG != m_mapGroup.end(); itServerG++)
        {
                sprintf(sTmpStr,"%s[SERVERGROUP%03d]\n GroupName = %s\n ServerList = ",
                        sTmpStr,
                        iLoop, itServerG->second.m_sName.c_str());
                for(vector<string>::iterator itGx =  itServerG->second.m_vecServerList.begin();
                        itGx != itServerG->second.m_vecServerList.end();itGx++)
                {
                        sprintf(sTmpStr,"%s%s|",  sTmpStr, (*itGx).c_str());
                }
                strcat(sTmpStr, "\n");
                iLoop++;
        }
        sPrtInfo = sPrtInfo + sTmpStr;
#endif

        //site info
        sTmpStr[0] = '\0';
        int kLoop = 1;
        for(map<string, CSiteInfo>::iterator it = m_mapSite.begin();
                it != m_mapSite.end(); it++)
        {
                sprintf(sTmpStr,"%s[SITE%03d]\n SiteName = %s\n",
                        sTmpStr, kLoop, it->second.m_sName.c_str());
                for(unsigned int jLoop = 0; jLoop < it->second.m_vecServerList.size(); jLoop++)
                {
                        sprintf(sTmpStr,"%s IP[%d] = %s\n Port = %d\n",
                                sTmpStr,
                                jLoop,
                                it->second.m_vecServerList[jLoop].m_sIP,
                                it->second.m_vecServerList[jLoop].m_iPort);
                }
                sprintf(sTmpStr, "%s RootDir=%s\n", sTmpStr, it->second.m_sRootDir.c_str());
                kLoop++;  //��һ��
        }
        sPrtInfo = sPrtInfo + sTmpStr;

        m_insShareMutex.unLock();  // ������

        //�Լ�дһ���������ļ�
        string myFile = g_sHomeDir + "/log/prt/config.prt";
        FILE *fp = fopen(myFile.c_str(), "w");
        if (NULL == fp)
        {
                return;
        }

        fputs(sPrtInfo.c_str(), fp);
        fclose(fp);

        return;
}

string CConfigMng::getSiteRootDir(string sSiteName)
{
        m_insShareMutex.setLock();  //������
        map<string, CSiteInfo>::iterator it = m_mapSite.find(sSiteName);
        if (it != m_mapSite.end())
        {
                string sRootDir = it->second.m_sRootDir;
                m_insShareMutex.unLock(); //������
                return  sRootDir;
        }
        m_insShareMutex.unLock(); //������
        return "";
}

int CConfigMng::checkServerExist(string sSiteName, string sServerIP, int iPort)
{
        m_insShareMutex.setLock();  // ������
        //�����������Ƶ�����������ݿ�, �Ѿ��жϹ�վ���Ƿ����,�˴������ж�,ʹ��ʱע��
        for (vector<CServerInfo>::iterator it =  m_mapSite[sSiteName].m_vecServerList.begin();
                it !=  m_mapSite[sSiteName].m_vecServerList.end(); it++)
        {
                 if (0 == strcmp(sServerIP.c_str(), (*it).m_sIP)  && iPort == (*it).m_iPort)
                {
                        m_insShareMutex.unLock();  // ������
                        return 0;
                }
        }
        m_insShareMutex.unLock();  // ������
        return -1;
}

int  CConfigMng::readInfoFromDB(string sSiteId)
{
#ifndef NODB  //����Ƿ����ݿ�ģʽ����������ݿ���Ϣ
        MYSQL* mysql;
        char sQuery[1024];
        MYSQL_RES* mysql_res;
        MYSQL_ROW stRow;

        //�������ݿ�
        mysql = mysql_init(NULL);
        if(!mysql_real_connect(mysql, m_tComCfg.m_sDBHost,m_tComCfg.m_sDBUser,m_tComCfg.m_sDBPassword, "site_config",0,NULL,0))
        {
                g_insLogMng.error("Connect to database(site_config) failed, reason = %s", mysql_error(mysql));
                return -1;
        }

        //���ҷ�������Ϣ
        if ( "" != sSiteId)  //��������
        {
                sprintf(sQuery, "select Fsite_id, Fip, Fport ,Fremote from t_site_server where Fsite_id = %s", sSiteId.c_str());
        }
        else  //ȫ������
        {
                sprintf(sQuery,"select Fsite_id, Fip, Fport ,Fremote from t_site_server");
        }

        if(mysql_real_query(mysql, sQuery, strlen(sQuery)))
        {
                g_insLogMng.error("Execute query SQL(%s) failed, reason = %s",sQuery, mysql_error(mysql));
                mysql_close(mysql);
                return -1;
        }

        //������
        if((mysql_res = mysql_store_result(mysql)) == NULL)
        {
                 g_insLogMng.error("Store SQL(%s) result failed, reason = %s", sQuery, mysql_error(mysql));
                mysql_close(mysql);
                return -1;
        }

        //��ý��
        CSiteInfo insSiteInfo;
        CServerInfo insServerInfo;
        map<string, CSiteInfo>::iterator it;
        while((stRow = mysql_fetch_row(mysql_res)) != NULL)
        {
                strcpy(insServerInfo.m_sName, stRow[1]);
                strcpy(insServerInfo.m_sIP, stRow[1]);
                insServerInfo.m_iPort = atoi(stRow[2]);

                it = m_mapSite.find(stRow[0]);

                //��վ��
                if ( m_mapSite.end() == it)
                {
                        insSiteInfo.m_sName = stRow[0];
                        insSiteInfo.m_vecServerList.clear(); //����ϵ�
                        insSiteInfo.m_vecServerList.push_back(insServerInfo);
                        //�������ݿ���û���Ǹ�Ŀ¼�ֶ�,����Ϊ�˴���ͳһ,Ĭ����/usr/local/qq_$sitename/htdocs
                        insSiteInfo.m_sRootDir = string("/usr/local/qq_") + insSiteInfo.m_sName + "/htdocs";
                        m_mapSite[stRow[0]] = insSiteInfo;
                }
                else //�Ѿ��������վ��
                {
                        it->second.m_vecServerList.push_back(insServerInfo);
                }
        }

        mysql_free_result(mysql_res);
        mysql_close(mysql);

        return 0;
#else
        return -1;  //��
#endif
}

void CConfigMng::getSiteMap(map<string, CSiteInfo>& insMapSite)
{
        m_insShareMutex.setLock();
        insMapSite = m_mapSite;
        /*
        map<string, CSiteInfo>::iterator it1 = m_mapSite.begin();
        CSiteInfo insSiteInfo;
        for(; it1 != m_mapSite.end(); it1++)
        {
                insSiteInfo.m_vecServerList.clear();
                vector<CServerInfo>::iterator it2 = (it1->second).m_vecServerList.begin();
                for(; it2 != (it1->second).m_vecServerList.end(); it2++)
                {
                        insSiteInfo.m_vecServerList.push_back(*it2);
                }
                insSiteInfo.m_sName = (it1->second).m_sName;
                insSiteInfo.m_sRootDir = (it1->second).m_sRootDir;
                insMapSite[insSiteInfo.m_sName]= insSiteInfo;
        }
        */
        m_insShareMutex.unLock();
        return;
}

void CConfigMng::getSiteServer(vector<CServerInfo>& vecServerInfo, const string& sSiteName)
{
        m_insShareMutex.setLock();
        vecServerInfo = m_mapSite[sSiteName].m_vecServerList;  //ʹ��֮ǰ����ȷ��վ���Ѿ�����
        m_insShareMutex.unLock();
        return;
}



