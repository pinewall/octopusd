//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *
//  *    $RCSfile: QueueMng.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: yourname $
//  *    $Name:  $
//  *    $Date: 2005/06/01 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "QueueMng.h"
#include "ProtocolMng.h"

extern CLogMng  g_insLogMng;
extern CConfigMng g_insConfigMng;
extern list<CTCPSocket> g_lstConnectPool;
extern string g_sHomeDir;

CQueueMng::CQueueMng()
{
        //m_iDutySeq = 0;
        m_iLastSucSeq = 0;
        m_iRecordSeq = 0;
        m_iFileSeq = 0;
        m_lTotalSizeOfRecvFile = 0;
        m_iNrOfRecvUploadFile = 0;
        m_iNrOfSendUploadFile = 0;
        m_iNrOfRecvDelFile = 0;
        m_iNrOfSendDelFile = 0;
        m_iNrOfRecvMvFile = 0;
        m_iNrOfSendMvFile = 0;
        m_iNrOfRecvCpFile = 0;
        m_iNrOfSendCpFile = 0;
}

CQueueMng::~CQueueMng()
{
}

int CQueueMng::init()
{
        //m_iDutySeq = 0;
        m_iLastSucSeq = 0;
        m_iRecordSeq = 0;
        m_iFileSeq = 0;
        m_lTotalSizeOfRecvFile = 0;
        m_iNrOfRecvUploadFile = 0;
        m_iNrOfSendUploadFile = 0;
        m_iNrOfRecvDelFile = 0;
        m_iNrOfSendDelFile = 0;
        m_iNrOfRecvMvFile = 0;
        m_iNrOfSendMvFile = 0;
        m_iNrOfRecvCpFile = 0;
        m_iNrOfSendCpFile = 0;

        //进度文件  没有什么特殊的格式，就是2行，第一行文件名称序号，第二行，记录序号
        string sPathFile =  g_sHomeDir + "/dat/failduty.seq";
        FILE* fp = fopen(sPathFile.c_str(), "r");
        if (NULL == fp)  //文件不存在
        {
                return 0;
        }

        //读取
        char sTmp[80] = "";
        fgets(sTmp, 79, fp);
        m_iFileSeq = atoi(sTmp);

        fgets(sTmp, 79, fp);
        m_iRecordSeq = atoi(sTmp);

        fclose(fp);

        return 0;
}

