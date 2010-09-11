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

        //�����ļ�  û��ʲô����ĸ�ʽ������2�У���һ���ļ�������ţ��ڶ��У���¼���
        string sPathFile =  g_sHomeDir + "/dat/failduty.seq";
        FILE* fp = fopen(sPathFile.c_str(), "r");
        if (NULL == fp)  //�ļ�������
        {
                return 0;
        }

        //��ȡ
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
        //����ֵ
        int iRet = 0;

        //���м��ʱ����Ҫ������
        m_insMutex.setLock();

        //======= 1.0ͳ����Ϣ, �����̴߳��������ͳ��,�����Ƿ���ֻҪ���յ��ͼ���
        g_insLogMng.debug(insRecvingDuty.m_sSiteName, CLogMng::LogMode2, "Register statistics info.");
        m_mapRecvWorkTime[(int)pthread_self()]++;
        if (CProtocolMng::ProtocolUploadFile == insRecvingDuty.m_iProtocolType
                || CProtocolMng::ProtocolPackFile == insRecvingDuty.m_iProtocolType)
        {
                m_iNrOfRecvUploadFile++; //�����upload
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

        //ע��: ����ÿ�ζ����,���������еĶ��У�������κζ����ж����������ͬ����

        //======= 1.1 ������ڷ��͵Ķ���
        g_insLogMng.debug(insRecvingDuty.m_sSiteName, CLogMng::LogMode2, "Begin to check recving queue.");
        map<int, CRecvingDuty>::iterator it1 = m_mapRecvingQ.begin();
        for (; it1 != m_mapRecvingQ.end(); it1++)
        {
                //���PID���Լ�����ֱ��continue������һ���ж�
                if(it1->first == (int)pthread_self())
                {
                        continue;
                }
                
                //�������ڽ��յ�����
                if ((it1->second).m_iState != CRecvingDuty::Normal)
                {
                        continue;
                }

                //��2����������,�����Ƿ����
                if (0 != strcmp(insRecvingDuty.m_sSiteName, (it1->second).m_sSiteName))
                {
                        continue;
                }
                if (0 != strcmp(insRecvingDuty.m_sDestPathFile, (it1->second).m_sDestPathFile))
                {
                        continue;
                }

                //��������ߵ�����ط�,˵����������ͬ������,ͨ��ʱ���������˭
                if (0 > strcmp(insRecvingDuty.m_sSendedTime, (it1->second).m_sSendedTime))
                {
                         g_insLogMng.normal("RecvingQ: find PID=%u has the same duty(%s, %s),I am older(%s < %s), give up me.",
                                        (unsigned int)it1->first,
                                        insRecvingDuty.m_sSiteName,
                                        insRecvingDuty.m_sDestPathFile,
                                        insRecvingDuty.m_sSendedTime,
                                        (it1->second).m_sSendedTime);
                        iRet = -1;  //�����µ�������Ϊ�н��µ������ڶ�����
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
                        iRet = 1;  //ֹͣ�ϵ�������Ϊ��������ǽ��µ�
                        break; //��Ҫbreak����Ϊֻ���ܴ�����һ���߳�������һ������
                }

        }
        if (-1 == iRet)
        {
                insRecvingDuty.m_iState = CRecvingDuty::Giveup;
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //����
                return iRet;
        }
        else if ( 1 == iRet)
        {
                //��Ҫ���������Ϣ,������� 1
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //����
                return iRet;
        } // else iRet = 0

        //��ת���Ļ���ֻ�н��ն���  iRet=0
        if (0 == g_insConfigMng.m_tComCfg.m_iDistributeFlag)
        {
                //��Ҫ���������Ϣ,�������
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //����
                return iRet;
        }

        //======= 1.2���д����Ͷ��еĲ��ң� �����1.1��û���ҵ�
#if 0  //Ϊ�����ܣ����в��Һͺϲ���ȡ������
        g_insLogMng.debug(insRecvingDuty.m_sSiteName, CLogMng::LogMode2, "Begin to check send queue.");

        string sTmpStr;
        CSendDuty *pTmpDuty;
        
        //�����վ���ÿ������������
        vector <CServerInfo> vecServerInfo;
        g_insConfigMng.getSiteServer(vecServerInfo, insRecvingDuty.m_sSiteName);
        vector<CServerInfo>::iterator it2 =  vecServerInfo.begin();
        for (; it2 != vecServerInfo.end(); it2++)
        {
                char sTmpPort[32];
                sprintf(sTmpPort, "%d", (*it2).m_iPort);
                sTmpStr = string("") + insRecvingDuty.m_sSiteName + insRecvingDuty.m_sDestPathFile + (*it2).m_sIP + "_" + sTmpPort;
                //===== ���һ���Ƿ������ͬ������=====
                map<string, int>::iterator it21 = m_mapSendQIndex.find(sTmpStr);
                if ( m_mapSendQIndex.end() != it21)
                {
                        //���ɾ��,�Ѿ����ϵ�����
                        //�������һ��IP�н��µ�����,����Ϊ���������������,��ȡ����
                        if (2 == iRet)
                        {
                                m_mapSendQ.erase(it21->second); //��������ɾ��
                                m_mapSendQIndex.erase(it21);  //����iteratorɾ��
                                continue;
                        }
                        pTmpDuty = &(m_mapSendQ[it21->second]);

                        //�Ƚ�ʱ��
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
                        else  //��Ҫȡ���������е�����
                        {
                                g_insLogMng.normal("SendQ: find the same duty(%s, %s),I am newer(%s < %s), use me.",
                                        insRecvingDuty.m_sSiteName,
                                        insRecvingDuty.m_sDestPathFile,
                                        insRecvingDuty.m_sSendedTime,
                                        pTmpDuty->m_sSendedTime);

                                m_mapSendQ.erase(it21->second); //��������ɾ��
                                m_mapSendQIndex.erase(it21);  //����iteratorɾ��

                                iRet = 2;  //��ֵΪ2,��ʾ�µ�������Ҫ���,��Ҫɾ�������е�������
                        }
                }
        }
        if (-1 == iRet)
        {
                 insRecvingDuty.m_iState = CRecvingDuty::Giveup;
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //����
                return iRet;
        }
        else if ( 2 == iRet)
        {
                //��Ҫ���������Ϣ,������� 1
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //����
                return iRet;
        } // else iRet = 0
#endif

        //====== 1.3���ҷ��Ͷ��� �����1.1��1.2�ж�û���ҵ���������1.2��ֻ��ȡ���˲�������
        g_insLogMng.debug(insRecvingDuty.m_sSiteName, CLogMng::LogMode2, "Begin to check sending queue.");

        map<int, CSendDuty>::iterator it3 = m_mapSendingQ.begin();
        for (; it3 != m_mapSendingQ.end(); it3++)
        {
                //���PID���Լ�����ֱ��continue������һ���ж�
                if(it3->first == (int)pthread_self())
                {
                        continue;
                }
                
                if ((it3->second).m_iState != CSendDuty::Normal)
                {
                        continue;
                }

                //��2����������,�����Ƿ����
                if (0 != strcmp(insRecvingDuty.m_sSiteName, (it3->second).m_sSiteName))
                {
                        continue;
                }
                if (0 != strcmp(insRecvingDuty.m_sDestPathFile, (it3->second).m_sDestPathFile))
                {
                        continue;
                }

                //�Ƚ�ʱ��
                if (0 > strcmp(insRecvingDuty.m_sSendedTime, (it3->second).m_sSendedTime))
                {
                         g_insLogMng.normal("SendingQ: find PID=%u have the same duty(%s, %s),I am older(%s < %s), give up me.",
                                        (unsigned int)it3->first,
                                        insRecvingDuty.m_sSiteName,
                                        insRecvingDuty.m_sDestPathFile,
                                        insRecvingDuty.m_sSendedTime,
                                        (it3->second).m_sSendedTime);
                        iRet = -1;  //�����µ�������Ϊ�н��µ������ڶ�����
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
                        iRet = 3;  //ֹͣ�ϵ�������Ϊ��������ǽ��µ�
                        // not break; ��Ϊ����������ܴ��ڶ�������߳���, ������Ҫ�����һ��
                }
        }
        if (-1 == iRet)
        {
                 insRecvingDuty.m_iState = CRecvingDuty::Giveup;
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //����
                return iRet;
        }
        else if ( 3 == iRet)
        {
                //��Ҫ���������Ϣ,������� 1
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //����
                return iRet;
        } // else iRet = 0

#if 0  //���ͳɹ����У���Щ���ߣ���ʱȥ�� 2007-06-13
        // 1.4��������ɹ����Ͷ��� �����1.1��1.2��1.3�ж�û���ҵ���������1.2��1.3��ֻ��ȡ���˲�������
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
                         iRet = -1;  //�����µ�������Ϊ�н��µ������ڶ�����
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
                m_insMutex.unLock();  //����
                return iRet;
        }
        else if ( 4 == iRet)
        {
                //��Ҫ���������Ϣ,������� 1
                m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty;
                m_insMutex.unLock();  //����
                return iRet;
        } // else iRet = 0
