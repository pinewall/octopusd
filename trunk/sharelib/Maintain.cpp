//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: Maintain.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "Maintain.h"
#include "ProtocolMng.h"

extern string g_sHomeDir;
extern CLogMng g_insLogMng;
extern CConfigMng g_insConfigMng;
extern CQueueMng g_insQueueMng;
extern map<int, int> g_mapThread2pid;
extern CThreadMutex g_insConCtrl;  //������
extern list<CTCPSocket> g_lstConnectPool;

CMaintain::CMaintain()
{
        m_pShmBaseAddr = NULL;
}

CMaintain::~CMaintain()
{
}

void CMaintain::process()
{
        //ExitState -- ������
        //LogMode  -- ������
        //LogModule -- ������

        //ReloadFailDuty -- ְ����( �ȽϺķ�ʱ�䣬���ŵ�����routineFailDutyȥ�����Ա�����û�������Ӧ)
        //if (0 != getFlag(ReloadFailDuty))
        //{
        //        reloadFailDuty(getFlag(ReloadFailDuty));
        //        setFlag(ReloadFailDuty, 0);
        //}

        //PrintCfg -- ְ����
        if (1 == getFlag(PrintCfg))
        {
                printCfg();
                setFlag(PrintCfg, 0);
        }

        //PrintServerStatus -- ְ����
        if (1 == getFlag(PrintServerStatus))
        {
                printServerStatus();
                setFlag(PrintServerStatus, 0);
        }

        //RefreshCommonCfg -- ְ����
        if (1 == getFlag(RefreshCommonCfg))
        {
                refreshCommonCfg();
                setFlag(RefreshCommonCfg, 0);
        }

        //RefreshSiteInfo -- ְ����
        if (1 == getFlag(RefreshSiteInfo))
        {
                refreshSiteInfo();
                setFlag(RefreshSiteInfo, 0);
        }

        //PrintQCurInfo -- ְ����
        if (1 == getFlag(PrintQCurInfo))
        {
                printQCurInfo();
                setFlag(PrintQCurInfo, 0);
        }

        //RefreshLogModule -- ְ����
        if (1 == getFlag(RefreshLogModule))
        {
                refreshLogModule();
                setFlag(RefreshLogModule, 0);
        }
        
        //PrintThread2Pid -- ְ����
        if (1 == getFlag(PrintThread2Pid))
        {
                printThread2Pid();
                setFlag(PrintThread2Pid, 0);
        }
        
        //SaveDutyQueueWhenExit -- ������

        //ForceCloseAllSocket
        if (1 == getFlag(ForceCloseAllSocket))
        {
                closeAllSocket();
                setFlag(ForceCloseAllSocket, 0);
        }

        //ClearErrorIPPort
        if (1 == getFlag(ClearErrorIPPort))
        {
                g_insQueueMng.clearErrorIPPort("");  //ȫ�����
                setFlag(ClearErrorIPPort, 0);
        }

        //ForceCloseAllSocketFD
        if (1 == getFlag(ForceCloseAllSocketFD))
        {
                for(int i = 10; i < 1024; i++)
                {
                        close(i);  //ǿ�ƹر�����fd
                }
                setFlag(ForceCloseAllSocketFD, 0);
        }

        //ClearDisabledDuty ����ڵ����߳���ʹ�ã��Ǹ����ڵĹ��ܿ���,��Ҫ�ֹ��޸ĺͻָ�
        //QueueSafeMode ����ʹ�ã�Ϊ1����ȫģʽ��2������:ִ��IP�ָ�����ط�,У�鱾���ļ����޸�ʱ��

        return;
}