int CQueueMng::check(CRecvingDuty& insRecvingDuty)
{
        //返回值
        int iRet = 0;

        //进行检查时，需要锁队列
        m_insMutex.setLock();

        //======= 1.0统计信息, 接受线程处理的任务统计,不管是否处理，只要接收到就计数
        g_insLogMng.debug(insRecvingDuty.m_sSiteName, CLogMng::LogMode2, "Register statistics info.");
        m_mapRecvWorkTime[(int)pthread_self()]++;
        if (CProtocolMng::ProtocolUploadFile == insRecvingDuty.m_iProtocolType
                || CProtocolMng::ProtocolPackFile == insRecvingDuty.m_iProtocolType)
        {
                m_iNrOfRecvUploadFile++; //打包算upload
        }
        else if (CProtocolMng::ProtocolDelFile == insRecvingDuty.m_iProtocolType)
        {
                m_iNrOfRecvDelFile++;
        }
        else if (CProtocolMng::ProtocolMvFile == insRecvingDuty.m_iProtocolType)
        {
                m_iNrOfRecvMvFile++;
        }
        else if (CProtocolMng::ProtocolCpFile == insRecvingDuty.m_iProtocolType)
        {
                m_iNrOfRecvCpFile++;
        }

        //注意: 由于每次都检查,都会检查所有的队列，因此在任何队列中都不会存在相同任务

        //======= 1.1 检查正在发送的队列
        g_insLogMng.debug(insRecvingDuty.m_sSiteName, CLogMng::LogMode2, "Begin to check recving queue.");
        map<int, CRecvingDuty>::iterator it1 = m_mapRecvingQ.begin();
        for (; it1 != m_mapRecvingQ.end(); it1++)
        {
                //如果PID是自己，则直接continue，做下一步判断
                if(it1->first == (int)pthread_self())
                {
                        continue;
                }
                
                //不是正在接收的任务
                if ((it1->second).m_iState != CRecvingDuty::Normal)
                {
                        continue;
                }

                //这2个条件决定,任务是否存在
                if (0 != strcmp(insRecvingDuty.m_sSiteName, (it1->second).m_sSiteName))
                {
                        continue;
                }
                if (0 != strcmp(insRecvingDuty.m_sDestPathFile, (it1->second).m_sDestPathFile))
                {
                        continue;
                }

                //如果程序走到这个地方,说明存在了相同的任务,通过时间决定启用谁
                if (0 > strcmp(insRecvingDuty.m_sSendedTime, (it1->second).m_sSendedTime))
                {
                         g_insLogMng.normal("RecvingQ: find PID=%u has the same duty(%s, %s),I am older(%s < %s), give up me.",
                                        (unsigned int)it1->first,
                                        insRecvingDuty.m_sSiteName,
                                        insRecvingDuty.m_sDestPathFile,
                                        insRecvingDuty.m_sSendedTime,
                                        (it1->second).m_sSendedTime);
                        iRet = -1;  //放弃新的任务，因为有较新的任务在队列中
                        break;
                }
                else
                {
                         g_insLogMng.normal("RecvingQ: find PID=%u has the same duty(%s, %s),I am newer(%s >= %s), use me.",
                                        (unsigned int)it1->first,
                                        insRecvingDuty.m_sSiteName,
                                        insRecvingDuty.m_sDestPathFile,
                                        insRecvingDuty.m_sSendedTime,
                                        (it1->second).m_sSendedTime);
                        (it1->second).m_iState = CRecvingDuty::Giveup;  // give up
                        iRet = 1;  //停止老的任务，因为这个任务是较新的
                        break; //需要break，因为只可能存在这一个线程在收这一份任务
                }

        }
        if (-1 == iRet)
        {
                insRecvingDuty.m_iState = CRecvingDuty::Giveup;
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //解锁
                return iRet;
        }
        else if ( 1 == iRet)
        {
                //需要处理这个消息,加入队列 1
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //解锁
                return iRet;
        } // else iRet = 0

        //不转发的话，只有接收队列  iRet=0
        if (0 == g_insConfigMng.m_tComCfg.m_iDistributeFlag)
        {
                //需要处理这个消息,加入队列
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //解锁
                return iRet;
        }

        //======= 1.2进行待发送队列的查找， 如果在1.1种没有找到
#if 0  //为了性能，队列查找和合并的取消掉先
        g_insLogMng.debug(insRecvingDuty.m_sSiteName, CLogMng::LogMode2, "Begin to check send queue.");

        string sTmpStr;
        CSendDuty *pTmpDuty;
        
        //针对于站点的每个服务器查找
        vector <CServerInfo> vecServerInfo;
        g_insConfigMng.getSiteServer(vecServerInfo, insRecvingDuty.m_sSiteName);
        vector<CServerInfo>::iterator it2 =  vecServerInfo.begin();
        for (; it2 != vecServerInfo.end(); it2++)
        {
                char sTmpPort[32];
                sprintf(sTmpPort, "%d", (*it2).m_iPort);
                sTmpStr = string("") + insRecvingDuty.m_sSiteName + insRecvingDuty.m_sDestPathFile + (*it2).m_sIP + "_" + sTmpPort;
                //===== 查找获得是否存在相同的任务=====
                map<string, int>::iterator it21 = m_mapSendQIndex.find(sTmpStr);
                if ( m_mapSendQIndex.end() != it21)
                {
                        //快捷删除,已经是老的任务
                        //如果发现一个IP有较新的任务,则认为此任务就是老任务,就取消掉
                        if (2 == iRet)
                        {
                                m_mapSendQ.erase(it21->second); //按照索引删除
                                m_mapSendQIndex.erase(it21);  //按照iterator删除
                                continue;
                        }
                        pTmpDuty = &(m_mapSendQ[it21->second]);

                        //比较时间
                        if (0 > strcmp(insRecvingDuty.m_sSendedTime, pTmpDuty->m_sSendedTime ))
                        {
                                g_insLogMng.normal("SendQ: find the same duty(%s, %s),I am older(%s < %s), give up me.",
                                        insRecvingDuty.m_sSiteName,
                                        insRecvingDuty.m_sDestPathFile,
                                        insRecvingDuty.m_sSendedTime,
                                        pTmpDuty->m_sSendedTime);
                                iRet = -1;
                                break;
                        }
                        else  //需要取消掉队列中的任务
                        {
                                g_insLogMng.normal("SendQ: find the same duty(%s, %s),I am newer(%s < %s), use me.",
                                        insRecvingDuty.m_sSiteName,
                                        insRecvingDuty.m_sDestPathFile,
                                        insRecvingDuty.m_sSendedTime,
                                        pTmpDuty->m_sSendedTime);

                                m_mapSendQ.erase(it21->second); //按照索引删除
                                m_mapSendQIndex.erase(it21);  //按照iterator删除

                                iRet = 2;  //赋值为2,表示新的任务需要添加,需要删除掉所有的老任务
                        }
                }
        }
        if (-1 == iRet)
        {
                 insRecvingDuty.m_iState = CRecvingDuty::Giveup;
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //解锁
                return iRet;
        }
        else if ( 2 == iRet)
        {
                //需要处理这个消息,加入队列 1
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //解锁
                return iRet;
        } // else iRet = 0
#endif

        //====== 1.3查找发送队列 如果在1.1和1.2中都没有找到，或者在1.2中只是取消了部分任务
        g_insLogMng.debug(insRecvingDuty.m_sSiteName, CLogMng::LogMode2, "Begin to check sending queue.");

        map<int, CSendDuty>::iterator it3 = m_mapSendingQ.begin();
        for (; it3 != m_mapSendingQ.end(); it3++)
        {
                //如果PID是自己，则直接continue，做下一步判断
                if(it3->first == (int)pthread_self())
                {
                        continue;
                }
                
                if ((it3->second).m_iState != CSendDuty::Normal)
                {
                        continue;
                }

                //这2个条件决定,任务是否存在
                if (0 != strcmp(insRecvingDuty.m_sSiteName, (it3->second).m_sSiteName))
                {
                        continue;
                }
                if (0 != strcmp(insRecvingDuty.m_sDestPathFile, (it3->second).m_sDestPathFile))
                {
                        continue;
                }

                //比较时间
                if (0 > strcmp(insRecvingDuty.m_sSendedTime, (it3->second).m_sSendedTime))
                {
                         g_insLogMng.normal("SendingQ: find PID=%u have the same duty(%s, %s),I am older(%s < %s), give up me.",
                                        (unsigned int)it3->first,
                                        insRecvingDuty.m_sSiteName,
                                        insRecvingDuty.m_sDestPathFile,
                                        insRecvingDuty.m_sSendedTime,
                                        (it3->second).m_sSendedTime);
                        iRet = -1;  //放弃新的任务，因为有较新的任务在队列中
                        break;
                }
                else
                {
                         g_insLogMng.normal("SendingQ: find PID=%u have the same duty(%s, %s),I am newer(%s >= %s), use me.",
                                        (unsigned int)it3->first,
                                        insRecvingDuty.m_sSiteName,
                                        insRecvingDuty.m_sDestPathFile,
                                        insRecvingDuty.m_sSendedTime,
                                        (it3->second).m_sSendedTime);
                        (it3->second).m_iState = CSendDuty::Giveup;  // give up
                        iRet = 3;  //停止老的任务，因为这个任务是较新的
                        // not break; 因为发送任务可能存在多个发送线程中, 所以需要都检查一遍
                }
        }
        if (-1 == iRet)
        {
                 insRecvingDuty.m_iState = CRecvingDuty::Giveup;
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //解锁
                return iRet;
        }
        else if ( 3 == iRet)
        {
                //需要处理这个消息,加入队列 1
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //解锁
                return iRet;
        } // else iRet = 0

#if 0  //发送成功队列，有些鸡肋，暂时去掉 2007-06-13
        // 1.4查找最近成功发送队列 如果在1.1和1.2和1.3中都没有找到，或者在1.2或1.3中只是取消了部分任务
        g_insLogMng.debug(insRecvingDuty.m_sSiteName, CLogMng::LogMode2, "Begin to check LastSuc queue.");
        sTmpStr = string("") + insRecvingDuty.m_sSiteName + insRecvingDuty.m_sDestPathFile;
        map<string, string>::iterator it4 = m_mapLastSucQ.find(sTmpStr);
        if (m_mapLastSucQ.end() != it4)
        {
                if(0 > strcmp(insRecvingDuty.m_sSendedTime, (it4->second).c_str()))
                {
                         g_insLogMng.normal("LastSucQ: find the same duty(%s, %s),I am older(%s < %s), give up me.",
                                        insRecvingDuty.m_sSiteName,
                                        insRecvingDuty.m_sDestPathFile,
                                        insRecvingDuty.m_sSendedTime,
                                        (it4->second).c_str());
                         iRet = -1;  //放弃新的任务，因为有较新的任务在队列中
                }
                else
                {
                        g_insLogMng.debug("protocol", CLogMng::LogMode2, "LastSucQ: find the same duty(%s, %s),I am newer(%s >= %s), use me.",
                                        insRecvingDuty.m_sSiteName,
                                        insRecvingDuty.m_sDestPathFile,
                                        insRecvingDuty.m_sSendedTime,
                                        (it4->second).c_str());
                        iRet = 4;
                }
        }
        if (-1 == iRet)
        {
                insRecvingDuty.m_iState = CRecvingDuty::Giveup;
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //解锁
                return iRet;
        }
        else if ( 4 == iRet)
        {
                //需要处理这个消息,加入队列 1
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //解锁
                return iRet;
        } // else iRet = 0
#endif 
        
        g_insLogMng.debug(insRecvingDuty.m_sSiteName, CLogMng::LogMode2, "Not find the same duty, deal with me(%s).",
                        insRecvingDuty.getPrintInfo().c_str());
        m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty; //加上队列
        m_insMutex.unLock();  //解锁
        return 0;
}