#endif 
        
        g_insLogMng.debug(insRecvingDuty.m_sSiteName, CLogMng::LogMode2, "Not find the same duty, deal with me(%s).",
                        insRecvingDuty.getPrintInfo().c_str());
        m_mapRecvingQ[(int)pthread_self()]= insRecvingDuty; //���϶���
        m_insMutex.unLock();  //����
        return 0;
}

int CQueueMng::judgeNormal(int pID, QueueType eQueueType)
{
        //��ǰֻ��Ҫʵ�ֽ��ն��кͷ��Ͷ���
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
        //��ǰֻ��Ҫʵ�ֽ��ն��кͷ��Ͷ���
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

        //���м��ʱ����Ҫ������
        m_insMutex.setLock();        

        //һ�����м�飬�϶����㹻��֤������ŵ���ȷ��,���������Զ��Ϊ�յ����: ��mapΪ��ʱ�������е������������
        //if (0 == m_mapSendQ.size())
        //{                        
        //        m_iDutySeq = 0;
        //}

        //ͳ����Ϣ,ͳ�ƽ��յ��ļ���С��ֻ��Ҫͳ�ƽ��յ����ļ�(��Ϊ�����ĵط���������ļ���С����0)
        m_lTotalSizeOfRecvFile = m_lTotalSizeOfRecvFile + iRecvFileSize;        

        //��Ҫ��ӷ�������Ϣ
        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode2, "Begin to add this msg to send queue.");
        if (AddServerInfo == ePutSendType)
        {
                //���ڽ��յ��������
                int pID = (int)pthread_self();
                if (m_mapRecvingQ[pID].m_iState == CRecvingDuty::Normal)
                {
                        m_mapRecvingQ[pID].m_iState = CRecvingDuty::Over;
                }
                else  //�����Ѿ���ȡ����
                {
                        g_insLogMng.normal( "This duty(%s) has been canceled, give up it.", insSendDuty.getPrintInfo().c_str());
                        m_insMutex.unLock();  //����
                        return 0;
                }

                //�������Ҫת������ֱ�ӽ���
                if (0 == g_insConfigMng.m_tComCfg.m_iDistributeFlag)
                {
                        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode3, "Need not forward, this duty is over.");
                        m_insMutex.unLock();  //����
                        return 0;
                }

                //���ݷ�������Ϣ�����μ������
                vector<CServerInfo> insSiteServer;
                g_insConfigMng.getSiteServer(insSiteServer, insSendDuty.m_sSiteName);
                for (vector<CServerInfo>::iterator it = insSiteServer.begin(); it != insSiteServer.end(); it++)
                {
                        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode2, "Config server info(%s, %d)",
                                (*it).m_sIP, (*it).m_iPort);
                        insSendDuty.m_iNrOfFail = 0;
                        strcpy(insSendDuty.m_sServerIP, (*it).m_sIP);
                        insSendDuty.m_iServerPort = (*it).m_iPort;
                        
                        //�������
                        char sTmpPort[32];
                        sprintf(sTmpPort, "%d", (*it).m_iPort);
                        string sTmpipp = string("") + (*it).m_sIP + "_" + sTmpPort;
                        
                        //�ж�һ�£�Ӧ�ü����Ǹ�����
                        unsigned int iFailTimes = 0;
                        map<string, unsigned int>::iterator itfind = m_mapErrorIPPort.find(sTmpipp);
                        if (itfind != m_mapErrorIPPort.end())  //����������
                        {
                                iFailTimes = itfind->second;  //����
                        }
                        if (iFailTimes < MaxTimesToErrorIPList)  //����������
                        {
                                if (0 != g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty)
                                {
                                        //�����Ƿ���
                                        if (m_lstSendQ.size() >= (unsigned int)g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty)
                                        {
                                                g_insLogMng.error("SendQ: SendQ is full(%d),put this duty to failduty file.",
                                                        g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty);
                                                putFailDuty2File(insSendDuty); //��ֹ����ͻ�����治���ٴμ���ͬ����
                                                continue;  //����������һ��IP
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
                        else  //ʧ�ܹ���IP��Port
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
                                        putFailDuty2File(insSendDuty); //��ֹ����ͻ�����治���ٴμ���ͬ����
                                        continue;  // ������һ��
                                }
                                m_lstSendErrorQ.push_back(insSendDuty);  //ʧ������ֱ�Ӽӵ�����
                                g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode2, "Add Duty(%s) to Error Q ok.", 
                                        insSendDuty.getPrintInfo().c_str());
                        }
                }
        }
        else if (Direct == ePutSendType)
        {
                //�������Ҫת������ֱ�ӽ���
                if (0 == g_insConfigMng.m_tComCfg.m_iDistributeFlag)
                {
                        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode3, "Need not forward, can't add to send queue.");
                        m_insMutex.unLock();  //����
                        return 0;
                } 
                                
                char sTmpPort[32];
                sprintf(sTmpPort, "%d", insSendDuty.m_iServerPort);
                //�������
                string sTmpipp = string("") + insSendDuty.m_sServerIP + "_" + sTmpPort;          
                if (insSendDuty.m_iNrOfFail > 0)  //��һ�μ����ʱ����0�����ܼ���
                {
                        m_mapErrorIPPort[sTmpipp]++;  //���Ӽ�¼
                        //д��normal��־������дdebug,  ǡ���ǵ�MaxTimesToErrorIPList��ʱ                 
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

                        //������ʧ�ܴ�������ֱ�Ӽ���ʧ���ļ�
                        if (insSendDuty.m_iNrOfFail >= g_insConfigMng.m_tComCfg.m_iMaxNrOfRetry)
                        {
                                 g_insLogMng.error("DirectSendQ: the duty(%s) NrOfFail >= Max(%d), so write to file.",
                                        insSendDuty.getPrintInfo().c_str(), g_insConfigMng.m_tComCfg.m_iMaxNrOfRetry);
                                putFailDuty2File(insSendDuty); //��ֹ����ͻ�����治���ٴμ���ͬ����
                                m_insMutex.unLock();  //����
                                return  -1;
                        }
                        
                }
                
                //�ж�һ�£�Ӧ�ü����Ǹ�����
                unsigned int iFailTimes = 0;
                map<string, unsigned int>::iterator itfind = m_mapErrorIPPort.find(sTmpipp);
                if (itfind != m_mapErrorIPPort.end())  //����������
                {
                        iFailTimes = itfind->second;  //����
                }
                if (iFailTimes < MaxTimesToErrorIPList)                
                {
                        //ֱ���жϼ���
                        if (0 != g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty)
                        {
                               if (m_lstSendQ.size() >= (unsigned int)g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty)
                               {
                                       g_insLogMng.error("SendQ: SendQ is full(%d),put this duty to failduty file..",
                                                g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty);
                                        putFailDuty2File(insSendDuty); //��ֹ����ͻ�����治���ٴμ���ͬ����
                                        m_insMutex.unLock();  //����
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
                else  //���������MaxTimesToErrorIPList��ʧ�ܣ������ʧ�ܶ���
                {      
                        //ʧ�ܶ������ڲ����ںϲ����������ǿ���޶����ֵ
                        unsigned int iMaxOfErrorQueue = g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty;
                        if (0 == iMaxOfErrorQueue)
                        {
                                iMaxOfErrorQueue = DefaultMaxOfErrorQueue;
                        }
                        if (m_lstSendErrorQ.size() >= iMaxOfErrorQueue)
                        {
                               g_insLogMng.error("ErrorSendQ: SendQ is full(%d),put this duty to failduty file..",
                                        g_insConfigMng.m_tComCfg.m_iMaxNrOfQueueDuty);
                                putFailDuty2File(insSendDuty); //��ֹ����ͻ�����治���ٴμ���ͬ����
                                m_insMutex.unLock();  //����
                                return  -1;
                        }
                        m_lstSendErrorQ.push_back(insSendDuty);
                        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode2, "Direct add Duty(%s) to Error Q ok", 
                                 insSendDuty.getPrintInfo().c_str());
                }
        }

        g_insLogMng.debug(insSendDuty.m_sSiteName, CLogMng::LogMode2, "End to add this msg to send queue.");
        m_insMutex.unLock();  //����
        return 0;
}