int CMaintain::init()
{
        //��ʼ��ʼ��ά��ģ��
        g_insLogMng.normal("Begin to create ShareMem(%s/bin, 0)......", 
                g_sHomeDir.c_str());
        
        //[1]���������ڴ�
        key_t iShMemkey = ftok((g_sHomeDir+"/bin").c_str(), 0);
        if (iShMemkey < 0)
        {
                g_insLogMng.error("Ftok(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return  -1;
        }        

        //����magic�����ڴ��ά����
        Magic_ShareMem insShMem;

        //���빲���ڴ�
        int iShmID = insShMem.GetShm(iShMemkey, SizeOfShareMem*sizeof(int));
        if (0 > iShmID)
        {
                g_insLogMng.error("Magic_ShareMem GetShm(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return -1;
        }        

        //���ڴ浽��ַ
        if (0 > insShMem.AttachShm())
        {
                g_insLogMng.error("Magic_ShareMem AttachShm(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return -1;
        }


        //ȷ���ڴ��Ƿ��ǵ�һ������
        if (true == insShMem.isCreate())  // ��
        {
                insShMem.InitShm();
                g_insLogMng.normal("Create ShareMem(id=%x) successfully.", iShMemkey);
        }
        else
        {
                g_insLogMng.normal("ShareMem(id=%x) exist, use it.", iShMemkey);
        }

        //��ַ
        m_pShmBaseAddr = insShMem.Address();

        //��ʼ���õ��Ĺ����ڴ�
        setFlag(ExitState, 0);
        setFlag(LogMode, g_insConfigMng.m_tComCfg.m_iLogMode);
        setFlag(LogModule, 0);
        setFlag(ReloadFailDuty, 0);
        setFlag(PrintCfg, 0);
        setFlag(PrintServerStatus, 0);
        setFlag(RefreshCommonCfg, 0);
        setFlag(RefreshSiteInfo, 0);
        setFlag(PrintQCurInfo, 0);
        setFlag(RefreshLogModule, 0);
        setFlag(PrintThread2Pid, 0);
        setFlag(SaveDutyQueueWhenExit, 0);
        setFlag(ForceCloseAllSocket, 0);
        setFlag(ClearErrorIPPort, 0);
        setFlag(ForceCloseAllSocketFD, 0);
        //setFlag(ClearDisabledDuty, 0); ��������û���һ�ε����ã�����Ҫʱ���ù������
        //setFlag(QueueSafeMode, 0);

        //[2]ʱ����Ϣ
        time(&m_iLastQStatTime);
        time(&m_iLastDealFailDutyTime);
        time(&m_iLastCheckRunStatusTime);
        time(&m_iLastBackupLogTime);
        time(&m_iLastCheckErrorIPTime);
        
        return 0; 
        
}

int CMaintain::connect()
{
        //��ʼ��ʼ��ά��ģ��
        g_insLogMng.normal("Begin to connect to ShareMem ......");
        
        //���������ڴ�
        key_t iShMemkey = ftok ((g_sHomeDir+"/bin").c_str(), 0);
        if (iShMemkey < 0)
        {
                g_insLogMng.error("Ftok(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return  -1;
        }        

        //����magic�����ڴ��ά����
        Magic_ShareMem insShMem;

        //���빲���ڴ�
        int iShmID = insShMem.GetShm(iShMemkey, SizeOfShareMem*sizeof(int));
        if (0 > iShmID)
        {
                g_insLogMng.error("Magic_ShareMem GetShm(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return -1;
        }        

        //���ڴ浽��ַ
        if (0 > insShMem.AttachShm())
        {
                g_insLogMng.error("Magic_ShareMem AttachShm(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return -1;
        }

        //��ַ
        m_pShmBaseAddr = insShMem.Address();

        //ȷ���ڴ��Ƿ��ǵ�һ������
        if (true == insShMem.isCreate())  // ��
        {
                g_insLogMng.normal("Connect to ShareMem failed, because of not exist.");
                insShMem.DetachShm();
                insShMem.RemoveShm();
                m_pShmBaseAddr = NULL;
                return -1;
        }
        else
        {
                g_insLogMng.normal("Connect to ShareMem successfully.");
                return 0; 
        }
}

int CMaintain::delShareMem()
{
        //��ʼ��ʼ��ά��ģ��
        g_insLogMng.normal("Begin to delete to ShareMem ......");
        
        //���������ڴ�
        key_t iShMemkey = ftok ((g_sHomeDir+"/bin").c_str(), 0);
        if (iShMemkey < 0)
        {
                g_insLogMng.error("Ftok(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return  -1;
        }        

        //����magic�����ڴ��ά����
        Magic_ShareMem insShMem;

        //���빲���ڴ�
        int iShmID = insShMem.GetShm(iShMemkey, SizeOfShareMem*sizeof(int));
        if (0 > iShmID)
        {
                g_insLogMng.error("Magic_ShareMem GetShm(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return -1;
        }        

        //���ڴ浽��ַ
        if (0 > insShMem.AttachShm())
        {
                g_insLogMng.error("Magic_ShareMem AttachShm(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return -1;
        }

        //��ַ
        m_pShmBaseAddr = insShMem.Address();

        g_insLogMng.normal("Delete ShareMem successfully.");
        insShMem.DetachShm();
        insShMem.RemoveShm();
        m_pShmBaseAddr = NULL;
        return 0;
}

void CMaintain::setFlag(MaintainFlag eSeq, int iValue)
{
        if (NULL == m_pShmBaseAddr)
        {
                return;
        }
        memcpy((m_pShmBaseAddr + (sizeof(int))*((int)eSeq)), (char*)&iValue, sizeof(int));
        return;

}

int CMaintain::getFlag(MaintainFlag eSeq)
{
        if (NULL == m_pShmBaseAddr)
        {
                return 0;
        }
        int iTmp;
        memcpy((char*)&iTmp, (m_pShmBaseAddr + (sizeof(int))*((int)eSeq)), sizeof(int));
        return iTmp;
}

int CMaintain::isExitProgram()
{
        return getFlag(ExitState);
}

void CMaintain::reloadFailDuty(const int iLogSeq)
{
        //��Ҫ��¼���ٵ����û���Ϊ
        g_insLogMng.normal("Recv a singal to reload failed files, seq = %d", iLogSeq);
        
        int iTmpLogSeq = iLogSeq;
        // 0
        if (-2 == iTmpLogSeq)
        {
                iTmpLogSeq = 0;
        }
        // ALL
        if (-1 == iTmpLogSeq)
        {
                //��ʱ�޸�Ϊ��������ʧ���ļ�,�������̫����޸�Ϊÿ�����ϣ�����߰����ļ���
                for(int iLoop = 0; iLoop < 10000; iLoop++)
                {
                        g_insQueueMng.getFailDutyFFile2DutyQ(iLoop);
                }
                return;
        }
        
        //normal
        if (iTmpLogSeq > 9999 || iTmpLogSeq< 0)
        {
                return;
        }
        g_insQueueMng.getFailDutyFFile2DutyQ(iTmpLogSeq);

        return;
}

void CMaintain::printCfg()
{
        g_insConfigMng.print();
        return;
}

void CMaintain::printServerStatus()
{
        char sTmpStr[4096] = "";
        string sPrintInfo = "";  
        int iRet = 0;
        sprintf(sTmpStr, "======%s server status Info======\n",  (CComFun::GetExactCurDateTime()).c_str());

        map<string, CSiteInfo> insMapSite;
        g_insConfigMng.getSiteMap(insMapSite);        
        for(map<string, CSiteInfo>::iterator it = insMapSite.begin();
                it != insMapSite.end(); it++)
        {
                sprintf(sTmpStr, "%s[%s]\n", sTmpStr, it->first.c_str());
                for(vector<CServerInfo>::iterator itx = it->second.m_vecServerList.begin();
                        itx != it->second.m_vecServerList.end(); itx++)
                {
                        iRet = CProtocolMng::checkServerStatus((*itx).m_sIP, (*itx).m_iPort);
                        if (0 == iRet)
                        {
                                sprintf(sTmpStr, "%s\t%s\t%d\tok\n ", sTmpStr, 
                                        (*itx).m_sIP,  (*itx).m_iPort);
                        }
                        else
                        {
                                sprintf(sTmpStr, "%s\t%s\t%d\tnot ok\n ", sTmpStr, 
                                        (*itx).m_sIP,  (*itx).m_iPort);
                        }
                }
                sPrintInfo = sPrintInfo + sTmpStr;
                sTmpStr[0] = '\0';
        }
                
        string myFile = g_sHomeDir + "/log/prt/serverstatus.prt";
        FILE *fp = fopen(myFile.c_str(), "w");
        if (NULL == fp)
        {
                return;
        }

        fputs(sPrintInfo.c_str(), fp);
        fclose(fp);

        return;        
}

int CMaintain::refreshCommonCfg()
{
        g_insLogMng.normal("Begin to refresh Common cfg info ......");
        CConfigMng tmpConfingMng;
        if (0 != tmpConfingMng.init())
        {
                 g_insLogMng.error("Refresh Common cfg info failed.");
                 return -1;
        }
 
        strcpy(g_insConfigMng.m_tComCfg.m_sLoginIP,  tmpConfingMng.m_tComCfg.m_sLoginIP);
        
        strcpy(g_insConfigMng.m_tComCfg.m_sServerFlag,  tmpConfingMng.m_tComCfg.m_sServerFlag);

        strcpy(g_insConfigMng.m_tComCfg.m_sFileModeOption, tmpConfingMng.m_tComCfg.m_sFileModeOption);

        g_insConfigMng.m_tComCfg.m_iDelFileAfterUpload = tmpConfingMng.m_tComCfg.m_iDelFileAfterUpload;

        g_insConfigMng.m_tComCfg.m_iIfUploadZeroFile = tmpConfingMng.m_tComCfg.m_iIfUploadZeroFile;

        g_insConfigMng.m_tComCfg.m_iMaxLimitOfUploadFile = tmpConfingMng.m_tComCfg.m_iMaxLimitOfUploadFile;
 
        g_insConfigMng.m_tComCfg.m_iMaxNrOfRetry = tmpConfingMng.m_tComCfg.m_iMaxNrOfRetry;

        g_insConfigMng.m_tComCfg.m_iProtocolType = tmpConfingMng.m_tComCfg.m_iProtocolType;
                
        g_insConfigMng.m_tComCfg.m_iIfFailLogFlag = tmpConfingMng.m_tComCfg.m_iIfFailLogFlag;
        
        g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty = tmpConfingMng.m_tComCfg.m_iMaxNrOfQueueDuty;

        g_insConfigMng.m_tComCfg.m_iMaxNrOfSaveLastSucDuty = tmpConfingMng.m_tComCfg.m_iMaxNrOfSaveLastSucDuty;

        g_insConfigMng.m_tComCfg.m_iTimeOfFailDutyRetry = tmpConfingMng.m_tComCfg.m_iTimeOfFailDutyRetry;

        g_insConfigMng.m_tComCfg.m_iTimeOfCheckRunStatus = tmpConfingMng.m_tComCfg.m_iTimeOfCheckRunStatus;

        g_insConfigMng.m_tComCfg.m_iMaxNrOfDutyToAlarm = tmpConfingMng.m_tComCfg.m_iMaxNrOfDutyToAlarm;

        g_insConfigMng.m_tComCfg.m_iTimeOfThreadStat = tmpConfingMng.m_tComCfg.m_iTimeOfThreadStat;
 
        g_insConfigMng.m_tComCfg.m_iMaxNrOfCnt = tmpConfingMng.m_tComCfg.m_iMaxNrOfCnt;
 
        g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout = tmpConfingMng.m_tComCfg.m_iRecvMsgTimeout;
 
        g_insConfigMng.m_tComCfg.m_iRecvFileTimeout = tmpConfingMng.m_tComCfg.m_iRecvFileTimeout;

         g_insConfigMng.m_tComCfg.m_iSendFlowCtrl = tmpConfingMng.m_tComCfg.m_iSendFlowCtrl;


        g_insLogMng.normal("Refresh Common cfg info successfully");
        
        return 0;

}

int CMaintain::refreshSiteInfo()
{
        g_insLogMng.normal("Begin to refresh site info ......");
        CConfigMng tmpConfingMng;
        if (0 != tmpConfingMng.init())
        {
                 g_insLogMng.error("Refresh Common cfg info failed.");
                 return -1;
        }

        g_insConfigMng.m_insShareMutex.setLock();  // ������
        g_insConfigMng.m_mapSite.clear();
        g_insConfigMng.m_mapSite = tmpConfingMng.m_mapSite;
        g_insConfigMng.m_insShareMutex.unLock();  // ������

        g_insLogMng.normal("Refresh site info successfully.");
        return 0;
}

void CMaintain::printQCurInfo()
{
        g_insQueueMng.printCurInfo();
        return;
}

void CMaintain::printQStatInfo()
{
        g_insQueueMng.printStatInfoXML();
        g_insQueueMng.printStatInfo();
        return;
}
int CMaintain:: getQueueInfo()
{        
        return g_insQueueMng.getQueueInfo();
}

void CMaintain::refreshLogModule()
{
        g_insLogMng.normal("Begin to refresh Log Module info ......");        
        g_insLogMng.refreshLogModule();
        g_insLogMng.normal("Refresh Log Module info successfully.");
        return;
}

void CMaintain::routineFailDuty()
{
        //ReloadFailDuty -- ְ���� (�ȽϺķ�ʱ�䣬���ŵ�����routineFailDutyȥ�����Ա�����û�������Ӧ)
        if (0 != getFlag(ReloadFailDuty))
        {
                reloadFailDuty(getFlag(ReloadFailDuty));
                setFlag(ReloadFailDuty, 0);
        }  

        time_t iCurTime;
        time(&iCurTime);//���д���ʧ������        
        if (0 != g_insConfigMng.m_tComCfg.m_iTimeOfFailDutyRetry &&
                iCurTime - m_iLastDealFailDutyTime >= g_insConfigMng.m_tComCfg.m_iTimeOfFailDutyRetry)
        {
                //��ʱ�޸�Ϊ��������ʧ���ļ�,�������̫����޸�Ϊÿ�����ϣ�����߰����ļ���
                reloadFailDuty(-1);

                time(&iCurTime);
                m_iLastDealFailDutyTime = iCurTime;
        }
        return;
        
}

void CMaintain::routine()
{
        time_t iCurTime;
        time(&iCurTime);

        //���д���ͳ����Ϣ
        if (0 != g_insConfigMng.m_tComCfg.m_iTimeOfThreadStat &&
                iCurTime - m_iLastQStatTime >= g_insConfigMng.m_tComCfg.m_iTimeOfThreadStat)
        {
                printQStatInfo();
                m_iLastQStatTime = iCurTime;
        }

        //���д���ʧ������
        //if (0 != g_insConfigMng.m_tComCfg.m_iTimeOfFailDutyRetry &&
        //        iCurTime - m_iLastDealFailDutyTime >= g_insConfigMng.m_tComCfg.m_iTimeOfFailDutyRetry)
        //{
                //��ʱ�޸�Ϊ��������ʧ���ļ�,�������̫����޸�Ϊÿ�����ϣ�����߰����ļ���
         //       for(int iLoop = 0; iLoop < 10000; iLoop++)
        //        {
        //                reloadFailDuty(iLoop);
        //        }
        //        m_iLastDealFailDutyTime = iCurTime;
        //}

        //���ͱ�����־���̶���ÿ60����һ��
        if (iCurTime - m_iLastBackupLogTime >= 60)
        {
                g_insLogMng.backuplogfile();
                m_iLastBackupLogTime = iCurTime;
        }

        //�������Ƿ�ָ� ÿ5*60����һ��
        if(iCurTime - m_iLastCheckErrorIPTime >= 300)
        {
                // ʧ�������б��е������Ƿ����ڿ��ٵ����ӳɹ���������������
                map<string, unsigned int> m_mapIPPort;
                g_insQueueMng.getErrorIPPort(m_mapIPPort);
                map<string, unsigned int>::iterator mit = m_mapIPPort.begin();
                for(; mit!= m_mapIPPort.end(); mit++)
                {                        
                        string sIPPort = mit->first;
                        string sIP, sPort;
                        string::size_type pos = 0;
                        pos = sIPPort.find("_", 0);
                        if(string::npos == pos)
                        {
                                g_insQueueMng.clearErrorIPPort(sIPPort);  //���Ϸ���ֱ�������
                                continue;
                        }
                        else
                        {
                                sIP = sIPPort.substr(0, pos);
                                sPort = sIPPort.substr(pos + 1);
                                //Ϊ�˱�֤�Է�������ȷʵ���������ʱ�価�����õĶ�һЩ��cfg/3���ӣ�ͬʱ����2��
                                if (0 == CProtocolMng::checkServerStatus(sIP, atoi(sPort.c_str()), 3)) 
                                {
                                        if (0 == CProtocolMng::checkServerStatus(sIP, atoi(sPort.c_str()), 3)) 
                                        {
                                                g_insLogMng.normal("===ErrorIPPort: %s is recovered ===", sIPPort.c_str());
                                                g_insQueueMng.clearErrorIPPort(sIPPort);   //����Ѿ�������IP
                                        }
                                }
                        }
                }
                m_iLastCheckErrorIPTime = iCurTime;
        }

        //��ر�������
        if (0 != g_insConfigMng.m_tComCfg.m_iTimeOfCheckRunStatus &&
                iCurTime - m_iLastCheckRunStatusTime >= g_insConfigMng.m_tComCfg.m_iTimeOfCheckRunStatus)
        {
                string sInfo = "";  //����Ϣ����

#if 0 //��ʱȡ��,��Ϊʧ���ļ��ĸ澯�Ѿ��ܱ�֤֮
                //[���1] �����ַ���������״̬
                map<string, CSiteInfo> insMapSite;
                map<string, CServerInfo> insServerList;
                g_insConfigMng.getSiteMap(insMapSite);   
                map<string, CSiteInfo>::iterator it = insMapSite.begin();
                for(; it != insMapSite.end(); it++)
                {
                        vector<CServerInfo>::iterator it1 = (it->second.m_vecServerList).begin();
                        for(; it1 != (it->second.m_vecServerList).end(); it1++)
                        {
                                string sKey =(*it1).m_sName; //����������Ψһ
                                insServerList[sKey] = (*it1);  //��ҪȡΨһ�ķ������б�
                        }
                        map<string, CServerInfo>::iterator it2 = insServerList.begin();
                        for(; it2!=insServerList.end(); it2++)
                        {
                                char sTmpPort[64];
                                sprintf(sTmpPort, "%d", it2->second.m_iPort);
                                if (0 != CProtocolMng::checkServerStatus(it2->second.m_sIP, it2->second.m_iPort))
                                {                                                                                
                                        sInfo = string("����������(") + it2->second.m_sIP + ":" + sTmpPort +  ")���ӳ�ʱ,��������.";
                                        sendMobileMsg(sInfo);  //���Ͷ���

                                        //дalarm�����ļ� IP.Port.alm  [ALARM:1]
                                        CComFun::writeWholeFileContent((g_sHomeDir + "/dat/" +  it2->second.m_sIP +  "." +  sTmpPort  + ".alm"), "Disconnected");
                                }
                                //else  //����Ǻõ�,ɾ����ǰ���ļ�
                                //{
                                //        CComFun::delFile(g_sHomeDir + "/dat/" +  it2->second.m_sIP +  "." +  sTmpPort  + ".alm");
                                //}
                        }                        
                }
#endif

                //[���2] ���г����������ͬʱ��ӡ�������д�����Щ����
                int iQueueSize = getQueueInfo();
                if (iQueueSize >= g_insConfigMng.m_tComCfg.m_iMaxNrOfDutyToAlarm)
                {
                        char sTmpInt[32];
                        sprintf(sTmpInt, "%d", iQueueSize);
                        sInfo = string("�����е��������Ѿ��ﵽ") + sTmpInt + "��,���顣";
                        sendMobileMsg(sInfo);  //���Ͷ���

                        //дalarm�����ļ� [ALARM:2]
                        CComFun::writeWholeFileContent((g_sHomeDir + "/dat/maxduty.alm"), "common.cfg:MaxNrOfDutyToAlarm");
                }
                //else  //����,�����ϴ��Ƿ񱨹����棬����ɾ���ļ�
                //{
                //        CComFun::delFile(g_sHomeDir + "/dat/maxduty.alm");
                //}                

                //[���3] �Ƿ���ʧ���ļ�������ֻ��������ʧ���ļ�
                if (1 == checkFailDutyExist())
                {
                        sInfo =  "�зַ�ʧ���ļ�������";
                        map<string, unsigned int> m_mapIPPort;
                        g_insQueueMng.getErrorIPPort(m_mapIPPort);
                        if (m_mapIPPort.size() != 0)
                        {
                                sInfo = sInfo + "�ַ��ٶȹ��ϻ���������������:"; 
                                map<string, unsigned int>::iterator mit = m_mapIPPort.begin();
                                for(mit = m_mapIPPort.begin(); mit != m_mapIPPort.end(); mit++)
                                {
                                        sInfo = sInfo + mit->first + "|"; 
                                }
                        }
                        sendMobileMsg(sInfo);  //���Ͷ���
                }
                //else {}  ���������.alm�ļ��ˣ���Ϊ�ⲿ���Լ�⵽ʧ���ļ��Ĵ��ںͽ�� [ALARM:3]

                //ʱ����ɵ���һ������
                m_iLastCheckRunStatusTime = iCurTime;
        }

        return;
}

int CMaintain::checkFailDutyExist()
{
        //��ȡ����ļ�
        string sPathName = g_sHomeDir + "/dat/failduty.seq";
        FILE *fp = fopen(sPathName.c_str(), "r");
        if (NULL == fp)
        {
                return 0;
        }
        char sTmp[LenOfNormal + 1];
        if (NULL == fgets(sTmp, LenOfNormal, fp))
        {
                fclose(fp);
                return 0;
        }
        fclose(fp);

        //����������ļ��Ƿ񻹴���
        int iFileNum = atoi(sTmp);
        sprintf(sTmp, "failduty.%04d", iFileNum);
        sPathName = g_sHomeDir + "/dat/" + sTmp;

        struct stat tmpStat;
        if(0 <= stat(sPathName.c_str(), &tmpStat))
        {
                //��С����0�������ļ����������3Сʱ����Ȼ�ڱ仯
                if (tmpStat.st_size > 0 && (time(NULL) - tmpStat.st_mtime < 3*60*60))
                {
                        return 1;
                }
        }        
        return 0;
        
}

void CMaintain::sendMobileMsg(string sInfo)
{
        string sPathFile =g_sHomeDir + "/cfg/mobilelist.cfg";
        map<string, string>  mapMobileNameValue;        
        int iRet = CComFun::readEqualValueFile(sPathFile, mapMobileNameValue);
        if (0 != iRet)
        {
                return;
        }
        if ( 0 == mapMobileNameValue.size())
        {
                return;
        }

        //������ַ
        string sSendInfo, sIP;
        string sName;
        string sMac, sMask;

        sSendInfo = string("���㱨��[") + g_insConfigMng.m_tComCfg.m_sServerFlag + "]";
        
        // eth1
        if (0 != CComFun::getHostName8IP("eth1", sMac, sMask, sIP))  
        {
                //ȡ����eth1����ȡeth0�������rootȨ�ޣ���������ȡ����Ϊ��
                if (0 == CComFun::getHostName8IP("eth0", sMac, sMask, sIP))  //û��ȡ��,����������������ȡ
                {
                        sSendInfo = sSendInfo + "[" + sIP  + "]" + sInfo;
                }
                else //û��ȡ��,���������ļ��е�IP
                {
                        sSendInfo = sSendInfo + "[" + g_insConfigMng.m_tComCfg.m_sServerIP + "]" + sInfo;
                }
        }
        else  //ȡ����
        {
                 sSendInfo = sSendInfo + "[" + sIP + "]" + sInfo;
        }
        
        map<int, string> mapShortMsg;  
        CComFun::getSubStrByWord(sSendInfo, mapShortMsg, 70);
        
        for(map<string, string>::const_iterator itr = mapMobileNameValue.begin();
                itr!= mapMobileNameValue.end(); itr++)
        {
                g_insLogMng.normal("Send message:%s to %s", sSendInfo.c_str(), (itr->second).c_str());
                map<int, string>::iterator itMsg = mapShortMsg.begin();
                while (itMsg != mapShortMsg.end())
                {
                        CMobileInterface::SendSMSMsg((itr->second), itMsg->second);
                        itMsg++;
                }                
        }

        return;
}

void CMaintain::printScreen()
{
        for (int iLoop = 0; iLoop < (int)MaxFlag; iLoop++)
        {
                printf("No.%d = %d\n", iLoop, getFlag((MaintainFlag)iLoop));
        }
        return;
}

void CMaintain::printThread2Pid()
{
        char sTmpStr[4096*2] = "";
        sprintf(sTmpStr, "\tThreadID\tpID\n");
        for(map<int, int>::iterator it = g_mapThread2pid.begin();
                it != g_mapThread2pid.end(); it++)
        {
                sprintf(sTmpStr, "%s\t%d\t\t%d\n", sTmpStr, it->first, it->second);                
        }
                
        string myFile = g_sHomeDir + "/log/prt/thread2pid.prt";
        FILE *fp = fopen(myFile.c_str(), "w");
        if (NULL == fp)
        {
                return;
        }

        fputs(sTmpStr, fp);
        fclose(fp);
        
        return;        
}

void CMaintain::closeAllSocket()
{
        g_insLogMng.normal("Force to close all socket connection.");
        g_insConCtrl.setLock();  //����
        for (list<CTCPSocket>::iterator it = g_lstConnectPool.begin();
                it != g_lstConnectPool.end(); it++)
        {
                 (*it).closeSocket();  //�ر�
        }
        g_lstConnectPool.clear();
        g_insLogMng.normal("Close all socket connection successfully.");
        g_insConCtrl.unLock(); //����
        return;
}