int CQueueMng::judgeNormal(int pID, QueueType eQueueType)
{
        //当前只需要实现接收队列和发送队列
        if (RecvingQueue == eQueueType)
        {
                if (CRecvingDuty::Normal == m_mapRecvingQ[pID].m_iState)
                {
                        return 0;
                }
                else
                {
                        return -1;
                }
        }
        else if (SendingQueue == eQueueType)
        {
                if (CSendDuty::Normal == m_mapSendingQ[pID].m_iState)
                {
                        return 0;
                }
                else
                {
                        return -1;
                }
        }

        return 0;
}

int CQueueMng::setOver(int pID, QueueType eQueueType)
{
        //当前只需要实现接收队列和发送队列
        if (RecvingQueue == eQueueType)
        {
                m_mapRecvingQ[pID].m_iState = CRecvingDuty::Over;
        }
        else if (SendingQueue == eQueueType)
        {
                m_mapSendingQ[pID].m_iState = CSendDuty::Over;
        }

        return 0;
}

int CQueueMng::putDuty2SendQ(CSendDuty& insSendDuty, PutSendType ePutSendType, int iRecvFileSize)
{
        string sTmpStr;

        //进行检查时，需要锁队列
        m_insMutex.setLock();        

        //一次例行检查，肯定能足够保证队列序号的正确性,不会出现永远不为空的情况: 当map为空时，将队列的序号重新清零
        //if (0 == m_mapSendQ.size())
        //{                        
        //        m_iDutySeq = 0;
        //}

        //统计信息,统计接收的文件大小，只需要统计接收到的文件(因为其他的地方，输入的文件大小都是0)
        m_lTotalSizeOfRecvFile = m_lTotalSizeOfRecvFile + iRecvFileSize;        

        //需要添加服务器信息
        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode2, "Begin to add this msg to send queue.");
        if (AddServerInfo == ePutSendType)
        {
                //正在接收的任务结束
                int pID = (int)pthread_self();
                if (m_mapRecvingQ[pID].m_iState == CRecvingDuty::Normal)
                {
                        m_mapRecvingQ[pID].m_iState = CRecvingDuty::Over;
                }
                else  //任务已经被取消了
                {
                        g_insLogMng.normal( "This duty(%s) has been canceled, give up it.", insSendDuty.getPrintInfo().c_str());
                        m_insMutex.unLock();  //解锁
                        return 0;
                }

                //如果不需要转发，则直接结束
                if (0 == g_insConfigMng.m_tComCfg.m_iDistributeFlag)
                {
                        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode3, "Need not forward, this duty is over.");
                        m_insMutex.unLock();  //解锁
                        return 0;
                }

                //根据服务器信息，依次加入队列
                vector<CServerInfo> insSiteServer;
                g_insConfigMng.getSiteServer(insSiteServer, insSendDuty.m_sSiteName);
                for (vector<CServerInfo>::iterator it = insSiteServer.begin(); it != insSiteServer.end(); it++)
                {
                        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode2, "Config server info(%s, %d)",
                                (*it).m_sIP, (*it).m_iPort);
                        insSendDuty.m_iNrOfFail = 0;
                        strcpy(insSendDuty.m_sServerIP, (*it).m_sIP);
                        insSendDuty.m_iServerPort = (*it).m_iPort;
                        
                        //加入队列
                        char sTmpPort[32];
                        sprintf(sTmpPort, "%d", (*it).m_iPort);
                        string sTmpipp = string("") + (*it).m_sIP + "_" + sTmpPort;
                        
                        //判断一下，应该加入那个队列
                        unsigned int iFailTimes = 0;
                        map<string, unsigned int>::iterator itfind = m_mapErrorIPPort.find(sTmpipp);
                        if (itfind != m_mapErrorIPPort.end())  //正常的任务
                        {
                                iFailTimes = itfind->second;  //次数
                        }
                        if (iFailTimes < MaxTimesToErrorIPList)  //正常的任务
                        {
                                if (0 != g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty)
                                {
                                        //队列是否满
                                        if (m_lstSendQ.size() >= (unsigned int)g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty)
                                        {
                                                g_insLogMng.error("SendQ: SendQ is full(%d),put this duty to failduty file.",
                                                        g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty);
                                                putFailDuty2File(insSendDuty); //防止锁冲突，里面不能再次加相同的锁
                                                continue;  //继续处理下一个IP
                                        }
                                }
                                //sTmpStr = string("") + insSendDuty.m_sSiteName + insSendDuty.m_sDestPathFile + (*it).m_sIP + "_" + sTmpPort;
                                //m_mapSendQ[m_iDutySeq] = insSendDuty;
                                //m_mapSendQIndex[sTmpStr] =  m_iDutySeq;
                                //m_iDutySeq++;
                                m_lstSendQ.push_back(insSendDuty);
                                g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode2, "Add Duty(%s) to Q ok.", 
                                        insSendDuty.getPrintInfo().c_str());
                        }
                        else  //失败过的IP和Port
                        {
                                unsigned int iMaxOfErrorQueue = g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty;
                                if (0 == iMaxOfErrorQueue)
                                {
                                        iMaxOfErrorQueue = DefaultMaxOfErrorQueue; 
                                }
                                if (m_lstSendErrorQ.size() >= iMaxOfErrorQueue)
                                {
                                        g_insLogMng.error("ErrorSendQ: ErrorSendQ is full(%d),put this duty to failduty file.",
                                                g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty);
                                        putFailDuty2File(insSendDuty); //防止锁冲突，里面不能再次加相同的锁
                                        continue;  // 继续下一个
                                }
                                m_lstSendErrorQ.push_back(insSendDuty);  //失败链表，直接加到后面
                                g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode2, "Add Duty(%s) to Error Q ok.", 
                                        insSendDuty.getPrintInfo().c_str());
                        }
                }
        }
        else if (Direct == ePutSendType)
        {
                //如果不需要转发，则直接结束
                if (0 == g_insConfigMng.m_tComCfg.m_iDistributeFlag)
                {
                        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode3, "Need not forward, can't add to send queue.");
                        m_insMutex.unLock();  //解锁
                        return 0;
                } 
                                
                char sTmpPort[32];
                sprintf(sTmpPort, "%d", insSendDuty.m_iServerPort);
                //加入队列
                string sTmpipp = string("") + insSendDuty.m_sServerIP + "_" + sTmpPort;          
                if (insSendDuty.m_iNrOfFail > 0)  //第一次加入的时候是0，不能计数
                {
                        m_mapErrorIPPort[sTmpipp]++;  //增加记录
                        //写个normal日志，其他写debug,  恰好是第MaxTimesToErrorIPList次时                 
                        if (MaxTimesToErrorIPList == m_mapErrorIPPort[sTmpipp])
                        {
                                 g_insLogMng.normal( "===ErrorIPPort: %s is created === for Duty(%s) failed. ==",
                                                sTmpipp.c_str(), (insSendDuty.getPrintInfo()).c_str());
                        }
                        else
                        {
                                g_insLogMng.debug("protocol", CLogMng::LogMode2, "===ErrorIPPort: %s is created === for Duty(%s) failed. ==",
                                                sTmpipp.c_str(), (insSendDuty.getPrintInfo()).c_str());
                        }

                        //超过了失败次数，则直接加入失败文件
                        if (insSendDuty.m_iNrOfFail >= g_insConfigMng.m_tComCfg.m_iMaxNrOfRetry)
                        {
                                 g_insLogMng.error("DirectSendQ: the duty(%s) NrOfFail >= Max(%d), so write to file.",
                                        insSendDuty.getPrintInfo().c_str(), g_insConfigMng.m_tComCfg.m_iMaxNrOfRetry);
                                putFailDuty2File(insSendDuty); //防止锁冲突，里面不能再次加相同的锁
                                m_insMutex.unLock();  //解锁
                                return  -1;
                        }
                        
                }
                
                //判断一下，应该加入那个队列
                unsigned int iFailTimes = 0;
                map<string, unsigned int>::iterator itfind = m_mapErrorIPPort.find(sTmpipp);
                if (itfind != m_mapErrorIPPort.end())  //正常的任务
                {
                        iFailTimes = itfind->second;  //次数
                }
                if (iFailTimes < MaxTimesToErrorIPList)                
                {
                        //直接判断加入
                        if (0 != g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty)
                        {
                               if (m_lstSendQ.size() >= (unsigned int)g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty)
                               {
                                       g_insLogMng.error("SendQ: SendQ is full(%d),put this duty to failduty file..",
                                                g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty);
                                        putFailDuty2File(insSendDuty); //防止锁冲突，里面不能再次加相同的锁
                                        m_insMutex.unLock();  //解锁
                                        return  -1;
                               }
                        }
                        //sTmpStr = string("") + insSendDuty.m_sSiteName + insSendDuty.m_sDestPathFile + insSendDuty.m_sServerIP + "_" + sTmpPort;
                        //m_mapSendQ[m_iDutySeq] = insSendDuty;
                        //m_mapSendQIndex[sTmpStr] =  m_iDutySeq;
                        //m_iDutySeq++;
                        m_lstSendQ.push_back(insSendDuty);
                        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode2, "Direct add Duty(%s) to Q ok", 
                                 insSendDuty.getPrintInfo().c_str());
                }
                else  //任务如果有MaxTimesToErrorIPList次失败，则加入失败队列
                {      
                        //失败队列由于不存在合并操作，因此强制限定最大值
                        unsigned int iMaxOfErrorQueue = g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty;
                        if (0 == iMaxOfErrorQueue)
                        {
                                iMaxOfErrorQueue = DefaultMaxOfErrorQueue;
                        }
                        if (m_lstSendErrorQ.size() >= iMaxOfErrorQueue)
                        {
                               g_insLogMng.error("ErrorSendQ: SendQ is full(%d),put this duty to failduty file..",
                                        g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty);
                                putFailDuty2File(insSendDuty); //防止锁冲突，里面不能再次加相同的锁
                                m_insMutex.unLock();  //解锁
                                return  -1;
                        }
                        m_lstSendErrorQ.push_back(insSendDuty);
                        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode2, "Direct add Duty(%s) to Error Q ok", 
                                 insSendDuty.getPrintInfo().c_str());
                }
        }

        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode2, "End to add this msg to send queue.");
        m_insMutex.unLock();  //解锁
        return 0;
}