int CQueueMng::getDutyFSendQ(int iType, CSendDuty& insSendingDuty)  // 2007-05-21 add itype
{
        //���м��ʱ����Ҫ������
        m_insMutex.setLock();
        int iLocalType = iType;
        if (0 == iLocalType)
        {
                if (m_lstSendQ.size() == 0) //�������в���������
                {
                        if(m_lstSendErrorQ.size() == 0) //ʧ���������
                        {
                                m_insMutex.unLock();  //����
                                return -1;
                        }
                        else
                        {
                                iLocalType = 1; //��ȡ�����������,ת����ȡʧ���������
                        }
                }  //else ά������������          
        }
        else if (1 == iLocalType)
        {
                if (m_lstSendErrorQ.size() == 0) //ʧ���������
                {
                        if(m_lstSendQ.size() == 0) //�����������
                        {
                                m_insMutex.unLock();  //����
                                return -1;
                        }
                        else
                        {
                                iLocalType = 0; //��ȡʧ���������,ת����ȡ�����������
                        }
                }  //else ά����ʧ������
        }
        else
        {
                 m_insMutex.unLock();  //����
                return -1;
        }

        // 2007-05-21 add itype
        //����Ϊ��,����-1
        if (0 == iLocalType)  //��������
        {
                //��ͷ��ȡ����,�������ڷ��Ͷ���
                list<CSendDuty>::iterator it = m_lstSendQ.begin();
                insSendingDuty = *it;
                m_lstSendQ.erase(it);

                //���������
                char sTmpPort[32];
                sprintf(sTmpPort, "%d", insSendingDuty.m_iServerPort);
                //string sTmpStr = string("") + insSendingDuty.m_sSiteName + insSendingDuty.m_sDestPathFile + insSendingDuty.m_sServerIP + "_" + sTmpPort;
                //m_mapSendQIndex.erase(sTmpStr);
                //���������
                //m_mapSendQ.erase(it);                
                //��������
                string sTmpipp = string(insSendingDuty.m_sServerIP) + "_" + sTmpPort;
                unsigned int iFailTimes = 0;
                map<string, unsigned int>::iterator itfind = m_mapErrorIPPort.find(sTmpipp);
                if (itfind != m_mapErrorIPPort.end())  //����������
                {
                        iFailTimes = itfind->second;  //����
                }
                if (iFailTimes >= MaxTimesToErrorIPList)
                {
                        //���뵽ʧ�ܶ�����
                        m_lstSendErrorQ.push_back(insSendingDuty);
                        m_insMutex.unLock();  //����
                        return -1;
                } 
        }
        else if (1 == iLocalType)  //��������
        {
                //��ͷ��ȡ����,�������ڷ��Ͷ���
                list<CSendDuty>::iterator it = m_lstSendErrorQ.begin();
                insSendingDuty = *it;
                m_lstSendErrorQ.erase(it);
        }
        else
        {
                m_insMutex.unLock();  //����
                return -1;
        }

        //ͳ����Ϣ, �����̴߳��������ͳ��,�����Ƿ���ֻҪժ�¾ͼ���
        m_mapSendWorkTime[(int)pthread_self()]++;
        
        m_mapSendingQ[(int)pthread_self()]= insSendingDuty;
        
        if (CProtocolMng::ProtocolDelFile == insSendingDuty.m_iProtocolType) 
        {
                m_iNrOfSendDelFile++; 
        }
        else if (CProtocolMng::ProtocolUploadFile == insSendingDuty.m_iProtocolType
                || CProtocolMng::ProtocolPackFile == insSendingDuty.m_iProtocolType) 
        {                
                m_iNrOfSendUploadFile++;  //����ϴ��ȣ�������upload
        }
        else if (CProtocolMng::ProtocolMvFile == insSendingDuty.m_iProtocolType) 
        {
                m_iNrOfSendMvFile++;
        }
        else if (CProtocolMng::ProtocolCpFile == insSendingDuty.m_iProtocolType) 
        {
                m_iNrOfSendCpFile++;
        }

        m_insMutex.unLock();  //����
        return 0;
}

