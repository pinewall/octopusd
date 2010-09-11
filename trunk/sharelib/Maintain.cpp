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
extern CThreadMutex g_insConCtrl;  //锁控制
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
        //ExitState -- 开关类
        //LogMode  -- 开关类
        //LogModule -- 开关类

        //ReloadFailDuty -- 职能类( 比较耗费时间，这块放到单独routineFailDuty去做，以便提高用户快速响应)
        //if (0 != getFlag(ReloadFailDuty))
        //{
        //        reloadFailDuty(getFlag(ReloadFailDuty));
        //        setFlag(ReloadFailDuty, 0);
        //}

        //PrintCfg -- 职能类
        if (1 == getFlag(PrintCfg))
        {
                printCfg();
                setFlag(PrintCfg, 0);
        }

        //PrintServerStatus -- 职能类
        if (1 == getFlag(PrintServerStatus))
        {
                printServerStatus();
                setFlag(PrintServerStatus, 0);
        }

        //RefreshCommonCfg -- 职能类
        if (1 == getFlag(RefreshCommonCfg))
        {
                refreshCommonCfg();
                setFlag(RefreshCommonCfg, 0);
        }

        //RefreshSiteInfo -- 职能类
        if (1 == getFlag(RefreshSiteInfo))
        {
                refreshSiteInfo();
                setFlag(RefreshSiteInfo, 0);
        }

        //PrintQCurInfo -- 职能类
        if (1 == getFlag(PrintQCurInfo))
        {
                printQCurInfo();
                setFlag(PrintQCurInfo, 0);
        }

        //RefreshLogModule -- 职能类
        if (1 == getFlag(RefreshLogModule))
        {
                refreshLogModule();
                setFlag(RefreshLogModule, 0);
        }
        
        //PrintThread2Pid -- 职能类
        if (1 == getFlag(PrintThread2Pid))
        {
                printThread2Pid();
                setFlag(PrintThread2Pid, 0);
        }
        
        //SaveDutyQueueWhenExit -- 开关类

        //ForceCloseAllSocket
        if (1 == getFlag(ForceCloseAllSocket))
        {
                closeAllSocket();
                setFlag(ForceCloseAllSocket, 0);
        }

        //ClearErrorIPPort
        if (1 == getFlag(ClearErrorIPPort))
        {
                g_insQueueMng.clearErrorIPPort("");  //全部清空
                setFlag(ClearErrorIPPort, 0);
        }

        //ForceCloseAllSocketFD
        if (1 == getFlag(ForceCloseAllSocketFD))
        {
                for(int i = 10; i < 1024; i++)
                {
                        close(i);  //强制关闭所有fd
                }
                setFlag(ForceCloseAllSocketFD, 0);
        }

        //ClearDisabledDuty 这个在单独线程中使用，是个长期的功能开关,需要手工修改和恢复
        //QueueSafeMode 长期使用，为1，安全模式下2个动作:执行IP恢复后的重发,校验本地文件的修改时间

        return;
}