int CQueueMng::getDutyFSendQ(int iType, CSendDuty& insSendingDuty)  // 2007-05-21 add itype
{
        //进行检查时，需要锁队列
        m_insMutex.setLock();
        int iLocalType = iType;
        if (0 == iLocalType)
        {
                if (m_lstSendQ.size() == 0) //正常队列不存在任务
                {
                        if(m_lstSendErrorQ.size() == 0) //失败任务队列
                        {
                                m_insMutex.unLock();  //解锁
                                return -1;
                        }
                        else
                        {
                                iLocalType = 1; //由取正常任务队列,转化成取失败任务队列
                        }
                }  //else 维持找正常任务          
        }
        else if (1 == iLocalType)
        {
                if (m_lstSendErrorQ.size() == 0) //失败任务队列
                {
                        if(m_lstSendQ.size() == 0) //正常任务队列
                        {
                                m_insMutex.unLock();  //解锁
                                return -1;
                        }
                        else
                        {
                                iLocalType = 0; //由取失败任务队列,转化成取正常任务队列
                        }
                }  //else 维持找失败任务
        }
        else
        {
                 m_insMutex.unLock();  //解锁
                return -1;
        }

        // 2007-05-21 add itype
        //队列为空,返回-1
        if (0 == iLocalType)  //常规任务
        {
                //从头部取任务,加入正在发送队列
                list<CSendDuty>::iterator it = m_lstSendQ.begin();
                insSendingDuty = *it;
                m_lstSendQ.erase(it);

                //清除掉索引
                char sTmpPort[32];
                sprintf(sTmpPort, "%d", insSendingDuty.m_iServerPort);
                //string sTmpStr = string("") + insSendingDuty.m_sSiteName + insSendingDuty.m_sDestPathFile + insSendingDuty.m_sServerIP + "_" + sTmpPort;
                //m_mapSendQIndex.erase(sTmpStr);
                //清除掉任务
                //m_mapSendQ.erase(it);                
                //处理任务
                string sTmpipp = string(insSendingDuty.m_sServerIP) + "_" + sTmpPort;
                unsigned int iFailTimes = 0;
                map<string, unsigned int>::iterator itfind = m_mapErrorIPPort.find(sTmpipp);
                if (itfind != m_mapErrorIPPort.end())  //正常的任务
                {
                        iFailTimes = itfind->second;  //次数
                }
                if (iFailTimes >= MaxTimesToErrorIPList)
                {
                        //加入到失败队列中
                        m_lstSendErrorQ.push_back(insSendingDuty);
                        m_insMutex.unLock();  //解锁
                        return -1;
                } 
        }
        else if (1 == iLocalType)  //重试任务
        {
                //从头部取任务,加入正在发送队列
                list<CSendDuty>::iterator it = m_lstSendErrorQ.begin();
                insSendingDuty = *it;
                m_lstSendErrorQ.erase(it);
        }
        else
        {
                m_insMutex.unLock();  //解锁
                return -1;
        }

        //统计信息, 发送线程处理的任务统计,不管是否处理，只要摘下就计数
        m_mapSendWorkTime[(int)pthread_self()]++;
        
        m_mapSendingQ[(int)pthread_self()]= insSendingDuty;
        
        if (CProtocolMng::ProtocolDelFile == insSendingDuty.m_iProtocolType) 
        {
                m_iNrOfSendDelFile++; 
        }
        else if (CProtocolMng::ProtocolUploadFile == insSendingDuty.m_iProtocolType
                || CProtocolMng::ProtocolPackFile == insSendingDuty.m_iProtocolType) 
        {                
                m_iNrOfSendUploadFile++;  //打包上传等，都算作upload
        }
        else if (CProtocolMng::ProtocolMvFile == insSendingDuty.m_iProtocolType) 
        {
                m_iNrOfSendMvFile++;
        }
        else if (CProtocolMng::ProtocolCpFile == insSendingDuty.m_iProtocolType) 
        {
                m_iNrOfSendCpFile++;
        }

        m_insMutex.unLock();  //解锁
        return 0;
}