int CQueueMng::putDuty2LastSucQ(CSendDuty& insLastSucDuty)
{
#if 0
        //�Ƿ���Ҫ��������ɹ�����
        if (0 == g_insConfigMng.m_tComCfg.m_iMaxNrOfSaveLastSucDuty)
        {
                return 0;
        }

        //���м��ʱ����Ҫ������
        m_insMutex.setLock();

        //����㹻��ʱ����ն������±��  8 ��9���������ܻᵼ�����⣬���ǲ�Ӱ����,��������ܸ��ӵ��㷨��
        if (m_iLastSucSeq >= 99999999)
        {
                m_iLastSucSeq = 0;
                m_mapLastSucIndexQ.clear();
                m_mapLastSucQ.clear();
        }

        //�������
        if (m_mapLastSucQ.size() >= (unsigned int)g_insConfigMng.m_tComCfg.m_iMaxNrOfSaveLastSucDuty)
        {
                map<int, string>::iterator it = m_mapLastSucIndexQ.begin();

                //������ϵĳɹ����е�����
                m_mapLastSucQ.erase(it->second);

                //������������е�����
                m_mapLastSucIndexQ.erase(it);
        }

        //������к���������
        string sTmpStr = string("") + insLastSucDuty.m_sSiteName + insLastSucDuty.m_sDestPathFile;
        m_mapLastSucQ[sTmpStr] = insLastSucDuty.m_sSendedTime;
        m_mapLastSucIndexQ[m_iLastSucSeq++] = sTmpStr;

        m_insMutex.unLock();  //����
#endif

        return 0;

}