int CMaintain::init()
{
        //开始初始化维护模块
        g_insLogMng.normal("Begin to create ShareMem(%s/bin, 0)......", 
                g_sHomeDir.c_str());
        
        //[1]创建共享内存
        key_t iShMemkey = ftok((g_sHomeDir+"/bin").c_str(), 0);
        if (iShMemkey < 0)
        {
                g_insLogMng.error("Ftok(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return  -1;
        }        

        //申请magic共享内存的维护类
        Magic_ShareMem insShMem;

        //申请共享内存
        int iShmID = insShMem.GetShm(iShMemkey, SizeOfShareMem*sizeof(int));
        if (0 > iShmID)
        {
                g_insLogMng.error("Magic_ShareMem GetShm(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return -1;
        }        

        //绑定内存到地址
        if (0 > insShMem.AttachShm())
        {
                g_insLogMng.error("Magic_ShareMem AttachShm(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return -1;
        }


        //确定内存是否是第一次申请
        if (true == insShMem.isCreate())  // 是
        {
                insShMem.InitShm();
                g_insLogMng.normal("Create ShareMem(id=%x) successfully.", iShMemkey);
        }
        else
        {
                g_insLogMng.normal("ShareMem(id=%x) exist, use it.", iShMemkey);
        }

        //地址
        m_pShmBaseAddr = insShMem.Address();

        //初始化用到的共享内存
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
        //setFlag(ClearDisabledDuty, 0); 这个保留用户上一次的设置，不需要时，用工具清空
        //setFlag(QueueSafeMode, 0);

        //[2]时间信息
        time(&m_iLastQStatTime);
        time(&m_iLastDealFailDutyTime);
        time(&m_iLastCheckRunStatusTime);
        time(&m_iLastBackupLogTime);
        time(&m_iLastCheckErrorIPTime);
        
        return 0; 
        
}

int CMaintain::connect()
{
        //开始初始化维护模块
        g_insLogMng.normal("Begin to connect to ShareMem ......");
        
        //创建共享内存
        key_t iShMemkey = ftok ((g_sHomeDir+"/bin").c_str(), 0);
        if (iShMemkey < 0)
        {
                g_insLogMng.error("Ftok(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return  -1;
        }        

        //申请magic共享内存的维护类
        Magic_ShareMem insShMem;

        //申请共享内存
        int iShmID = insShMem.GetShm(iShMemkey, SizeOfShareMem*sizeof(int));
        if (0 > iShmID)
        {
                g_insLogMng.error("Magic_ShareMem GetShm(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return -1;
        }        

        //绑定内存到地址
        if (0 > insShMem.AttachShm())
        {
                g_insLogMng.error("Magic_ShareMem AttachShm(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return -1;
        }

        //地址
        m_pShmBaseAddr = insShMem.Address();

        //确定内存是否是第一次申请
        if (true == insShMem.isCreate())  // 是
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
        //开始初始化维护模块
        g_insLogMng.normal("Begin to delete to ShareMem ......");
        
        //创建共享内存
        key_t iShMemkey = ftok ((g_sHomeDir+"/bin").c_str(), 0);
        if (iShMemkey < 0)
        {
                g_insLogMng.error("Ftok(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return  -1;
        }        

        //申请magic共享内存的维护类
        Magic_ShareMem insShMem;

        //申请共享内存
        int iShmID = insShMem.GetShm(iShMemkey, SizeOfShareMem*sizeof(int));
        if (0 > iShmID)
        {
                g_insLogMng.error("Magic_ShareMem GetShm(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return -1;
        }        

        //绑定内存到地址
        if (0 > insShMem.AttachShm())
        {
                g_insLogMng.error("Magic_ShareMem AttachShm(%s, %d) failed.", (g_sHomeDir+"/bin").c_str(), 0);
                return -1;
        }

        //地址
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
        //主要记录跟踪到的用户行为
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
                //暂时修改为处理所有失败文件,如果性能太差，则修改为每天晚上０点或者按照文件来
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

        g_insConfigMng.m_insShareMutex.setLock();  // 锁控制
        g_insConfigMng.m_mapSite.clear();
        g_insConfigMng.m_mapSite = tmpConfingMng.m_mapSite;
        g_insConfigMng.m_insShareMutex.unLock();  // 锁控制

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
        //ReloadFailDuty -- 职能类 (比较耗费时间，这块放到单独routineFailDuty去做，以便提高用户快速响应)
        if (0 != getFlag(ReloadFailDuty))
        {
                reloadFailDuty(getFlag(ReloadFailDuty));
                setFlag(ReloadFailDuty, 0);
        }  

        time_t iCurTime;
        time(&iCurTime);//例行处理失败任务        
        if (0 != g_insConfigMng.m_tComCfg.m_iTimeOfFailDutyRetry &&
                iCurTime - m_iLastDealFailDutyTime >= g_insConfigMng.m_tComCfg.m_iTimeOfFailDutyRetry)
        {
                //暂时修改为处理所有失败文件,如果性能太差，则修改为每天晚上０点或者按照文件来
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

        //例行处理统计信息
        if (0 != g_insConfigMng.m_tComCfg.m_iTimeOfThreadStat &&
                iCurTime - m_iLastQStatTime >= g_insConfigMng.m_tComCfg.m_iTimeOfThreadStat)
        {
                printQStatInfo();
                m_iLastQStatTime = iCurTime;
        }

        //例行处理失败任务
        //if (0 != g_insConfigMng.m_tComCfg.m_iTimeOfFailDutyRetry &&
        //        iCurTime - m_iLastDealFailDutyTime >= g_insConfigMng.m_tComCfg.m_iTimeOfFailDutyRetry)
        //{
                //暂时修改为处理所有失败文件,如果性能太差，则修改为每天晚上０点或者按照文件来
         //       for(int iLoop = 0; iLoop < 10000; iLoop++)
        //        {
        //                reloadFailDuty(iLoop);
        //        }
        //        m_iLastDealFailDutyTime = iCurTime;
        //}

        //检查和备份日志，固定，每60秒检查一次
        if (iCurTime - m_iLastBackupLogTime >= 60)
        {
                g_insLogMng.backuplogfile();
                m_iLastBackupLogTime = iCurTime;
        }

        //检查队列是否恢复 每5*60秒检查一次
        if(iCurTime - m_iLastCheckErrorIPTime >= 300)
        {
                // 失败任务列表中的任务，是否能在快速的链接成功，如果可以清空它
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
                                g_insQueueMng.clearErrorIPPort(sIPPort);  //不合法的直接清除掉
                                continue;
                        }
                        else
                        {
                                sIP = sIPPort.substr(0, pos);
                                sPort = sIPPort.substr(pos + 1);
                                //为了保证对方服务器确实正常，这儿时间尽量设置的短一些，cfg/3秒钟，同时发送2次
                                if (0 == CProtocolMng::checkServerStatus(sIP, atoi(sPort.c_str()), 3)) 
                                {
                                        if (0 == CProtocolMng::checkServerStatus(sIP, atoi(sPort.c_str()), 3)) 
                                        {
                                                g_insLogMng.normal("===ErrorIPPort: %s is recovered ===", sIPPort.c_str());
                                                g_insQueueMng.clearErrorIPPort(sIPPort);   //清掉已经正常的IP
                                        }
                                }
                        }
                }
                m_iLastCheckErrorIPTime = iCurTime;
        }

        //监控报警处理
        if (0 != g_insConfigMng.m_tComCfg.m_iTimeOfCheckRunStatus &&
                iCurTime - m_iLastCheckRunStatusTime >= g_insConfigMng.m_tComCfg.m_iTimeOfCheckRunStatus)
        {
                string sInfo = "";  //短消息内容

#if 0 //暂时取消,因为失败文件的告警已经能保证之
                //[监控1] 下属分发服务器的状态
                map<string, CSiteInfo> insMapSite;
                map<string, CServerInfo> insServerList;
                g_insConfigMng.getSiteMap(insMapSite);   
                map<string, CSiteInfo>::iterator it = insMapSite.begin();
                for(; it != insMapSite.end(); it++)
                {
                        vector<CServerInfo>::iterator it1 = (it->second.m_vecServerList).begin();
                        for(; it1 != (it->second.m_vecServerList).end(); it1++)
                        {
                                string sKey =(*it1).m_sName; //机器的名称唯一
                                insServerList[sKey] = (*it1);  //主要取唯一的服务器列表
                        }
                        map<string, CServerInfo>::iterator it2 = insServerList.begin();
                        for(; it2!=insServerList.end(); it2++)
                        {
                                char sTmpPort[64];
                                sprintf(sTmpPort, "%d", it2->second.m_iPort);
                                if (0 != CProtocolMng::checkServerStatus(it2->second.m_sIP, it2->second.m_iPort))
                                {                                                                                
                                        sInfo = string("下属服务器(") + it2->second.m_sIP + ":" + sTmpPort +  ")连接超时,请检查网络.";
                                        sendMobileMsg(sInfo);  //发送短信

                                        //写alarm跟踪文件 IP.Port.alm  [ALARM:1]
                                        CComFun::writeWholeFileContent((g_sHomeDir + "/dat/" +  it2->second.m_sIP +  "." +  sTmpPort  + ".alm"), "Disconnected");
                                }
                                //else  //如果是好的,删掉以前的文件
                                //{
                                //        CComFun::delFile(g_sHomeDir + "/dat/" +  it2->second.m_sIP +  "." +  sTmpPort  + ".alm");
                                //}
                        }                        
                }
#endif

                //[监控2] 队列超长的情况，同时打印出队列中存在哪些任务
                int iQueueSize = getQueueInfo();
                if (iQueueSize >= g_insConfigMng.m_tComCfg.m_iMaxNrOfDutyToAlarm)
                {
                        char sTmpInt[32];
                        sprintf(sTmpInt, "%d", iQueueSize);
                        sInfo = string("队列中的任务数已经达到") + sTmpInt + "个,请检查。";
                        sendMobileMsg(sInfo);  //发送短信

                        //写alarm跟踪文件 [ALARM:2]
                        CComFun::writeWholeFileContent((g_sHomeDir + "/dat/maxduty.alm"), "common.cfg:MaxNrOfDutyToAlarm");
                }
                //else  //正常,看看上次是否报过警告，有则删掉文件
                //{
                //        CComFun::delFile(g_sHomeDir + "/dat/maxduty.alm");
                //}                

                //[监控3] 是否有失败文件产生，只检查最近的失败文件
                if (1 == checkFailDutyExist())
                {
                        sInfo =  "有分发失败文件产生。";
                        map<string, unsigned int> m_mapIPPort;
                        g_insQueueMng.getErrorIPPort(m_mapIPPort);
                        if (m_mapIPPort.size() != 0)
                        {
                                sInfo = sInfo + "分发速度故障或慢的下属机器有:"; 
                                map<string, unsigned int>::iterator mit = m_mapIPPort.begin();
                                for(mit = m_mapIPPort.begin(); mit != m_mapIPPort.end(); mit++)
                                {
                                        sInfo = sInfo + mit->first + "|"; 
                                }
                        }
                        sendMobileMsg(sInfo);  //发送短信
                }
                //else {}  这个不再做.alm文件了，因为外部可以检测到失败文件的存在和解决 [ALARM:3]

                //时间过渡到下一个周期
                m_iLastCheckRunStatusTime = iCurTime;
        }

        return;
}

int CMaintain::checkFailDutyExist()
{
        //读取序号文件
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

        //检查最近序号文件是否还存在
        int iFileNum = atoi(sTmp);
        sprintf(sTmp, "failduty.%04d", iFileNum);
        sPathName = g_sHomeDir + "/dat/" + sTmp;

        struct stat tmpStat;
        if(0 <= stat(sPathName.c_str(), &tmpStat))
        {
                //大小大于0，而且文件必须是最近3小时内仍然在变化
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

        //机器地址
        string sSendInfo, sIP;
        string sName;
        string sMac, sMask;

        sSendInfo = string("章鱼报警[") + g_insConfigMng.m_tComCfg.m_sServerFlag + "]";
        
        // eth1
        if (0 != CComFun::getHostName8IP("eth1", sMac, sMask, sIP))  
        {
                //取不到eth1，就取eth0，如果是root权限，这儿则可能取到的为空
                if (0 == CComFun::getHostName8IP("eth0", sMac, sMask, sIP))  //没有取到,尝试用主机名称来取
                {
                        sSendInfo = sSendInfo + "[" + sIP  + "]" + sInfo;
                }
                else //没有取到,就用配置文件中的IP
                {
                        sSendInfo = sSendInfo + "[" + g_insConfigMng.m_tComCfg.m_sServerIP + "]" + sInfo;
                }
        }
        else  //取到了
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
        g_insConCtrl.setLock();  //加锁
        for (list<CTCPSocket>::iterator it = g_lstConnectPool.begin();
                it != g_lstConnectPool.end(); it++)
        {
                 (*it).closeSocket();  //关闭
        }
        g_lstConnectPool.clear();
        g_insLogMng.normal("Close all socket connection successfully.");
        g_insConCtrl.unLock(); //解锁
        return;
}