int CQueueMng::putDuty2LastSucQ(CSendDuty& insLastSucDuty)
{
#if 0
        //是否需要处理最近成功队列
        if (0 == g_insConfigMng.m_tComCfg.m_iMaxNrOfSaveLastSucDuty)
        {
                return 0;
        }

        //进行检查时，需要锁队列
        m_insMutex.setLock();

        //序号足够大时，清空队列重新编号  8 个9，这样可能会导致问题，但是不影响大局,索引不搞很复杂的算法了
        if (m_iLastSucSeq >= 99999999)
        {
                m_iLastSucSeq = 0;
                m_mapLastSucIndexQ.clear();
                m_mapLastSucQ.clear();
        }

        //超大清空
        if (m_mapLastSucQ.size() >= (unsigned int)g_insConfigMng.m_tComCfg.m_iMaxNrOfSaveLastSucDuty)
        {
                map<int, string>::iterator it = m_mapLastSucIndexQ.begin();

                //清理较老的成功队列的数据
                m_mapLastSucQ.erase(it->second);

                //清理掉索引队列的数据
                m_mapLastSucIndexQ.erase(it);
        }

        //加入队列和索引队列
        string sTmpStr = string("") + insLastSucDuty.m_sSiteName + insLastSucDuty.m_sDestPathFile;
        m_mapLastSucQ[sTmpStr] = insLastSucDuty.m_sSendedTime;
        m_mapLastSucIndexQ[m_iLastSucSeq++] = sTmpStr;

        m_insMutex.unLock();  //解锁
#endif

        return 0;

}

//不锁文件，因为能保证这个函数只在锁的情况下才调用
void CQueueMng::putFailDuty2File(CSendDuty& insFailDuty)
{
        if (0 == g_insConfigMng.m_tComCfg.m_iIfFailLogFlag)
        {
                return;
        }

        //为了防止无删除文件，删除文件的失败信息不写入失败文件，当发生问题时，手工再删除
        if (CProtocolMng::ProtocolDelFile == insFailDuty.m_iProtocolType)
        {
                return;
        }

        if (m_iRecordSeq >= MaxSeqOfFailRecord)  //记录达到上限
        {
                m_iFileSeq++;  //文件名称加1
                m_iRecordSeq = 0;
        }

        //文件名循环使用
        if (MaxSeqOfFailFileName < m_iFileSeq)
        {
                m_iFileSeq = 0;
        }

        //要写入的文件名称
        char sTmp[LenOfPathFileName];
        sprintf(sTmp, "failduty.%04d", m_iFileSeq);
        string sPathFile = g_sHomeDir + "/dat/" + sTmp;

        //写入文件
        FILE *fp = fopen(sPathFile.c_str(), "a+");
        if (NULL == fp)
        {
                return;
        }

        unsigned int iTmpSize = sizeof(CSendDuty);
        if (1 != fwrite((char*)&insFailDuty, iTmpSize, 1, fp))
        {
                fclose(fp);

                return;
        }

        fclose(fp);

        //记录数++
        m_iRecordSeq++;

        //进度文件  没有什么特殊的格式，就是2行，第一行文件名称序号，第二行，记录序号
        sPathFile =  g_sHomeDir + "/dat/failduty.seq";
        fp = fopen(sPathFile.c_str(), "w");
        if (NULL == fp)
        {
                return;
        }

        sprintf(sTmp, "%d\n", m_iFileSeq);
        fputs(sTmp, fp);

        sprintf(sTmp, "%d\n", m_iRecordSeq);
        fputs(sTmp, fp);

        fclose(fp);

        return;
}