//�����ļ�����Ϊ�ܱ�֤�������ֻ����������²ŵ���
void CQueueMng::putFailDuty2File(CSendDuty& insFailDuty)
{
        if (0 == g_insConfigMng.m_tComCfg.m_iIfFailLogFlag)
        {
                return;
        }

        //Ϊ�˷�ֹ��ɾ���ļ���ɾ���ļ���ʧ����Ϣ��д��ʧ���ļ�������������ʱ���ֹ���ɾ��
        if (CProtocolMng::ProtocolDelFile == insFailDuty.m_iProtocolType)
        {
                return;
        }

        if (m_iRecordSeq >= MaxSeqOfFailRecord)  //��¼�ﵽ����
        {
                m_iFileSeq++;  //�ļ����Ƽ�1
                m_iRecordSeq = 0;
        }

        //�ļ���ѭ��ʹ��
        if (MaxSeqOfFailFileName < m_iFileSeq)
        {
                m_iFileSeq = 0;
        }

        //Ҫд����ļ�����
        char sTmp[LenOfPathFileName];
        sprintf(sTmp, "failduty.%04d", m_iFileSeq);
        string sPathFile = g_sHomeDir + "/dat/" + sTmp;

        //д���ļ�
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

        //��¼��++
        m_iRecordSeq++;

        //�����ļ�  û��ʲô����ĸ�ʽ������2�У���һ���ļ�������ţ��ڶ��У���¼���
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
        //Ҫд����ļ�����
        char sTmp[LenOfPathFileName];
        sprintf(sTmp, "failduty.%04d", iFileSeq);
        string sPathFile1 = g_sHomeDir + "/dat/" + sTmp;
        string sPathFile2 = g_sHomeDir + "/dat/failduty.do";

        //��ֹ��д�����ļ�mv���ٲ���
        if (0 != CComFun::mvFile(sPathFile1, sPathFile2))
        {
                g_insLogMng.debug("protocol",CLogMng::LogMode3, "%s is not exsit.", sPathFile1.c_str());
                return;
        }

        //д���ļ�
        FILE *fp = fopen(sPathFile2.c_str(), "r");
        if (NULL == fp)
        {
                return;  //�ļ�������
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
                //��Ҫ�������жϼ�¼��ʽ����ȷ�ԣ���ֹ�ļ���λ
                if (insFailDuty.m_iProtocolType != CProtocolMng::ProtocolUploadFile
                        && insFailDuty.m_iProtocolType != CProtocolMng::ProtocolDelFile
                        && insFailDuty.m_iProtocolType != CProtocolMng::ProtocolMvFile
                        && insFailDuty.m_iProtocolType != CProtocolMng::ProtocolCpFile
                        && insFailDuty.m_iProtocolType != CProtocolMng::ProtocolPackFile)  // add by kevinliu 2007-01-10
                {
                        g_insLogMng.error("Error failed duty protocoltype(%d)",insFailDuty.m_iProtocolType );
                        continue;
                }

                //���������״̬
                insFailDuty.m_iState = CSendDuty::Normal;
                insFailDuty.m_iNrOfFail = 0;

                //ȡ�����ļ���
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

                //�ж������Ƿ��Ѿ�����,������ʲô����,�ȷ��������˵,�ڷ��͵�ʱ���ټ����Ч��
                struct stat tmpStat;
                if(0 <= stat(sLocalPathFile.c_str(), &tmpStat))
                {
                        //ͬ���ܽ��,ɾ���ļ�������,������,�϶����ϵ�����,������,���Ϳ϶�����������
                        if (tmpStat.st_mtime > insFailDuty.m_iLocalTime)
                        {
                                g_insLogMng.normal("WhenLoadFile: %s has been modified, give up.",
                                        insFailDuty.m_sLocalPathFile);
                                continue;
                        }
                }

                //��Ҫcheckһ�²ſ��ԣ�������п������ظ�������
                strcpy(insRecvingDuty.m_sSiteName,insFailDuty.m_sSiteName);
                insRecvingDuty.m_iState = CRecvingDuty::Normal;
                insRecvingDuty.m_iProtocolType = insFailDuty.m_iProtocolType;
                strcpy(insRecvingDuty.m_sDestPathFile, insFailDuty.m_sLocalPathFile);
                strcpy(insRecvingDuty.m_sSendedTime, insFailDuty.m_sSendedTime);
                if ( 0 > check(insRecvingDuty))
                {
                        continue;
                }
                //�������
                putDuty2SendQ(insFailDuty, Direct);
        }

        fclose(fp);
        CComFun::delFile(sPathFile2);  //��do�ļ�һ��ɾ��
        
        return;
}