void CQueueMng::getFailDutyFFile2DutyQ(int iFileSeq)
{
        //要写入的文件名称
        char sTmp[LenOfPathFileName];
        sprintf(sTmp, "failduty.%04d", iFileSeq);
        string sPathFile1 = g_sHomeDir + "/dat/" + sTmp;
        string sPathFile2 = g_sHomeDir + "/dat/failduty.do";

        //防止重写，将文件mv后再操作
        if (0 != CComFun::mvFile(sPathFile1, sPathFile2))
        {
                g_insLogMng.debug("protocol",CLogMng::LogMode3, "%s is not exsit.", sPathFile1.c_str());
                return;
        }

        //写入文件
        FILE *fp = fopen(sPathFile2.c_str(), "r");
        if (NULL == fp)
        {
                return;  //文件不存在
        }
        
        g_insLogMng.normal("Begin to reload %s", sPathFile1.c_str());
        CSendDuty insFailDuty;
        CRecvingDuty insRecvingDuty;
        unsigned int iTmpSize = sizeof(CSendDuty);
        while(!feof(fp))
        {
                if (1 != fread((char*)&insFailDuty, iTmpSize, 1, fp))
                {
                        break;
                }
                //主要是用来判断记录格式的正确性，防止文件错位
                if (insFailDuty.m_iProtocolType != CProtocolMng::ProtocolUploadFile
                        && insFailDuty.m_iProtocolType != CProtocolMng::ProtocolDelFile
                        && insFailDuty.m_iProtocolType != CProtocolMng::ProtocolMvFile
                        && insFailDuty.m_iProtocolType != CProtocolMng::ProtocolCpFile
                        && insFailDuty.m_iProtocolType != CProtocolMng::ProtocolPackFile)  // add by kevinliu 2007-01-10
                {
                        g_insLogMng.error("Error failed duty protocoltype(%d)",insFailDuty.m_iProtocolType );
                        continue;
                }

                //重置任务的状态
                insFailDuty.m_iState = CSendDuty::Normal;
                insFailDuty.m_iNrOfFail = 0;

                //取本地文件名
                string sLocalPathFile;
                if ( insFailDuty.m_iProtocolType == CProtocolMng::ProtocolMvFile
                        || insFailDuty.m_iProtocolType == CProtocolMng::ProtocolCpFile)
                {
                       vector<string> tmpPathInfo;
                        CComFun::divideupStr(insFailDuty.m_sLocalPathFile, tmpPathInfo, ':');
                        if (2 != tmpPathInfo.size())
                        {
                                 g_insLogMng.error("MvCp file failed, analysePathFile(%s) is an invalid pathfile.",
                                       insFailDuty.m_sLocalPathFile);  
                                continue;
                        }  
                        sLocalPathFile = g_insConfigMng.getSiteRootDir(tmpPathInfo[0]);
                        if ("" == sLocalPathFile)
                        {
                                 g_insLogMng.error("MvCp file failed, %s site not exist.",
                                       tmpPathInfo[0].c_str());  
                                 continue;
                        }
                        sLocalPathFile = CComFun::analysePathFile(sLocalPathFile, tmpPathInfo[1], 0);
                        if ("" == sLocalPathFile)
                        {
                                  g_insLogMng.error("MvCp file failed, analysePathFile(%s) is an invalid pathfile.",
                                       insFailDuty.m_sLocalPathFile);  
                                continue;
                        }  
                }
                else
                {
                        sLocalPathFile = insFailDuty.m_sLocalPathFile;
                }

                //判断任务是否已经过期,不管是什么任务,先放入队列再说,在发送的时候再检查有效性
                struct stat tmpStat;
                if(0 <= stat(sLocalPathFile.c_str(), &tmpStat))
                {
                        //同样能解决,删除文件的问题,存在了,肯定是老的任务,不存在,发送肯定不会有问题
                        if (tmpStat.st_mtime > insFailDuty.m_iLocalTime)
                        {
                                g_insLogMng.normal("WhenLoadFile: %s has been modified, give up.",
                                        insFailDuty.m_sLocalPathFile);
                                continue;
                        }
                }

                //需要check一下才可以，否则会有可能有重复的任务
                strcpy(insRecvingDuty.m_sSiteName,insFailDuty.m_sSiteName);
                insRecvingDuty.m_iState = CRecvingDuty::Normal;
                insRecvingDuty.m_iProtocolType = insFailDuty.m_iProtocolType;
                strcpy(insRecvingDuty.m_sDestPathFile, insFailDuty.m_sLocalPathFile);
                strcpy(insRecvingDuty.m_sSendedTime, insFailDuty.m_sSendedTime);
                if ( 0 > check(insRecvingDuty))
                {
                        continue;
                }
                //放入队列
                putDuty2SendQ(insFailDuty, Direct);
        }

        fclose(fp);
        CComFun::delFile(sPathFile2);  //将do文件一并删除
        
        return;
}

void CQueueMng::printCurInfo()
{
        //自己写一个独立的文件
        char sTmp[1024];
        sprintf(sTmp, "\n======%s cur Info======\n SendDutySize = %d\n ErrorSendDutySize = %d\n LastSucSeq = %d\n"
                                " LastSucQSize = %d\n FailDutyFileSeq = %d\n FailDutyRecordSeq = %d\n ======queue info ======\n",
                (CComFun::GetExactCurDateTime()).c_str(),
                m_lstSendQ.size(),
                m_lstSendErrorQ.size(),
                m_iLastSucSeq,
                m_mapLastSucQ.size(),
                m_iFileSeq,
                m_iRecordSeq
                );

        string myFile = g_sHomeDir + "/log/prt/queue.prt";
        FILE *fp = fopen(myFile.c_str(), "a+");  
        if (NULL == fp)
        {
                return;
        }

        fputs(sTmp, fp);

        //错误的IP队列
        m_insMutex.setLock();
        map<string, unsigned int> tmpErorrIPPort = m_mapErrorIPPort;
        m_insMutex.unLock();     
        
        map<string, unsigned int>::iterator itip = tmpErorrIPPort.begin();
        for(;itip != tmpErorrIPPort.end();itip++)
        {
               fprintf(fp, "ErrorIPPort=%s:%u\n", itip->first.c_str(), itip->second);
        }

        //队列的详细信息
        if (0 == access((myFile + ".if").c_str(),  F_OK))  //如果存在queue.prt.if文件，则打印详细信息
        {
                //复制出一份数据
                m_insMutex.setLock();
                list<CSendDuty> tmpOkQueue = m_lstSendQ;
                list<CSendDuty> tmplstQueue = m_lstSendErrorQ;
                m_insMutex.unLock(); 

                //打印队列中的内容到文件中去
                //正常队列
                list<CSendDuty>::iterator it = tmpOkQueue.begin();
                for(;it != tmpOkQueue.end();it++)
                {
                       fprintf(fp, "TYPE>>Ok;INFO>>%d:%s:%s:%s:%s:%d\n", 
                                it->m_iProtocolType, 
                                it->m_sSiteName, 
                                it->m_sDestPathFile, 
                                it->m_sSendedTime,
                                it->m_sServerIP,
                                it->m_iServerPort);
                }
                //失败队列
                list<CSendDuty>::iterator it2 = tmplstQueue.begin();
                for(it2 = tmplstQueue.begin();it2 != tmplstQueue.end();it2++)
                {
                       fprintf(fp, "TYPE>>Error;INFO>>%d:%s:%s:%s:%s:%d\n", 
                                it2->m_iProtocolType, 
                                it2->m_sSiteName, 
                                it2->m_sDestPathFile, 
                                it2->m_sSendedTime,
                                it2->m_sServerIP,
                                it2->m_iServerPort);
                }
        }
        
        fclose(fp);

        return;
}