void CQueueMng::printCurInfo()
{
        //�Լ�дһ���������ļ�
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

        //�����IP����
        m_insMutex.setLock();
        map<string, unsigned int> tmpErorrIPPort = m_mapErrorIPPort;
        m_insMutex.unLock();     
        
        map<string, unsigned int>::iterator itip = tmpErorrIPPort.begin();
        for(;itip != tmpErorrIPPort.end();itip++)
        {
               fprintf(fp, "ErrorIPPort=%s:%u\n", itip->first.c_str(), itip->second);
        }

        //���е���ϸ��Ϣ
        if (0 == access((myFile + ".if").c_str(),  F_OK))  //�������queue.prt.if�ļ������ӡ��ϸ��Ϣ
        {
                //���Ƴ�һ������
                m_insMutex.setLock();
                list<CSendDuty> tmpOkQueue = m_lstSendQ;
                list<CSendDuty> tmplstQueue = m_lstSendErrorQ;
                m_insMutex.unLock(); 

                //��ӡ�����е����ݵ��ļ���ȥ
                //��������
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
                //ʧ�ܶ���
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
        //�Լ�дһ���������ļ�
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
        sInfo = string("")  + sTmp;  //��ʼ��Ϣ

        //�����IP����
        m_insMutex.setLock();
        map<string, unsigned int> tmpErorrIPPort = m_mapErrorIPPort;
        m_insMutex.unLock();     
        
        map<string, unsigned int>::iterator itip = tmpErorrIPPort.begin();
        for(;itip != tmpErorrIPPort.end();itip++)
        {
               sprintf(sTmp, "ErrorIPPort=%s:%u\n", itip->first.c_str(), itip->second);
               sInfo = sInfo + sTmp; //����
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

        //�����߳�
        sStatInfo = sStatInfo + sTmp + " Recv Thread:\n";
        for(map<int,int>::iterator it = m_mapRecvWorkTime.begin();
                it != m_mapRecvWorkTime.end(); it++)
        {
                sprintf(sTmp, " WorkTimesOfThread(%d) = %d\n", it->first, it->second);
                sStatInfo = sStatInfo + sTmp;
                 it->second = 0;//���
        }

        //�����߳�
        sStatInfo = sStatInfo + " Send Thread:\n";
        for(map<int,int>::iterator it = m_mapSendWorkTime.begin();
                it != m_mapSendWorkTime.end(); it++)
        {
                sprintf(sTmp, " WorkTimesOfThread(%d) = %d\n", it->first, it->second);
                sStatInfo = sStatInfo + sTmp;
                it->second = 0;//���
        }

        //���
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

void CQueueMng::printStatInfoXML() //���뱣֤����printStatInfoִ�У��������ݾͻᱻ������
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

//Ϊʲô������������������״̬�ж�
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

//���д������񣬽�ʧ�ܶ�����,IP�Ѿ��ָ�������ת�Ƶ��ɹ������У���ֹ��Ϊ���������IP
void CQueueMng::mvErrorQ2SendQ()
{
        m_insMutex.setLock();
        if (m_lstSendErrorQ.size() == 0)
        {
                m_insMutex.unLock();  //����
                return;
        }
        //��ͷ��ȡ�����ж�IP_Port�Ƿ��Ѿ����Ƴ���ʧ�ܶ��У�����Ѿ���������ӵ������Ķ�����ȥ
        CSendDuty insSendingDuty;
        list<CSendDuty>::iterator itduty = m_lstSendErrorQ.begin();
        insSendingDuty = *itduty;
        m_lstSendErrorQ.erase(itduty);

        char sTmpPort[64];
        sprintf(sTmpPort, "%d", insSendingDuty.m_iServerPort);
        string sTmpipport = string("") + insSendingDuty.m_sServerIP + "_" + sTmpPort;
        //string sTmpStr = string("") + insSendingDuty.m_sSiteName + insSendingDuty.m_sDestPathFile + sTmpipport;

        //û�з��֣�˵���Ѿ������
        if(m_mapErrorIPPort.end() == m_mapErrorIPPort.find(sTmpipport)) 
        {
                g_insLogMng.debug("all", CLogMng::LogMode1, "Duty(%s) is moved from EQ to Q.",
                        insSendingDuty.getPrintInfo().c_str());

                //Ϊ������ٶȣ������ж��Ƿ������ͬ��������Ϊ��ʹ���ڣ�m_mapSendQIndex����С��m_mapSendQ,ϵͳҲ��������
                //m_mapSendQ[m_iDutySeq] = insSendingDuty;
                //m_mapSendQIndex[sTmpStr] =  m_iDutySeq;
                //m_iDutySeq++;
                m_lstSendQ.push_back(insSendingDuty);
        }
        else  //�Ż�ȥ
        {
                //ʧ��������У������ʱ����Ҫ���Ǻ�ǿ�����Կ���ѭ��ʹ��
                m_lstSendErrorQ.push_back(insSendingDuty);
        }
        
        m_insMutex.unLock();
        return;
}

//���һ��IP��Ϊ���ϣ��ӷַ��б���ժ��ʱ��������������񽫻᲻����ʧ�����������Ŀ�ľ������֮
//�������Ĭ�ϲ�����(�ⲿ),��Ҫ�Ǵ�
//��һ�ַ�����Ҳ����ͨ��ɾ�����ɵ�dat�µ��ļ��������Щ����
void CQueueMng::clearDisabledDuty()
{
        m_insMutex.setLock();
        //���������п��Բ�������˴���Ǳ�ȡ����IP�϶��Ƿַ�ʧ�ܵģ��϶������ʧ�ܶ���
        if (m_lstSendErrorQ.size() == 0)
        {
                m_insMutex.unLock();  //����
                return;
        }
        //��ͷ��ȡ�����ж�IP_Port�Ƿ��������ļ�����
        CSendDuty insSendingDuty;
        list<CSendDuty>::iterator itduty = m_lstSendErrorQ.begin();
        insSendingDuty = *itduty;
        m_lstSendErrorQ.erase(itduty);

        //���IP_Port��Ψһ��
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
                        insServerList[sKey] = 0;  //��ҪȡΨһ�ķ������б�
                }                 
        }        

        char sTmpPort[64];
        sprintf(sTmpPort, "%d", insSendingDuty.m_iServerPort);
        string sTmpipport = string("") + insSendingDuty.m_sServerIP + "_" + sTmpPort;
        
        //û�з������IP���ڣ���˵����������Ѿ��������ˣ�ɾ��֮
        if(insServerList.end() == insServerList.find(sTmpipport)) 
        {
                //�����������дnormal��־
                g_insLogMng.normal("Duty(%s) is Disabled, delete it.",  insSendingDuty.getPrintInfo().c_str());
                
                //�����Ӧ�ðѲ����ڵ�ʧ�ܵ�IPɾ��������ɾ����mvErrorQ2SendQ�ᱻִ�У������γ�һ�����Եĵ�����
                //����mv��ÿ���жϣ����˷ѣ�
                //��˸ɴ಻������ʹ���ˣ����Ҳ��������ʲô���ص��������⣬���ȷʵ��Ҫ��������tool��ClearErrorIPPort����
                //m_mapErrorIPPort.erase(sTmpipport);
        }
        else  // �������Ż�ȥ
        {
                //ʧ��������У������ʱ����Ҫ���Ǻ�ǿ�����Կ���ѭ��ʹ��
                m_lstSendErrorQ.push_back(insSendingDuty);
        }
        
        m_insMutex.unLock();
        return;
}