void CQueueMng::getCurInfo(string& sInfo)
{
        //自己写一个独立的文件
        char sTmp[1024];
        sprintf(sTmp, "\n======%s cur Info======\n NrOfConnecting = %d\n SendDutySize = %d\n ErrorSendDutySize = %d\n LastSucSeq = %d\n"
                                " LastSucQSize = %d\n FailDutyFileSeq = %d\n FailDutyRecordSeq = %d\n ======queue info ======\n",
                (CComFun::GetExactCurDateTime()).c_str(),
                g_lstConnectPool.size(),
                m_lstSendQ.size(),
                m_lstSendErrorQ.size(),
                m_iLastSucSeq,
                m_mapLastSucQ.size(),
                m_iFileSeq,
                m_iRecordSeq
                );
        sInfo = string("")  + sTmp;  //开始信息

        //错误的IP队列
        m_insMutex.setLock();
        map<string, unsigned int> tmpErorrIPPort = m_mapErrorIPPort;
        m_insMutex.unLock();     
        
        map<string, unsigned int>::iterator itip = tmpErorrIPPort.begin();
        for(;itip != tmpErorrIPPort.end();itip++)
        {
               sprintf(sTmp, "ErrorIPPort=%s:%u\n", itip->first.c_str(), itip->second);
               sInfo = sInfo + sTmp; //依次
        }
        return;
}


void CQueueMng::printStatInfo()
{
        string sStatInfo = "";
        char sTmp[1024];
        sprintf(sTmp, "\n======%s stat Info======\n NrOfConnecting = %d\n"
                " TotalSizeOfRecvFile = %ld\n "
                "NrOfRecvUploadFile = %d\n NrOfRecvDelFile = %d\n NrOfRecvMvFile = %d\n NrOfRecvCpFile = %d\n "
                "NrOfSendUploadFile = %d\n NrOfSendDelFile = %d\n NrOfSendMvFile = %d\n NrOfSendMvFile = %d\n",
                CComFun::GetExactCurDateTime().c_str(),
                g_lstConnectPool.size(),
                m_lTotalSizeOfRecvFile,
                m_iNrOfRecvUploadFile,
                m_iNrOfRecvDelFile,
                m_iNrOfRecvMvFile,
                m_iNrOfRecvCpFile,
                m_iNrOfSendUploadFile,
                m_iNrOfSendDelFile,
                m_iNrOfSendMvFile,
                m_iNrOfSendCpFile                
                );

        //接收线程
        sStatInfo = sStatInfo + sTmp + " Recv Thread:\n";
        for(map<int,int>::iterator it = m_mapRecvWorkTime.begin();
                it != m_mapRecvWorkTime.end(); it++)
        {
                sprintf(sTmp, " WorkTimesOfThread(%d) = %d\n", it->first, it->second);
                sStatInfo = sStatInfo + sTmp;
                 it->second = 0;//清掉
        }

        //发送线程
        sStatInfo = sStatInfo + " Send Thread:\n";
        for(map<int,int>::iterator it = m_mapSendWorkTime.begin();
                it != m_mapSendWorkTime.end(); it++)
        {
                sprintf(sTmp, " WorkTimesOfThread(%d) = %d\n", it->first, it->second);
                sStatInfo = sStatInfo + sTmp;
                it->second = 0;//清掉
        }

        //清掉
        m_lTotalSizeOfRecvFile = 0;
        m_iNrOfRecvUploadFile = 0;
        m_iNrOfRecvDelFile = 0;
        m_iNrOfRecvMvFile = 0;
        m_iNrOfRecvCpFile = 0;
        m_iNrOfSendUploadFile = 0;
        m_iNrOfSendDelFile = 0;
        m_iNrOfSendMvFile = 0;
        m_iNrOfSendCpFile = 0;

        string myFile = g_sHomeDir + "/log/stat/queue"+ CComFun::GetCurDate() +".stat";
        FILE *fp = fopen(myFile.c_str(), "a+");
        if (NULL == fp)
        {
                return;
        }

        fputs(sStatInfo.c_str(), fp);
        fclose(fp);

        return;
}

void CQueueMng::printStatInfoXML() //必须保证先于printStatInfo执行，否则数据就会被更新了
{
        string sStatInfo = "";
        char sTmp[1024];
        sprintf(sTmp, "<node>\n"
                           "    <time>%s</time>\n"
                           "    <recvsize>%ld</recvsize>\n"
                           "    <recvnum>%d</recvnum>\n"
                           "    <sendnum>%d</sendnum>\n"
                            "</node>\n", 
                            CComFun::GetExactCurDateTime().c_str(),
                            m_lTotalSizeOfRecvFile,
                            m_iNrOfRecvUploadFile,
                            m_iNrOfSendUploadFile
                            );
        sStatInfo = sTmp;
        string myFile = g_sHomeDir + "/log/stat/queue"+ CComFun::GetCurDate() +".xml";
        FILE *fp = fopen(myFile.c_str(), "a+");
        if (NULL == fp)
        {
                return;
        }

        fputs(sStatInfo.c_str(), fp);
        fclose(fp);

        return;
}


int CQueueMng::getQueueInfo()
{
        return m_lstSendQ.size() + m_lstSendErrorQ.size();  //kevinliu modify 2007-05-21
}

void CQueueMng::initThreadStatInfo(int iThreadType, int iThreadID)
{
        m_insMutex.setLock();
        if (0 == iThreadType)
        {
                m_mapRecvWorkTime[iThreadID] = 0;
        }
        else if (1 == iThreadType)
        {
                m_mapSendWorkTime[iThreadID] = 0;
        }
        m_insMutex.unLock();
        return;
}

void CQueueMng::saveDutyQueue2File()
{
        g_insLogMng.normal("Save all duties to failed files.");  
        list<CSendDuty>::iterator it = m_lstSendQ.begin();
        for (;it != m_lstSendQ.end(); it++)
        {
                putFailDuty2File(*it);
        }
        //kevinliu add 2007-05-17
        list<CSendDuty>::iterator it2 = m_lstSendErrorQ.begin();
        for (it2 = m_lstSendErrorQ.begin(); it2 != m_lstSendErrorQ.end(); it2++)
        {
                putFailDuty2File(*it2);
        }
        return;
}

void CQueueMng::getErrorIPPort(map<string, unsigned int>& m_mapIPPort)
{
        m_insMutex.setLock();
        m_mapIPPort = m_mapErrorIPPort; 
        m_insMutex.unLock();
        return;
}

//为什么情况，在外面根据网络状态判断
void CQueueMng::clearErrorIPPort(string ipport)
{
        m_insMutex.setLock();
        if ("" == ipport)
        {
                g_insLogMng.normal("m_mapErrorIPPort.clear()");  
                m_mapErrorIPPort.clear();  
        }
        else
        {
                g_insLogMng.normal("m_mapErrorIPPort.erase(%s)", ipport.c_str());  
                m_mapErrorIPPort.erase(ipport);  
        }
        m_insMutex.unLock();
        return;
}

//例行处理任务，将失败队列中,IP已经恢复的任务，转移到成功队列中，防止因为有有问题的IP
void CQueueMng::mvErrorQ2SendQ()
{
        m_insMutex.setLock();
        if (m_lstSendErrorQ.size() == 0)
        {
                m_insMutex.unLock();  //解锁
                return;
        }
        //从头部取任务，判断IP_Port是否已经被移除出失败队列，如果已经，则将任务加到正常的队列中去
        CSendDuty insSendingDuty;
        list<CSendDuty>::iterator itduty = m_lstSendErrorQ.begin();
        insSendingDuty = *itduty;
        m_lstSendErrorQ.erase(itduty);

        char sTmpPort[64];
        sprintf(sTmpPort, "%d", insSendingDuty.m_iServerPort);
        string sTmpipport = string("") + insSendingDuty.m_sServerIP + "_" + sTmpPort;
        //string sTmpStr = string("") + insSendingDuty.m_sSiteName + insSendingDuty.m_sDestPathFile + sTmpipport;

        //没有发现，说明已经解封了
        if(m_mapErrorIPPort.end() == m_mapErrorIPPort.find(sTmpipport)) 
        {
                g_insLogMng.debug("all", CLogMng::LogMode1, "Duty(%s) is moved from EQ to Q.",
                        insSendingDuty.getPrintInfo().c_str());

                //为了提高速度，不用判断是否存在相同的任务，因为即使存在，m_mapSendQIndex队列小于m_mapSendQ,系统也不会出错的
                //m_mapSendQ[m_iDutySeq] = insSendingDuty;
                //m_mapSendQIndex[sTmpStr] =  m_iDutySeq;
                //m_iDutySeq++;
                m_lstSendQ.push_back(insSendingDuty);
        }
        else  //放回去
        {
                //失败任务队列，本身的时序性要求不是很强，所以可以循环使用
                m_lstSendErrorQ.push_back(insSendingDuty);
        }
        
        m_insMutex.unLock();
        return;
}

//如果一个IP因为故障，从分发列表中摘除时，里面残留的任务将会不会消失，这儿函数的目的就是清除之
//这个做成默认不处理(外部),需要是打开
//另一种方法，也可以通过删除生成的dat下的文件来清除这些任务
void CQueueMng::clearDisabledDuty()
{
        m_insMutex.setLock();
        //正常队列中可以不处理，因此大多是被取消的IP肯定是分发失败的，肯定会进入失败队列
        if (m_lstSendErrorQ.size() == 0)
        {
                m_insMutex.unLock();  //解锁
                return;
        }
        //从头部取任务，判断IP_Port是否在配置文件里面
        CSendDuty insSendingDuty;
        list<CSendDuty>::iterator itduty = m_lstSendErrorQ.begin();
        insSendingDuty = *itduty;
        m_lstSendErrorQ.erase(itduty);

        //获得IP_Port的唯一性
        map<string, CSiteInfo> insMapSite;
        map<string, int> insServerList;
        g_insConfigMng.getSiteMap(insMapSite);   
        map<string, CSiteInfo>::iterator it = insMapSite.begin();
        for(; it != insMapSite.end(); it++)
        {
                vector<CServerInfo>::iterator it1 = (it->second.m_vecServerList).begin();
                for(; it1 != (it->second.m_vecServerList).end(); it1++)
                {
                        char sTmpIPPort[128];
                        sprintf(sTmpIPPort, "%s_%d",  (*it1).m_sIP,  (*it1).m_iPort);
                        string sKey =sTmpIPPort; 
                        insServerList[sKey] = 0;  //主要取唯一的服务器列表
                }                 
        }        

        char sTmpPort[64];
        sprintf(sTmpPort, "%d", insSendingDuty.m_iServerPort);
        string sTmpipport = string("") + insSendingDuty.m_sServerIP + "_" + sTmpPort;
        
        //没有发现这个IP存在，则说明这个任务已经不存在了，删除之
        if(insServerList.end() == insServerList.find(sTmpipport)) 
        {
                //清除掉的任务，写normal日志
                g_insLogMng.normal("Duty(%s) is Disabled, delete it.",  insSendingDuty.getPrintInfo().c_str());
                
                //这儿本应该把不存在的失败的IP删掉，但是删掉后，mvErrorQ2SendQ会被执行，导致形成一个恶性的导换，
                //放在mv中每次判断，又浪费；
                //因此干脆不清理，即使多了，这块也不会引起什么严重的性能问题，如果确实需要，可以用tool的ClearErrorIPPort功能
                //m_mapErrorIPPort.erase(sTmpipport);
        }
        else  // 正常，放回去
        {
                //失败任务队列，本身的时序性要求不是很强，所以可以循环使用
                m_lstSendErrorQ.push_back(insSendingDuty);
        }
        
        m_insMutex.unLock();
        return;
}


