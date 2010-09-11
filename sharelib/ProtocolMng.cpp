//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *
//  *    $RCSfile: ProtocolMng.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "ProtocolMng.h"

extern CQueueMng g_insQueueMng;
extern CConfigMng g_insConfigMng;
extern CLogMng g_insLogMng;
extern string g_sHomeDir;
extern CMaintain g_insMaintain;

//  *************************************************************************
//        Э��򵥽���
// ����������Ϣ��Ϊ��λ�ķ���ȷ�ϻ���:
// Ҳ����˵��ÿ����һ����Ϣ�飬�ͱ����յ�����Ӧ�ž�����һ���Ƿ����;
// ��λ����Ϣͷһ����,��Ϣ���ֳɶಿ��
// ͬʱҲ֧��,һ�������Ϣ,һ��
//  *************************************************************************

void CProtocolMng::processRecv(CTCPSocket& insTcpServer)
{
        //��ʼ����һ���µ�����
        g_insLogMng.debug("protocol", CLogMng::LogMode2, "Begin to recv protocol process.");

        //��Ϣ�շ���Ϣ
        int iMsgTimeout = g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout;  //��Ϣ��ʱʱ��
        int iFileTimeout = g_insConfigMng.m_tComCfg.m_iRecvFileTimeout;  //�ļ���ʱʱ��
        ResponseCode eResponseCode = Success;  //��Ӧ��

        //��Ϣͷ��Ϣ
        char pMsgHead[MsgHeadLen + 1];

        //���ջ�����
        char sMsgBuff[MsgBufferSize + 1];

        //������Ϣͷ��Ϣ
        g_insLogMng.debug("protocol", CLogMng::LogMode3, "Begin to recv a msg head info:");
        int iRecvMsgRet;
        iRecvMsgRet = insTcpServer.receiveMsg(pMsgHead, MsgHeadLen, iMsgTimeout);
        if (0 == iRecvMsgRet)
        {
                 g_insLogMng.debug("protocol", CLogMng::LogMode1, "MsgVersion = %d, MsgType = %d.", 
                        pMsgHead[0], pMsgHead[1]);
                 
                //kevinliu add for 2.1�汾 - 2007-04-29
                if (pMsgHead[0] == Version2)
                {
                        processVersion2(insTcpServer, pMsgHead[1]); //����д�������д���
                        return;
                }
                else  if (pMsgHead[0] != Version1)  //����Ĺ���ֻ���� Version1��
                {
                        g_insLogMng.error("Recv an error msg version(%x), discard this msg.",
                               pMsgHead[0]);
                        return;
                }                
                
                //������Ϣ����,���д���
                switch(pMsgHead[1])
                {
                case ProtocolUploadFile:
                case ProtocolPackFile:  // add by kevinliu 2007-01-10
                {
                        //������Ϣ��Ϣ����
                        CFileUploadMsg insFileUploadMsg;
                        g_insLogMng.debug("protocol", CLogMng::LogMode3, "Begin to recv an info head.");
                        iRecvMsgRet = insTcpServer.receiveMsg((char*)(&insFileUploadMsg), sizeof(CFileUploadMsg), iFileTimeout);
                        if (0 != iRecvMsgRet) //�����ļ�ʱ�䣬���ϳ����ʱ��
                        {
                                if (1 == iRecvMsgRet)
                                {
                                        eResponseCode = CancelDuty;
                                        g_insLogMng.normal("Recv FileUploadMsg canceled because recv msgheadinfo error.");
                                }
                                else
                                {
                                        g_insLogMng.error("Recv FileUploadMsg failed because recv msgheadinfo error.");
                                        eResponseCode = HeadFail;
                                }
                                break;
                        }//�����Ӧ����һ�鴦����Ϊ�������ڻ��к�����Ϣ������������ܵ��¶Զ�д���� CONTINUE1>>
                        //<<--Ϊ�˷�ֹ,���͹�������Ϣ���ֶ�û�н�����,���Ҫ��char����Ľ�������
                        insFileUploadMsg.sureValidSrt();

                        g_insLogMng.debug("protocol", CLogMng::LogMode3, "End to recv an info head.");

                        insFileUploadMsg.m_iDataLength = ntohl(insFileUploadMsg.m_iDataLength);  //ת�������ֽ���
                        g_insLogMng.debug(insFileUploadMsg.m_sSiteName, CLogMng::LogMode1, "BEGIN<<%s>>",
                                 insFileUploadMsg.getPrintInfo().c_str());

                        //��Ϣ�峤���Ƿ���Ч
                        if (0 > insFileUploadMsg.m_iDataLength)
                        {
                                g_insLogMng.error("FileUploadMsg invalid because the msg body len error.");
                                eResponseCode = InfoFail;
                                break;
                        }

                        //վ���Ƿ����
                        map<string, CSiteInfo> insMapSite;
                        g_insConfigMng.getSiteMap(insMapSite);
                        if (insMapSite.end() == insMapSite.find(insFileUploadMsg.m_sSiteName))
                        {
                                g_insLogMng.normal("FileUploadMsg invalid because site:%s does not exist.",
                                        insFileUploadMsg.m_sSiteName);
                                eResponseCode = InfoFail;
                                break;
                        }

                        //�������ڽ�������
                        CRecvingDuty insRecvingDuty;
                        insRecvingDuty.m_iState = CRecvingDuty::Normal;
                        insRecvingDuty.m_iProtocolType = pMsgHead[1];
                        strcpy(insRecvingDuty.m_sSiteName, insFileUploadMsg.m_sSiteName);
                        strcpy(insRecvingDuty.m_sDestPathFile, insFileUploadMsg.m_sDestPathFile);
                        strcpy(insRecvingDuty.m_sSendedTime, insFileUploadMsg.m_sSendedTime);

                        //��������Ϣ�Ƿ���Ҫ���������Ҫ��Ӵ���
                        //���Ѿ���check����ӵ������У�����Ҫ����ֱ���˳�����,��־��check����д
                        if (0 > g_insQueueMng.check(insRecvingDuty))
                        {
                                eResponseCode = CancelDuty; //�������Ѿ�д����־��������д
                                //break; // --PAUSE--Ϊ�˲�ʹ�Դ�дʧ�ܵ����ط�,�����ж�����,����󲻴���ķ�ʽ
                        }

                        //��ʼ���գ�׼�����ɴ���������(1.1&2.1��һ���֣�������Ϣ)
                        CSendDuty insSendDuty;
                        insSendDuty.m_iState = CSendDuty::Normal;
                        insSendDuty.m_iProtocolType = pMsgHead[1];
                        strcpy(insSendDuty.m_sSiteName, insFileUploadMsg.m_sSiteName);
                        strcpy(insSendDuty.m_sDestPathFile, insFileUploadMsg.m_sDestPathFile);
                        strcpy(insSendDuty.m_sSendedTime, insFileUploadMsg.m_sSendedTime);

                         //�ж�������������
                        string sSaveLocalFileName = "";  //��Ҫ��������¼run��־��
                        if (CFileUploadMsg::FileName == insFileUploadMsg.m_cDataType)  //�ļ���
                        {
                                //�����ļ�������󳤶�
                                if (insFileUploadMsg.m_iDataLength >= LenOfPathFileName)
                                {
                                         g_insLogMng.error("FileUploadMsg(%s) invalid, Filename too long(%d > %d).",
                                                        insFileUploadMsg.getPrintInfo().c_str(),
                                                        insFileUploadMsg.m_iDataLength ,
                                                        LenOfPathFileName);
                                        eResponseCode = MsgError;
                                        break;
                                }

                                //<<CONTINUE1 ÿ����һ��Ҫȷ����Ϣ����֤��Ϣ������,��ʱ�����öϵ������ķ�ʽ
                                //����λ��������´ν���֮ǰ������û�н�����Ϣ�Ĳ������������Է�ֹ�Զ�д����
                                if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                                {
                                        if (0 != sendResponse(insTcpServer, Success))
                                        {
                                                g_insLogMng.error("Send FileUploadMsg(%s) data block response msg failed.",
                                                        insFileUploadMsg.getPrintInfo().c_str());
                                                eResponseCode = ServicePause;
                                                break;
                                        }
                                }

                                //������Ϣʧ��
                                g_insLogMng.debug("protocol", CLogMng::LogMode3, "Begin to recv msg body...");
                                iRecvMsgRet = insTcpServer.receiveMsg(sMsgBuff, insFileUploadMsg.m_iDataLength, iFileTimeout);
                                if (0 != iRecvMsgRet)
                                {
                                        if (1 == iRecvMsgRet)
                                        {
                                                eResponseCode = CancelDuty;
                                                g_insLogMng.normal("Recv FileUploadMsg(%s) data block canceled.",
                                                        insFileUploadMsg.getPrintInfo().c_str());
                                        }
                                        else
                                        {
                                                eResponseCode = DataFail;
                                                g_insLogMng.error("Recv FileUploadMsg(%s) data block failed.",
                                                        insFileUploadMsg.getPrintInfo().c_str());
                                        }
                                        break;
                                } //CONTINUE2>>

                                // --PAUSE--Ϊ�˲�ʹ�Դ�дʧ�ܵ����ط�,�����ж�����,����󲻴���ķ�ʽ
                                if (eResponseCode == CancelDuty)
                                {
                                        break;
                                }

                                //���Ͻ�������,��Ϊ��memset̫�˷�������
                                sMsgBuff[insFileUploadMsg.m_iDataLength] = '\0';
                                CComFun::trimString(sMsgBuff, sMsgBuff, 0);  //���˿ո�

                                //׼�����ɴ���������(1.2�ڶ����֣��ļ���Ϣ)
                                time((time_t*)&insSendDuty.m_iLocalTime); //��ǰ����
                                strcpy(insSendDuty.m_sLocalPathFile, sMsgBuff);
                                 g_insLogMng.debug(insFileUploadMsg.m_sSiteName, CLogMng::LogMode1, "%s",
                                        insSendDuty.m_sLocalPathFile);
                                 sSaveLocalFileName = insSendDuty.m_sLocalPathFile;

                                //���ɴ��������񲢼������(1.3�������֣���������Ϣ)
                                if (1 == CComFun::isDirectory(insSendDuty.m_sLocalPathFile))
                                {
                                        //Ŀ¼����ת��,���ٻ�Ӧʧ��
                                        g_insLogMng.normal("The duty(%s)'s file is a directory, give up forwarding it.",
                                                 insSendDuty.getPrintInfo().c_str());

                                }
                                else  // �������ļ�����,�������
                                {
                                        //�ж�һ���ļ��Ƿ񳬹��������
                                        struct stat stCheckFStat;
                                        stat(insSendDuty.m_sLocalPathFile, &stCheckFStat); //���жϽ���ˣ���Ϊǰ����ʵ�Ѿ��жϹ���
                                        if (0 != g_insConfigMng.m_tComCfg.m_iMaxLimitOfUploadFile) //Ϊ0������
                                        {
                                                if (stCheckFStat.st_size >= g_insConfigMng.m_tComCfg.m_iMaxLimitOfUploadFile)
                                                {
                                                        eResponseCode = DataFail;
                                                        g_insLogMng.error("Recv FileUploadMsg(%s) blocked, File is too large(%d over %d bytes).",
                                                                insFileUploadMsg.getPrintInfo().c_str(), stCheckFStat.st_size, g_insConfigMng.m_tComCfg.m_iMaxLimitOfUploadFile);
                                                        break;
                                                }
                                        }
                                
                                        //����Ǵ������Ļ�,�����һ��У��,ֻ������2������ kevinliu add 2007-01-10
                                        if (pMsgHead[1] == ProtocolPackFile)
                                        {
                                                Fun_FileType ut = CComFun::getFileType(insSendDuty.m_sLocalPathFile);
                                                if (ut != FILE_TARGZ && ut != FILE_ZIP)
                                                {
                                                        eResponseCode = DataFail; //�������Ѿ�д����־��������д
                                                         g_insLogMng.normal("The duty(%s)'s file is not tar.gz and not zip file, give up forwarding it.",
                                                                 insSendDuty.getPrintInfo().c_str());
                                                        break; 
                                                }
                                        }  // add end                                
                                        if ( 0 != g_insQueueMng.putDuty2SendQ(insSendDuty, CQueueMng::AddServerInfo))
                                        {
                                                eResponseCode = ServicePause; //�������Ѿ�д����־��������д
                                                break;
                                        }
                                }
                                //CONTINUE2<< �ĳɹ���Ϣ,�������Ǹ���Ϣ

                        }
                        else if (CFileUploadMsg::FileData == insFileUploadMsg.m_cDataType)  //������
                        {
                                //�ļ��ĸ�Ŀ¼
                                string sRootPathFile = g_insConfigMng.getSiteRootDir(insFileUploadMsg.m_sSiteName);
                                if("" == sRootPathFile )
                                {
                                        g_insLogMng.error("Analyse FileUploadMsg(%s) failed because getSiteRootDir not find rootdir.",
                                                        insFileUploadMsg.getPrintInfo().c_str());
                                        eResponseCode = InfoFail;
                                        break;
                                }

                                //�ļ����ݴ�Ŀ¼
                                char sTmp[LenOfPathFileName];
                                sprintf(sTmp, "/octopusd_save/tmp_%u", (unsigned int)pthread_self());
                                string sTmpPathFile = CComFun::analysePathFile(sRootPathFile, sTmp, 1);
                                 if ("" == sTmpPathFile)
                                {
                                        g_insLogMng.error("Analyse FileUploadMsg(%s) failed because sTmpPathFile(%s, %s) is dir or mkdir error",
                                                        insFileUploadMsg.getPrintInfo().c_str(), sRootPathFile.c_str(), sTmp);
                                        eResponseCode = InfoFail;
                                        break;
                                }

                                //�ļ���ʵ��Ŀ¼
                                //���ɱ�����ļ����ƺ�·��(ǰ���Ѿ��жϹ��ˣ�վ�㵽����Ѿ�����)
                                string sRealPathFile = CComFun::analysePathFile(sRootPathFile, insFileUploadMsg.m_sDestPathFile, 1);
                                if ("" == sRealPathFile)
                                {
                                        g_insLogMng.error("Analyse FileUploadMsg(%s) failed because desFile(%s,%s) is dir or mkdir error",
                                                        insFileUploadMsg.getPrintInfo().c_str(), sRootPathFile.c_str(), insFileUploadMsg.m_sDestPathFile);
                                        eResponseCode = InfoFail;
                                        break;
                                }

                                //д��ʱ�ļ�
                                FILE *fp = fopen(sTmpPathFile.c_str(), "w");
                                if (NULL == fp)
                                {
                                        g_insLogMng.error("The duty(%s) failed because open(%s) failed as w.",
                                                        insFileUploadMsg.getPrintInfo().c_str(), sTmpPathFile.c_str());
                                        eResponseCode = InfoFail;
                                        break;
                                }
                                g_insLogMng.debug(insFileUploadMsg.m_sSiteName, CLogMng::LogMode1, "Tmp:%s:Real:%s",
                                        sTmpPathFile.c_str(), sRealPathFile.c_str());

                                //д���ļ���ʱ�������ı�����Ϣ
                                char sWriteFileBuff[WriteToFileSize + 1];
                                unsigned int iWriteFileBuffLen = 0;  //�Ѿ�д��writefilebuff�е���Ϣ����

                                //��Ϣ�������ı���
                                int iLeaveMsgLen = insFileUploadMsg.m_iDataLength;
                                //�ж�һ���ļ��Ƿ񳬹��������
                                if (0 != g_insConfigMng.m_tComCfg.m_iMaxLimitOfUploadFile) //Ϊ0������
                                {
                                        if (iLeaveMsgLen >= g_insConfigMng.m_tComCfg.m_iMaxLimitOfUploadFile)
                                        {
                                                eResponseCode = DataFail;
                                                g_insLogMng.error("Recv FileUploadMsg(%s) blocked, File is too large(%s over % bytes).",
                                                        insFileUploadMsg.getPrintInfo().c_str(), iLeaveMsgLen, g_insConfigMng.m_tComCfg.m_iMaxLimitOfUploadFile);
                                                break;
                                        }
                                }
                                
                                int iNeedRecvMsgLen = 0;

                                int iRecvRet = 0;
                                g_insLogMng.debug("protocol", CLogMng::LogMode3, "Begin to recv msg body data...");

                                int iWriteLogFlag = 0;
                                while(iLeaveMsgLen > 0)
                                {
                                        //ÿ�ε���Ҫ�ж��Ƿ��Ѿ���ȡ��,ȡ�������ٴ���
                                        if (0 == iWriteLogFlag && 0 != g_insQueueMng.judgeNormal(pthread_self(), CQueueMng::RecvingQueue))
                                        {
                                                eResponseCode = CancelDuty;  //״̬�����˸ı䣬����Ϊ�ɹ�
                                                iRecvRet = 1;
                                                g_insLogMng.normal( "Find the recving duty(%s) is canceled, give up it but need recv all datas.",
                                                        insFileUploadMsg.getPrintInfo().c_str());
                                                iWriteLogFlag = 1;
                                                //break;   // --PAUSE--Ϊ�˲�ʹ�Դ�дʧ�ܵ����ط�,�����ж�����,����󲻴���ķ�ʽ
                                        }

                                        //<<CONTINUE 1 or while ÿ����һ��Ҫȷ����Ϣ����֤��Ϣ������,��ʱ�����öϵ������ķ�ʽ
                                        //����λ��������´ν���֮ǰ������û�н�����Ϣ�Ĳ������������Է�ֹ�Զ�д����
                                        // Continue while end�Ǵβ�����
                                        if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                                        {
                                                if (0 != sendResponse(insTcpServer, Success))
                                                {
                                                        g_insLogMng.error("Send the duty(%s) data block response msg failed.",
                                                                insFileUploadMsg.getPrintInfo().c_str());
                                                        eResponseCode = ServicePause;
                                                        break;
                                                }
                                        }

                                        //����
                                        iNeedRecvMsgLen = (iLeaveMsgLen >= MsgBufferSize)?MsgBufferSize:iLeaveMsgLen;
                                        g_insLogMng.debug("protocol", CLogMng::LogMode3, "Recv msg body data block...");
                                        iRecvMsgRet = insTcpServer.receiveMsg(sMsgBuff, iNeedRecvMsgLen, iFileTimeout);
                                        if ( 0 != iRecvMsgRet)
                                        {
                                                iRecvRet = 2;
                                                if (1 == iRecvMsgRet)
                                                {
                                                        eResponseCode = CancelDuty;
                                                        g_insLogMng.normal("Recv the duty(%s) data block canceled.",
                                                                insFileUploadMsg.getPrintInfo().c_str());
                                                }
                                                else
                                                {
                                                        eResponseCode = DataFail;
                                                        g_insLogMng.error("Recv the duty(%s) data block failed.",
                                                                insFileUploadMsg.getPrintInfo().c_str());
                                                }
                                                break;
                                        } //CONTINUE while>> ȷ����Ϣ����һ��ѭ����ʱ���ͣ������������Ǹ�������Ϣ

                                        //��д�ļ�
                                        if (int(iWriteFileBuffLen + iNeedRecvMsgLen) > WriteToFileSize)
                                        {
                                                if (1 != fwrite(sWriteFileBuff, iWriteFileBuffLen, 1, fp))
                                                {
                                                        eResponseCode = DataFail;
                                                        iRecvRet = 3;
                                                        g_insLogMng.error("Write the duty(%s) data block to file(%s) failed.",
                                                                insFileUploadMsg.getPrintInfo().c_str(), sTmpPathFile.c_str());
                                                        break;
                                                }
                                                iWriteFileBuffLen = 0;  //�� 0
                                        }

                                        //�������ν��յ��ַ�
                                        memcpy(sWriteFileBuff + iWriteFileBuffLen, sMsgBuff, iNeedRecvMsgLen);
                                        iWriteFileBuffLen = iWriteFileBuffLen + iNeedRecvMsgLen;
                                        iLeaveMsgLen = iLeaveMsgLen - iNeedRecvMsgLen;
                                }

                                //�Ƿ���ʣ����ַ�û��д��
                                if (iWriteFileBuffLen != 0 && 0 == iRecvRet)
                                {
                                        if (1 != fwrite(sWriteFileBuff, iWriteFileBuffLen, 1, fp))
                                        {
                                                eResponseCode = DataFail;
                                                g_insLogMng.error("Write the duty(%s) data block to file(%s) failed..",
                                                                insFileUploadMsg.getPrintInfo().c_str(), sTmpPathFile.c_str());
                                                fclose(fp);
                                                break;
                                        }
                                }

                                //�ر�
                                fclose(fp);

                                //������������,��ʧ�ܻ��߷�������,���˳�
                                if (0 != iRecvRet)  //�������ж�--PAUSE--
                                {
                                        break; //�����ʹ�����ǰ���Ѿ�����,����Ҫ�������д��
                                }

                                //������ɣ����ļ��ƶ�����ȷλ��
                                g_insLogMng.debug(insFileUploadMsg.m_sSiteName, CLogMng::LogMode2, "mv %s to %s.",
                                        sTmpPathFile.c_str(), sRealPathFile.c_str());
                                if (0 != CComFun::mvFile(sTmpPathFile, sRealPathFile))
                                {
                                        eResponseCode = DataFail; 
                                        g_insLogMng.error("the duty(%s), mvFile(%s,%s) failed.",
                                                                insFileUploadMsg.getPrintInfo().c_str(), sTmpPathFile.c_str(), sRealPathFile.c_str());
                                        break;
                                }
                                
                                sSaveLocalFileName = sRealPathFile;

                                //�ж�һ���Ƿ�Ҫ����Ȩ�޵ĸ���,��ֵ��������޸�
                                if('\0' != g_insConfigMng.m_tComCfg.m_sFileModeOption[0])
                                {
                                        CComFun::chmodFile(sRealPathFile, g_insConfigMng.m_tComCfg.m_sFileModeOption);
                                }
                                
                                //kevinliu add 2007-01-10 ���ļ��ڶ�Ӧ��λ���½Ӱ�
                                if (pMsgHead[1] == ProtocolPackFile)
                                {
                                        g_insLogMng.debug(insFileUploadMsg.m_sSiteName, CLogMng::LogMode2, "need unpack %s",
                                                sRealPathFile.c_str());
                                        int iTmp = CComFun::unpackFile(sRealPathFile);   
                                        if (0 != iTmp)
                                        {
                                                eResponseCode = Fail; 
                                                g_insLogMng.error("the duty(%s), unpack(%s) failed, ret=%d .",
                                                                insFileUploadMsg.getPrintInfo().c_str(), sRealPathFile.c_str(), iTmp);
                                                break;
                                        }
                                }

                                //׼�����ɴ���������(2.2�ڶ����֣��ļ���Ϣ)
                                time((time_t*)&insSendDuty.m_iLocalTime); //��ǰ����
                                strcpy(insSendDuty.m_sLocalPathFile, sRealPathFile.c_str());

                                //���ɴ��������񲢼������(2.3�������֣���������Ϣ)
                                if ( 0 != g_insQueueMng.putDuty2SendQ(insSendDuty, CQueueMng::AddServerInfo, insFileUploadMsg.m_iDataLength))
                                {
                                        eResponseCode = ServicePause; //�������Ѿ�д����־��������д
                                        break;
                                }
                                //CONTINUE while end << ������Ǵ���Ϣ

                        }
                        g_insLogMng.debug(insFileUploadMsg.m_sSiteName, CLogMng::LogMode1, "END_suc<<%s>>",
                                 insFileUploadMsg.getPrintInfo().c_str());
                        
                        //�ؼ��ؼ���־��¼�����ô򿪿��أ���¼���յ������е��ļ�վ����ļ�����Ӧ�ò��������Ӱ��ܴ�
                        g_insLogMng.runlog("Type=%d,%s=>%s:%s", 
                                  pMsgHead[1], 
                                  sSaveLocalFileName.c_str(),
                                  insFileUploadMsg.m_sSiteName, insFileUploadMsg.m_sDestPathFile);
                        break;
                }
                case ProtocolDelFile:
                {
                        //������Ϣ��Ϣ����
                        CFileDelMsg insFileDelMsg;
                        iRecvMsgRet = insTcpServer.receiveMsg((char*)(&insFileDelMsg), sizeof(CFileDelMsg), iFileTimeout);
                        if (0 != iRecvMsgRet) //�����ļ�ʱ�䣬���ϳ����ʱ��
                        {
                                if (1 == iRecvMsgRet)
                                {
                                        eResponseCode = CancelDuty;
                                        g_insLogMng.normal("Recv FileDelMsg canceled because recv msg len error.");
                                }
                                else
                                {
                                        g_insLogMng.error("Recv FileDelMsg failed because recv msg len error.");
                                        eResponseCode = HeadFail;
                                }
                                break;
                        }
                        //�����Ӧ����һ�鴦����Ϊ�������ڻ��к�����Ϣ������������ܵ��¶Զ�д���� CONTINUE1>><<
                        //����һ����Ӧ��Ϣ�ķ��ӣ������һ����Ӧ��Ϣ,���Խ�����  <<
                        //<<--Ϊ�˷�ֹ,���͹�������Ϣ���ֶ�û�н�����,���Ҫ��char����Ľ�������
                        insFileDelMsg.sureValidSrt();

                        g_insLogMng.debug(insFileDelMsg.m_sSiteName, CLogMng::LogMode1, "START<<%s>>",
                                 insFileDelMsg.getPrintInfo().c_str());

                        //�������ڽ�������
                        CRecvingDuty insRecvingDuty;
                        insRecvingDuty.m_iState = CRecvingDuty::Normal;
                        insRecvingDuty.m_iProtocolType = ProtocolDelFile;
                        strcpy(insRecvingDuty.m_sSiteName, insFileDelMsg.m_sSiteName);
                        strcpy(insRecvingDuty.m_sDestPathFile, insFileDelMsg.m_sDestPathFile);
                        strcpy(insRecvingDuty.m_sSendedTime , insFileDelMsg.m_sSendedTime);

                        //վ���Ƿ����
                        map<string, CSiteInfo> insMapSite;
                        g_insConfigMng.getSiteMap(insMapSite);
                        if (insMapSite.end() == insMapSite.find(insRecvingDuty.m_sSiteName))
                        {
                                g_insLogMng.normal("FileDelMsg error because site:%s does not exist.",
                                        insRecvingDuty.m_sSiteName);
                                eResponseCode = InfoFail;
                                break;
                        }

                        //��������Ϣ�Ƿ���Ҫ���������Ҫ��Ӵ������Ѿ���check����ӵ������У�����Ҫ����ֱ���˳�����
                        if (0 > g_insQueueMng.check(insRecvingDuty))
                        {
                                eResponseCode = CancelDuty;  //�������Ѿ�д����־��������д
                                //break; --PAUSE--Ϊ�˲�ʹ�Դ�дʧ�ܵ����ط�,�����ж�����,����󲻴���ķ�ʽ
                        }

                        //��ʼ���գ�׼�����ɴ���������(��һ���֣�������Ϣ)
                        CSendDuty insSendDuty;
                        insSendDuty.m_iState = CSendDuty::Normal;
                        insSendDuty.m_iProtocolType = ProtocolDelFile;
                        strcpy(insSendDuty.m_sSiteName, insRecvingDuty.m_sSiteName);
                        strcpy(insSendDuty.m_sDestPathFile, insRecvingDuty.m_sDestPathFile);
                        strcpy(insSendDuty.m_sSendedTime,  insRecvingDuty.m_sSendedTime);

                        //׼�����ɴ���������(�ڶ����֣��ļ���Ϣ)
                        time((time_t*)&insSendDuty.m_iLocalTime); //��ǰ����
                        string sPathFile = g_insConfigMng.getSiteRootDir(insRecvingDuty.m_sSiteName);
                        sPathFile = CComFun::analysePathFile(sPathFile, insRecvingDuty.m_sDestPathFile, 0);
                        if ("" == sPathFile)
                        {
                                 g_insLogMng.error("Delete file failed, analysePathFile(%s,%s) is an invalid pathfile.",
                                        (g_insConfigMng.getSiteRootDir(insRecvingDuty.m_sSiteName)).c_str(),
                                        insRecvingDuty.m_sDestPathFile);
                                eResponseCode = InfoFail;
                                break;
                        }
                        strcpy(insSendDuty.m_sLocalPathFile, sPathFile.c_str());

                        //ִ��
                        if (0 != g_insQueueMng.judgeNormal((int)pthread_self(), CQueueMng::RecvingQueue))
                        {
                                eResponseCode = CancelDuty;  //״̬�����˸ı䣬����Ϊ�ɹ�
                                //break; //--PAUSE--Ϊ�˲�ʹ�Դ�дʧ�ܵ����ط�,�����ж�����,����󲻴���ķ�ʽ
                        }

                        // --PAUSE--Ϊ�˲�ʹ�Դ�дʧ�ܵ����ط�,�����ж�����,����󲻴���ķ�ʽ
                        if (eResponseCode == CancelDuty)
                        {
                                g_insLogMng.debug(insFileDelMsg.m_sSiteName, CLogMng::LogMode1, "OVER_suc<<%s>>",
                                        insFileDelMsg.getPrintInfo().c_str());
                                break;
                        }

                         g_insLogMng.debug(insFileDelMsg.m_sSiteName, CLogMng::LogMode2, "delete %s.",
                                        sPathFile.c_str());
                        if (0 != CComFun::delFile(sPathFile))  //�ڿ�ʼ�Ļ�����Ҳִ��һ��,����û���ļ�,Ϊ�˴���ͳһ,ֻ�����
                        {
                                g_insLogMng.error("Delete file(%s) failed, but continue.",
                                        sPathFile.c_str());  //д����־,���Ǽ���ִ��
                        }

                        //׼�����ɴ���������(�������֣���������Ϣ)
                        if ( 0 != g_insQueueMng.putDuty2SendQ(insSendDuty, CQueueMng::AddServerInfo))
                        {
                                eResponseCode = ServicePause;  //�������Ѿ�д����־��������д
                                break;
                        }

                        g_insLogMng.debug(insFileDelMsg.m_sSiteName, CLogMng::LogMode1, "OVER_suc<<%s>>",
                                 insFileDelMsg.getPrintInfo().c_str());
                        break;
                }
                //BEGIN kevinliu add 2006-04-21
                case ProtocolCpFile:
                case ProtocolMvFile:
                {
                        //������Ϣ��Ϣ����
                        CFileMvCpMsg insFileMvCpMsg;
                        iRecvMsgRet = insTcpServer.receiveMsg((char*)(&insFileMvCpMsg), sizeof(CFileMvCpMsg), iFileTimeout);
                        if (0 != iRecvMsgRet) //�����ļ�ʱ�䣬���ϳ����ʱ��
                        {
                                if (1 == iRecvMsgRet)
                                {
                                        eResponseCode = CancelDuty;
                                        g_insLogMng.normal("Recv CFileMvCpMsg canceled because recv msg len error.");
                                }
                                else
                                {
                                        g_insLogMng.error("Recv CFileMvCpMsg failed because recv msg len error.");
                                        eResponseCode = HeadFail;
                                }
                                break;
                        }
                        //�����Ӧ����һ�鴦����Ϊ�������ڻ��к�����Ϣ������������ܵ��¶Զ�д���� CONTINUE1>><<
                        //����һ����Ӧ��Ϣ�ķ��ӣ������һ����Ӧ��Ϣ,���Խ�����  <<
                        //<<--Ϊ�˷�ֹ,���͹�������Ϣ���ֶ�û�н�����,���Ҫ��char����Ľ�������
                        insFileMvCpMsg.sureValidSrt();

                        g_insLogMng.debug(insFileMvCpMsg.m_sSiteName, CLogMng::LogMode1, "START<<%s>>",
                                 insFileMvCpMsg.getPrintInfo().c_str());

                        //�������ڽ�������
                        CRecvingDuty insRecvingDuty;
                        insRecvingDuty.m_iState = CRecvingDuty::Normal;
                        insRecvingDuty.m_iProtocolType = (ProtocolMvFile==pMsgHead[1])?ProtocolMvFile:ProtocolCpFile;
                        strcpy(insRecvingDuty.m_sSiteName, insFileMvCpMsg.m_sSiteName);
                        strcpy(insRecvingDuty.m_sDestPathFile, insFileMvCpMsg.m_sDestPathFile);
                        strcpy(insRecvingDuty.m_sSendedTime , insFileMvCpMsg.m_sSendedTime);

                        //վ���Ƿ����
                        map<string, CSiteInfo> insMapSite;
                        g_insConfigMng.getSiteMap(insMapSite);
                        if (insMapSite.end() == insMapSite.find(insRecvingDuty.m_sSiteName))
                        {
                                g_insLogMng.normal("FileMvCpMsg error because site:%s does not exist.",
                                        insRecvingDuty.m_sSiteName);
                                eResponseCode = InfoFail;
                                break;
                        }

                        //��������Ϣ�Ƿ���Ҫ���������Ҫ��Ӵ������Ѿ���check����ӵ������У�����Ҫ����ֱ���˳�����
                        if (0 > g_insQueueMng.check(insRecvingDuty))
                        {
                                eResponseCode = CancelDuty;  //�������Ѿ�д����־��������д
                                //break; --PAUSE--Ϊ�˲�ʹ�Դ�дʧ�ܵ����ط�,�����ж�����,����󲻴���ķ�ʽ
                        }

                        //��ʼ���գ�׼�����ɴ���������(��һ���֣�������Ϣ)
                        CSendDuty insSendDuty;
                        insSendDuty.m_iState = CSendDuty::Normal;
                        insSendDuty.m_iProtocolType = insRecvingDuty.m_iProtocolType;
                        strcpy(insSendDuty.m_sSiteName, insRecvingDuty.m_sSiteName);
                        strcpy(insSendDuty.m_sDestPathFile, insRecvingDuty.m_sDestPathFile);
                        strcpy(insSendDuty.m_sSendedTime,  insRecvingDuty.m_sSendedTime);

                        //׼�����ɴ���������(�ڶ����֣��ļ���Ϣ)
                        time((time_t*)&insSendDuty.m_iLocalTime); //��ǰ����
                        strcpy(insSendDuty.m_sLocalPathFile, insFileMvCpMsg.m_sSrcPathFile);

                        //Ŀ���ļ�
                        string sDestPathFile = g_insConfigMng.getSiteRootDir(insRecvingDuty.m_sSiteName);
                        sDestPathFile = CComFun::analysePathFile(sDestPathFile, insRecvingDuty.m_sDestPathFile, 1);
                        if ("" == sDestPathFile)
                        {
                                 g_insLogMng.error("MvCp file failed, analysePathFile(%s,%s) is an invalid pathfile.",
                                        (g_insConfigMng.getSiteRootDir(insRecvingDuty.m_sSiteName)).c_str(),
                                        insRecvingDuty.m_sDestPathFile);
                                eResponseCode = InfoFail;
                                break;
                        }

                        //Դ�ļ�
                        vector<string> tmpPathInfo;
                        CComFun::divideupStr(insFileMvCpMsg.m_sSrcPathFile, tmpPathInfo, ':');
                        if (2 != tmpPathInfo.size())
                        {
                                 g_insLogMng.error("MvCp file failed, analysePathFile(%s) is an invalid pathfile.",
                                       insFileMvCpMsg.m_sSrcPathFile);
                                eResponseCode = InfoFail;
                                break;
                        }
                        string sSrcPathFile = g_insConfigMng.getSiteRootDir(tmpPathInfo[0]);
                        if ("" == sSrcPathFile)
                        {
                                 g_insLogMng.error("MvCp file failed, %s site not exist.",
                                       tmpPathInfo[0].c_str());
                                eResponseCode = InfoFail;
                                break;
                        }
                        sSrcPathFile = CComFun::analysePathFile(sSrcPathFile, tmpPathInfo[1], 0);
                        if ("" == sSrcPathFile)
                        {
                                  g_insLogMng.error("MvCp file failed, analysePathFile(%s) is an invalid pathfile.",
                                       insFileMvCpMsg.m_sSrcPathFile);
                                eResponseCode = InfoFail;
                                break;
                        }

                        //ִ��
                        if (0 != g_insQueueMng.judgeNormal((int)pthread_self(), CQueueMng::RecvingQueue))
                        {
                                eResponseCode = CancelDuty;  //״̬�����˸ı䣬����Ϊ�ɹ�
                                //break; //--PAUSE--Ϊ�˲�ʹ�Դ�дʧ�ܵ����ط�,�����ж�����,����󲻴���ķ�ʽ
                        }

                        // --PAUSE--Ϊ�˲�ʹ�Դ�дʧ�ܵ����ط�,�����ж�����,����󲻴���ķ�ʽ
                        if (eResponseCode == CancelDuty)
                        {
                                g_insLogMng.debug(insFileMvCpMsg.m_sSiteName, CLogMng::LogMode1, "OVER_suc<<%s>>",
                                        insFileMvCpMsg.getPrintInfo().c_str());
                                break;
                        }

                         g_insLogMng.debug(insFileMvCpMsg.m_sSiteName, CLogMng::LogMode2, "mv %s %s.",
                                        sDestPathFile.c_str(), sSrcPathFile.c_str());

                        if (ProtocolMvFile == pMsgHead[1])  // mvoe
                        {
                                if (0 != CComFun::mvFile(sSrcPathFile, sDestPathFile))
                                {
                                        g_insLogMng.error("Mv file(%s %s) failed, but continue.",
                                                sDestPathFile.c_str(), sSrcPathFile.c_str());  //д����־,���Ǽ���ִ��
                                }
                        }
                        else  //copy
                        {
                                if (0 != CComFun::cpFile(sSrcPathFile, sDestPathFile))
                                {
                                        g_insLogMng.error("cp file(%s %s) failed, but continue.",
                                                sDestPathFile.c_str(), sSrcPathFile.c_str());  //д����־,���Ǽ���ִ��
                                }
                        }

                        //׼�����ɴ���������(�������֣���������Ϣ)
                        if ( 0 != g_insQueueMng.putDuty2SendQ(insSendDuty, CQueueMng::AddServerInfo))
                        {
                                eResponseCode = ServicePause;  //�������Ѿ�д����־��������д
                                break;
                        }

                        g_insLogMng.debug(insFileMvCpMsg.m_sSiteName, CLogMng::LogMode1, "OVER_suc<<%s>>",
                                 insFileMvCpMsg.getPrintInfo().c_str());
                        break;
                }
                //END kevinliu add 2006-04-21                
                case ProtocolAddServer:
                {
                        //������Ϣ��Ϣ����
                        CServerAddMsg insServerAddMsg;
                        iRecvMsgRet = insTcpServer.receiveMsg((char*)(&insServerAddMsg), sizeof(CServerAddMsg), iFileTimeout);
                        if (0 != iRecvMsgRet)//�����ļ�ʱ�䣬���ϳ����ʱ��
                        {
                                if (1 == iRecvMsgRet)
                                {
                                        eResponseCode = CancelDuty;
                                          g_insLogMng.normal("Recv ServerAddMsg canceled because recv msg len error.");
                                }
                                else
                                {
                                        g_insLogMng.error("Recv ServerAddMsg failed because recv msg len error.");
                                        eResponseCode = HeadFail;
                                }
                                break;
                        }
                        //<<--Ϊ�˷�ֹ,���͹�������Ϣ���ֶ�û�н�����,���Ҫ��char����Ľ�������
                        insServerAddMsg.sureValidSrt();

                        insServerAddMsg.m_iPort = ntohl(insServerAddMsg.m_iPort);

                        g_insLogMng.debug("protocol", CLogMng::LogMode1, "ADD SERVER<<%s>>",
                                insServerAddMsg.getPrintInfo().c_str());

                        //����վ�����վ������������ã��Ѿ������й����У�������ʹ�ã��������ٴ���
                        //ֻ����վ�����������
                        if (0 != CServerMng::addServer(insServerAddMsg))
                        {
                                g_insLogMng.error("Add Server(%s) failed.",
                                        insServerAddMsg.getPrintInfo().c_str());
                                break;
                        }

                        g_insLogMng.normal("%s added successfully.", insServerAddMsg.getPrintInfo().c_str());

                        break;
                }
                case ProtocolDelServer:
                {
                       //������Ϣ��Ϣ����
                        CServerDelMsg insServerDelMsg;
                       iRecvMsgRet = insTcpServer.receiveMsg((char*)(&insServerDelMsg), sizeof(CServerDelMsg), iFileTimeout);
                        if (0 != iRecvMsgRet)//�����ļ�ʱ�䣬���ϳ����ʱ��
                        {
                                if (1 == iRecvMsgRet)
                                {
                                        eResponseCode = CancelDuty;
                                         g_insLogMng.normal("Recv ServerDelMsg cancel bacause recv msg len error.");
                                }
                                else
                                {
                                        g_insLogMng.error("Recv ServerDelMsg failed bacause recv msg len error.");
                                        eResponseCode = HeadFail;
                                }
                                break;
                        }
                        //<<--Ϊ�˷�ֹ,���͹�������Ϣ���ֶ�û�н�����,���Ҫ��char����Ľ�������
                        insServerDelMsg.sureValidSrt();

                        insServerDelMsg.m_iPort = ntohl(insServerDelMsg.m_iPort);

                        g_insLogMng.debug("protocol", CLogMng::LogMode1, "DELSERVER<<%s>>",
                                insServerDelMsg.getPrintInfo().c_str());

                        //����վ�����վ������������ã��Ѿ������й����У�������ʹ�ã��������ٴ���
                        //ֻ����վ�����������
                        if (0 != CServerMng::delServer(insServerDelMsg))
                        {
                                g_insLogMng.error("Del Server(%s) failed.",
                                        insServerDelMsg.getPrintInfo().c_str());
                                break;
                        }

                        g_insLogMng.normal("%s deleted successfully.",  insServerDelMsg.getPrintInfo().c_str());

                        break;
                }
                case ProtocolCheckStatus:
                {
                        g_insLogMng.debug("protocol", CLogMng::LogMode2, "Recv a checkstatus msg.");
                        eResponseCode = checkOK;
                        break;
                }
                case ProtocolHeartBeat:
                {
                        g_insLogMng.error("Recv heartbead, current not support.");
                        eResponseCode = MsgError;
                        break;
                }
                case ProtocolAddSubServer:
                {
                        //���Ҳ�֧��
                        g_insLogMng.error("Recv AddSubServer, current not support.");
                        eResponseCode = MsgError;
                        break;
                }
                case ProtocolDelSubServer:
                {
                        //���Ҳ�֧��
                        g_insLogMng.error("Recv DelSubServer, current not support.");
                        eResponseCode = MsgError;
                        break;
                }
                default:
                {
                        g_insLogMng.error("Recv unknown msg.");
                        eResponseCode = MsgError;
                        break;
                }
                }  // end switch
        }
        else
        {
                if (1 == iRecvMsgRet )
                {
                        eResponseCode = CancelDuty;
                }
                else
                {
                        g_insLogMng.error("Recv msg head failed.");
                        eResponseCode = HeadFail;
                }
        }

        //����������Ӧ��Ϣ
        if (CancelDuty == eResponseCode)  //--PAUSE--ȡ���������񲻴���
        {
                eResponseCode = Success;
        }

        g_insLogMng.debug("protocol", CLogMng::LogMode2, "Send last response msg.");
        if (0 != sendResponse(insTcpServer, eResponseCode))
        {
                g_insLogMng.normal("Send last response msg failed.");
        }

        //����һ�δ���
        g_insLogMng.debug("protocol", CLogMng::LogMode2, "End to protocol process.");

        return;

}


void CProtocolMng::processSend(int iType)   // 2007-05-21 add itype
{
        //�ڶ�����ȡ��һ������
        CSendDuty insSendingDuty;
        if (0 != g_insQueueMng.getDutyFSendQ(iType, insSendingDuty))  //�Ѿ����������ڷ��͵Ķ���
        {
                return;  //û������,����ȡ����
        }
        // BEGIN kevinliu 2007-06-14 list���У����Կ��ж��ļ��ı���ʱ���������Ƿ�������Ч
        if ((0 == g_insMaintain.getFlag(CMaintain::QueueSafeMode)) &&
                (insSendingDuty.m_iProtocolType == (int)ProtocolUploadFile)) 
        //���ɿ����õ�,��ֹ���������,����ҲҪ��ֻ�����ϴ��ļ�
        //��һ�ֿ��ܾ���bj78�ϣ��ļ����޸���(����Ԥ��),���ֽ��ᵼ�±����������ָ��ʺ�С��
        //һ���������ԣ�������������Ϊ1
        {
                //��ǰ�ŵ������Ҳ���������棬�������������һ��connectserver�Ĳ���
                //ͬʱ�����Ӧ��Ҳ���Էŵ�getDutyFSendQ��ʵ�֣����ǿ��ǵ�һ�����ڶ�������޸ģ�
                //��ʹ�����й��������
                struct stat tmpStat;
                if(0 <= stat( insSendingDuty.m_sLocalPathFile, &tmpStat))
                {
                        //m_iLocalTime ʱ�����ÿ���յ���Ϣ�����ļ�ʱ���������ʱ��£������ļ�����������
                        //���ʱ�䶼�����ʱ�俿���ˣ�˵���ļ����޸Ĺ��ˣ�ͬʱ�϶������µ���Ϣ�������Ҳ��ٷ���
                        if (tmpStat.st_mtime > insSendingDuty.m_iLocalTime)
                        {
                                //дnormal��־,���ڹؼ��ķ����ļ�����¼��־Ϊ��
                                g_insLogMng.normal("WhenSend: %s=>%s:%s has been modified, give up.",
                                        insSendingDuty.m_sLocalPathFile,
                                        insSendingDuty.m_sSiteName,
                                        insSendingDuty.m_sDestPathFile);
                                g_insQueueMng.setOver((int)pthread_self(), CQueueMng::SendingQueue);
                                return;
                        }
                }
        }      
        //END kevinliu 2007-06-14
        g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode1, "BEGIN<<%s>>",
               insSendingDuty.getPrintInfo().c_str());

        //��insSendDuty ��ȡ��IP�Ͷ˿ڣ��������Ӻͷ���
        //ע��ÿ��Ҫ�ж�һ�£��Ƿ������ȡ������
        int iMsgTimeout = g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout;  //��Ϣ��ʱʱ��
        int iFileTimeout = g_insConfigMng.m_tComCfg.m_iRecvFileTimeout;  //�ļ���ʱʱ��

        CTCPSocket  insClientSocket(insSendingDuty.m_sServerIP, insSendingDuty.m_iServerPort);
        if ( 0 != insClientSocket.connectServer())
        {
                g_insLogMng.error("Connect to server failed when sending duty(%s).",
                        insSendingDuty.getPrintInfo().c_str());
                insSendingDuty.m_iNrOfFail++;
                g_insQueueMng.putDuty2SendQ(insSendingDuty, CQueueMng::Direct); //���·������
                g_insQueueMng.setOver((int)pthread_self(), CQueueMng::SendingQueue);//add1025
                return;
        }
        //��ӱ�������connectServer����ʵ����ã����ǿ��ǵ����룬��ʱ�ӵ����������С�ط�connect�ģ����Ҳ�����
        if(insClientSocket.m_iFd > 900)
        {
                if (0 != access((g_sHomeDir + "/cfg/fd.if").c_str(),  F_OK))  //�����ھͱ��������ȷ��ϵͳ����֧�ֵ�1000��������Ӵ��ļ���������
                {
                        g_insMaintain.sendMobileMsg("���������ļ�������FD>900,ϵͳ�Ѿ��Զ�����ǰ500��������ϵͳ�Ƿ���FDй¶��");
                        //for(int ixx = 10; ixx < 500; ixx++)
                        //{
                        //        close(ixx); //Ϊ�˷�ֹϵͳ���µĹ��ϣ����ǿ�м�������߼������ȷ�ϲ���Ҫ������cfg���fd.if�ļ�
                        //}
                }
                //дalarm�����ļ� [ALARM:4]
                CComFun::writeWholeFileContent((g_sHomeDir + "/dat/maxfd.alm"), "fd>900"); 
        }
        //else
        //{
        //        CComFun::delFile(g_sHomeDir + "/dat/maxfd.alm"); 
        //}
        
        //��Ϣ���������ļ��ݴ滺����
        char sMsgBuff[MsgBufferSize + 1];
        char sFileBuff[WriteToFileSize + 1];

        //������Ϣͷ��Ϣ
        sMsgBuff[0] = Version1;
        sMsgBuff[1] = insSendingDuty.m_iProtocolType;

         g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode3, "Begin to send the msg head.");
        if (0 != insClientSocket.sendMsg(sMsgBuff, MsgHeadLen, iMsgTimeout))  //  send 1.1
        {
                 g_insLogMng.error("Write msg head data to socket failed when sending duty(%s).",
                        insSendingDuty.getPrintInfo().c_str());
                 insClientSocket.closeSocket();
                 insSendingDuty.m_iNrOfFail++;
                g_insQueueMng.putDuty2SendQ(insSendingDuty, CQueueMng::Direct); //���·������
                g_insQueueMng.setOver((int)pthread_self(), CQueueMng::SendingQueue);
                return;
        }

        //���ݲ�ͬ��Э�飬�ֱ���в�ͬ�Ĵ���
        int iNotWaitResultMsg = 0; // ��Ҫ���⴦��ĵط��ı�־,0����,����,��Ҫ����
        ResponseCode eLastResponseCode = UnknownCode;
        switch(insSendingDuty.m_iProtocolType)
        {
        case ProtocolUploadFile:
        case ProtocolPackFile:  // add by kevinliu 2007-01-10
        {
                //�Ƿ�����Ҳ���Ŀ¼
                struct stat stSendFileStat;
                if(stat(insSendingDuty.m_sLocalPathFile, &stSendFileStat)!= 0)
                {
                        g_insLogMng.error("%s not exist where send the duty(%s).",
                                insSendingDuty.m_sLocalPathFile,
                                insSendingDuty.getPrintInfo().c_str());
                         iNotWaitResultMsg = 1;
                        break;
                }
                if (S_ISDIR(stSendFileStat.st_mode))
                {
                        g_insLogMng.error("%s is a directory where send the duty(%s).",
                                insSendingDuty.m_sLocalPathFile,
                                insSendingDuty.getPrintInfo().c_str());
                         iNotWaitResultMsg = 1;
                        break;
                }
                //�ļ���С
                long iFileSize = stSendFileStat.st_size;
                if (0 == iFileSize && 0 == g_insConfigMng.m_tComCfg.m_iIfUploadZeroFile )
                {
                         g_insLogMng.error("%s size is zero where send the duty(%s).",
                                insSendingDuty.m_sLocalPathFile,
                                insSendingDuty.getPrintInfo().c_str());
                         iNotWaitResultMsg = 1;
                        break;
                }


                //��ʼ����
                FILE *fp = fopen(insSendingDuty.m_sLocalPathFile, "r");
                if (NULL == fp)
                {
                         g_insLogMng.error("Open %s failed where send the duty(%s).",
                                insSendingDuty.m_sLocalPathFile,
                                insSendingDuty.getPrintInfo().c_str());
                         iNotWaitResultMsg = 1;
                        break;
                }

                //ƴװ��������Ϣ��info��Ϣ
                CFileUploadMsg insFileUploadMsg;
                strcpy(insFileUploadMsg.m_sSiteName, insSendingDuty.m_sSiteName);
                strcpy(insFileUploadMsg.m_sDestPathFile, insSendingDuty.m_sDestPathFile);
                strcpy(insFileUploadMsg.m_sSendedTime, insSendingDuty.m_sSendedTime);
                insFileUploadMsg.m_cDataType = CFileUploadMsg::FileData;
                insFileUploadMsg.m_iDataLength = htonl(iFileSize);
                g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode3,  "Begin to send the info head");
                if (0 != insClientSocket.sendMsg((char*)(&insFileUploadMsg), sizeof(CFileUploadMsg), iMsgTimeout))
                {
                        g_insLogMng.error("Write FileUploadMsg to socket failed when sending duty(%s).",
                               insSendingDuty.getPrintInfo().c_str());
                        fclose(fp);
                        break;
                 }
                //CONTINUEÿ����һ��Ҫ��ȡȷ����Ϣ����֤��Ϣ������,��ʱ�����öϵ������ķ�ʽ
                else
                {
                        if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                        {
                                ResponseCode eResponseCode;
                                if (0 != recvResponse(insClientSocket, eResponseCode))
                                {
                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg head to socket,but recv response timeout.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        fclose(fp);
                                        eLastResponseCode = ServicePause;
                                        break;
                                }
                                if (CancelDuty == eResponseCode)
                                {
                                        fclose(fp);
                                        eLastResponseCode = Success;
                                        g_insLogMng.normal("Send Duty(%s)'s FileUploadMsg head to socket, recv CancelDuty responecode.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        break;
                                }
                                else if (Success == eResponseCode)
                                {
                                        //����
                                        eLastResponseCode = Success;
                                }
                                else //ʧ��
                                {
                                        fclose(fp);
                                        eLastResponseCode = eResponseCode;
                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg head block to socket,recv %x responecode.",
                                                                insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                        break;
                                }
                        }
                }

                //��ʼ�����ļ�����Ϣ
                int iLeftSize = iFileSize;
                int iWantBlockSize = 0;
                int iReadLen = 0;
                int iSendLen = 0;
                int iHadSendLen = 0;
                g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode3,
                        "Begin to send the msg data stream,filestream len = %ld", iFileSize);

                //����һ�£�����10��Ҫ��Ϣ���ٺ���
                int iCtrlTime = 0;
                if (0 != g_insConfigMng.m_tComCfg.m_iSendFlowCtrl)
                {
                        //�ж��ٸ�10K
                        int iSizeCtrl = g_insConfigMng.m_tComCfg.m_iSendFlowCtrl/10;

                        //ÿ����Ҫ�ķѶ��ٺ��룬����㷨�У�����������ÿ��10Mʱ�����޷�����������
                        if (0 == iSizeCtrl)
                        {
                                iCtrlTime = 1000;  //����������Ʋ���10K����
                        }
                        else
                        {
                                iCtrlTime = 1000/iSizeCtrl;
                        }
                        g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode1,
                                "Send msg data need control flow(sizeCtrl = %d, CtrlTime = %d)", iSizeCtrl, iCtrlTime);
                }

                //��ʼ����
                while(iLeftSize > 0)
                {
                        //�жϱ��θö�����
                        if (iLeftSize >= WriteToFileSize)
                        {
                                iReadLen = WriteToFileSize;
                        }
                        else
                        {
                                iReadLen = iLeftSize;
                        }

                        //��ȡ
                        iWantBlockSize = fread(sFileBuff, iReadLen, 1, fp);
                        //��Ϊ�ɹ�,Ψһʧ�ܵ�����������ļ���ȡ�б���д,��¼��־������
                        if (1 != iWantBlockSize)
                        {
                                g_insLogMng.normal("fread(size=%d) != 1 when send the duty(%s).",
                                        iReadLen, insSendingDuty.getPrintInfo().c_str());
                                break;
                        }
                         iLeftSize = iLeftSize - iReadLen;  //��дʣ���ֽ�

                        iHadSendLen = 0;
                        while(iReadLen > 0)
                        {
                                iSendLen = (iReadLen >= MsgBufferSize)?MsgBufferSize:iReadLen;
                                memcpy(sMsgBuff, sFileBuff+ iHadSendLen, iSendLen);

                                //�ж�һ�������Ƿ�ȡ��,����ȡ������������ν����Ϊ�Զ˼�ʹ����ʧ�ܣ�Ҳ�����ٽ��к���������
                                if (0 != g_insQueueMng.judgeNormal((int)pthread_self(), CQueueMng::SendingQueue))
                                {
                                        iNotWaitResultMsg = 2;
                                        g_insLogMng.normal("Find the sending duty(%s) is canceled, give up it.",
                                                        insSendingDuty.getPrintInfo().c_str());
                                        break;
                                }

                                g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode3,
                                        "Begin to send the msg data stream block");
                                if( 0 !=  insClientSocket.sendMsg(sMsgBuff, iSendLen, iFileTimeout))
                                {
                                        g_insLogMng.error("Send the duty(%s) data block failed.",
                                                        insSendingDuty.getPrintInfo().c_str());
                                         iNotWaitResultMsg = 3;
                                        break;
                                }
                                //CONTINUEÿ����һ��Ҫ��ȡȷ����Ϣ����֤��Ϣ������,��ʱ�����öϵ������ķ�ʽ
                                else
                                {
                                        if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                                        {
                                                ResponseCode eResponseCode;
                                                if (0 != recvResponse(insClientSocket, eResponseCode))
                                                {
                                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg data block to socket,but recv response timeout.",
                                                                insSendingDuty.getPrintInfo().c_str());
                                                        iNotWaitResultMsg = 4;
                                                        eLastResponseCode = ServicePause;
                                                        break;
                                                }
                                                if (CancelDuty == eResponseCode)
                                                {
                                                        eLastResponseCode = Success;
                                                        g_insLogMng.normal("Send Duty(%s)'s FileUploadMsg data block to socket, recv CancelDuty responecode.",
                                                                insSendingDuty.getPrintInfo().c_str());
                                                         iNotWaitResultMsg = 5;
                                                        break;
                                                }
                                                else if (Success == eResponseCode)
                                                {
                                                        eLastResponseCode = Success;
                                                        //����
                                                }
                                                else //ʧ��
                                                {
                                                        iNotWaitResultMsg = 6;
                                                        eLastResponseCode = eResponseCode;
                                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg data block to socket,recv %x responecode.",
                                                                insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                                        break;
                                                }
                                        }
                                }
                                iHadSendLen = iHadSendLen + iSendLen;
                                iReadLen = iReadLen - iSendLen;

                                //������������
                                if (0 != iCtrlTime)
                                {
                                        g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode2,
                                                "Send msg data need control flow, sleep(CtrlTime = %d ms)", iCtrlTime);
                                        if (iCtrlTime >= 1000)  //��������
                                        {
                                                int iTmpCtrlTime = iCtrlTime/1000;
                                                CComFun::sleep_msec(iTmpCtrlTime, 0);
                                        }
                                        else
                                        {
                                                CComFun::sleep_msec(0, iCtrlTime);  //��Ϣ��Ӧ�ĺ�����
                                        }
                                }
                        }
                        if ( 0 != iNotWaitResultMsg)
                        {
                                break;
                        }
                }
                fclose(fp);
                break;
        }
        case ProtocolDelFile:
        {
                //ƴװ��������Ϣ��info��Ϣ
                CFileDelMsg insFileDelMsg;
                strcpy(insFileDelMsg.m_sSiteName , insSendingDuty.m_sSiteName);
                strcpy(insFileDelMsg.m_sDestPathFile, insSendingDuty.m_sDestPathFile);
                strcpy(insFileDelMsg.m_sSendedTime, insSendingDuty.m_sSendedTime);
                g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode3,  "Begin to send the info head");
                //�ж�һ�������Ƿ�ȡ��,����ȡ������������ν����Ϊ�Զ˼�ʹ����ʧ�ܣ�Ҳ�����ٽ��к���������
                if (0 != g_insQueueMng.judgeNormal((int)pthread_self(), CQueueMng::SendingQueue))
                {
                        iNotWaitResultMsg = 2;
                        g_insLogMng.normal("Find the sending duty(%s) is canceled, give up it.",
                                        insSendingDuty.getPrintInfo().c_str());
                        break;
                }
                if (0 != insClientSocket.sendMsg((char*)(&insFileDelMsg), sizeof(CFileDelMsg), iMsgTimeout))
                {
                        g_insLogMng.error("Write ProtocolDelFile to socket failed when sending duty(%s).",
                               insSendingDuty.getPrintInfo().c_str());
                        break;
                }
                //CONTINUEÿ����һ��Ҫ��ȡȷ����Ϣ����֤��Ϣ������,��ʱ�����öϵ������ķ�ʽ
                else
                {
                        if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                        {
                                ResponseCode eResponseCode;
                                if (0 != recvResponse(insClientSocket, eResponseCode))
                                {
                                        g_insLogMng.error("Send Duty(%s)'s FileDelMsg to socket,but recv response timeout.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        eLastResponseCode = ServicePause;
                                        break;
                                }
                                if (CancelDuty == eResponseCode)
                                {
                                        eLastResponseCode = Success;
                                        g_insLogMng.normal("Send Duty(%s)'s FileDelMsg to socket,recv CancelDuty responecode.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        break;
                                }
                                else if (Success == eResponseCode)
                                {
                                        eLastResponseCode = Success;
                                        //����
                                }
                                else //ʧ��
                                {
                                        eLastResponseCode = eResponseCode;
                                        g_insLogMng.error("Send Duty(%s)'s FileDelMsg to socket,recv %x responecode.",
                                                                insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                        break;
                                }
                        }
                }
                break;  //����delete
        }
        //BEGIN kevinliu add 2006-04-21
        case ProtocolMvFile:  // mv �� copy��Э����һ����,���Է���ʱ����ͳһ����
        case ProtocolCpFile:
        {
                //ƴװ��������Ϣ��info��Ϣ
                CFileMvCpMsg insFileMvCpMsg;
                strcpy(insFileMvCpMsg.m_sSiteName , insSendingDuty.m_sSiteName);
                strcpy(insFileMvCpMsg.m_sDestPathFile, insSendingDuty.m_sDestPathFile);
                strcpy(insFileMvCpMsg.m_sSrcPathFile, insSendingDuty.m_sLocalPathFile);
                strcpy(insFileMvCpMsg.m_sSendedTime, insSendingDuty.m_sSendedTime);
                g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode3,  "Begin to send the info head");
                //�ж�һ�������Ƿ�ȡ��,����ȡ������������ν����Ϊ�Զ˼�ʹ����ʧ�ܣ�Ҳ�����ٽ��к���������
                if (0 != g_insQueueMng.judgeNormal((int)pthread_self(), CQueueMng::SendingQueue))
                {
                        iNotWaitResultMsg = 2;
                        g_insLogMng.normal("Find the sending duty(%s) is canceled, give up it.",
                                        insSendingDuty.getPrintInfo().c_str());
                        break;
                }
                if (0 != insClientSocket.sendMsg((char*)(&insFileMvCpMsg), sizeof(CFileMvCpMsg), iMsgTimeout))
                {
                        g_insLogMng.error("Write CFileMvCpMsg to socket failed when sending duty(%s).",
                               insSendingDuty.getPrintInfo().c_str());
                        break;
                }
                //CONTINUEÿ����һ��Ҫ��ȡȷ����Ϣ����֤��Ϣ������,��ʱ�����öϵ������ķ�ʽ
                else
                {
                        if (1 == g_insConfigMng.m_tComCfg.m_iProtocolType)
                        {
                                ResponseCode eResponseCode;
                                if (0 != recvResponse(insClientSocket, eResponseCode))
                                {
                                        g_insLogMng.error("Send Duty(%s)'s CFileMvCpMsg to socket,but recv response timeout.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        eLastResponseCode = ServicePause;
                                        break;
                                }
                                if (CancelDuty == eResponseCode)
                                {
                                        eLastResponseCode = Success;
                                        g_insLogMng.normal("Send Duty(%s)'s CFileMvCpMsg to socket,recv CancelDuty responecode.",
                                                insSendingDuty.getPrintInfo().c_str());
                                        break;
                                }
                                else if (Success == eResponseCode)
                                {
                                        eLastResponseCode = Success;
                                        //����
                                }
                                else //ʧ��
                                {
                                        eLastResponseCode = eResponseCode;
                                        g_insLogMng.error("Send Duty(%s)'s CFileMvCpMsg to socket,recv %x responecode.",
                                                                insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                        break;
                                }
                        }
                }
                break;
        }
        //END kevinliu add 2006-04-21
        default:
        {
                eLastResponseCode = InfoFail;
                break;
        }
        }

        //���������޸�һ��״̬
        g_insQueueMng.setOver((int)pthread_self(), CQueueMng::SendingQueue);

        // 1 ��ʾ��Ŀ¼����open�����ļ�ʧ�ܣ���ʱ������������ļ������ڣ�Ҳ�����Ǳ�ɾ�����Բ�����
        if (1 == iNotWaitResultMsg)
        {
                g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode1, "END_failed_end<<%s>>",
                                 insSendingDuty.getPrintInfo().c_str());
                insClientSocket.closeSocket();
                return;
        }
        // 2 ����ȡ��
        else if (2 == iNotWaitResultMsg)
        {
                g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode1, "END_suc_cancel<<%s>>",
                                 insSendingDuty.getPrintInfo().c_str());
                insClientSocket.closeSocket();
                return;
        } //else  0�ɹ��� ������������ɻ��߲���Ԥ֪�Ĵ���,�����ǶԷ����£����Խ��ܽ����Ϣ

        //���������ȷ��Э��ʱ,��Ҫ������һ����Ϣ
        if (0 == g_insConfigMng.m_tComCfg.m_iProtocolType)
        {
                ResponseCode eResponseCode;
                if (0 != recvResponse(insClientSocket, eResponseCode))
                {
                        g_insLogMng.error("Send Duty(%s) recv response timeout.",
                                insSendingDuty.getPrintInfo().c_str());
                        eLastResponseCode = UnknownCode;
                }
                else
                {
                        if (CancelDuty == eResponseCode)
                        {
                                eLastResponseCode = Success;
                                g_insLogMng.normal("Send Duty(%s) recv CancelDuty responecode.",
                                        insSendingDuty.getPrintInfo().c_str());
                        }
                        else if (Success == eResponseCode)
                        {
                                eLastResponseCode = Success;
                        }
                        else //ʧ��
                        {
                                eLastResponseCode = eResponseCode;
                                g_insLogMng.error("Send Duty(%s) recv %x responecode.",
                                                        insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                        }
                }
        }

        //������Ӧ��Ϣ
        if (Success == eLastResponseCode )
        {
                 g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode1, "END_suc<<%s>>",
                         insSendingDuty.getPrintInfo().c_str());
                //��Ҫ���룬����ɹ�����
                g_insQueueMng.putDuty2LastSucQ(insSendingDuty);

                //������������Ҫɾ���ļ���ע���ʱֻ�ܴ���һ���ַ�����ʱʹ�ã��������ֺ����صĴ���
                if (ProtocolUploadFile == insSendingDuty.m_iProtocolType && 1 == g_insConfigMng.m_tComCfg.m_iDelFileAfterUpload)
                {
                         g_insLogMng.normal("Finished Duty(%s) and delete file.", insSendingDuty.getPrintInfo().c_str());
                        CComFun::delFile(insSendingDuty.m_sLocalPathFile);
                }
        }
        else
        {
                 g_insLogMng.error("END_fail<<%s;Reponsecode=%d>>",
                         insSendingDuty.getPrintInfo().c_str(), eLastResponseCode);
                 //���·������
                 insSendingDuty.m_iNrOfFail++;
                 g_insQueueMng.putDuty2SendQ(insSendingDuty, CQueueMng::Direct);
        }
        insClientSocket.closeSocket();
        return;

}


int CProtocolMng::checkServerStatus(string sIP, int iPort, int iTimeOut)
{        
        int iMsgTimeout;
        if (0 >= iTimeOut)
        {                
                iMsgTimeout = 2*g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout;  //��Ϣ��ʱʱ��
        }
        else 
        {
                iMsgTimeout = g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout/iTimeOut;  //ָ����ʱ��
                if (iMsgTimeout <= 0)
                {
                        iMsgTimeout = 2;  //������Ϸ���Ĭ��Ϊ2��
                }
        }
        CTCPSocket  insClientSocket(sIP.c_str(), iPort);
        if ( 0 != insClientSocket.connectServer())
        {
                g_insLogMng.debug("protocol", CLogMng::LogMode2, "Connect to server(%s %d) failed when checkServerStaus.",
                        sIP.c_str(), iPort);
                return -1;
        }

        //������Ϣͷ��Ϣ
        char sMsgBuff[2];
        sMsgBuff[0] = Version1;
        sMsgBuff[1] = ProtocolCheckStatus;

        if (0 != insClientSocket.sendMsg(sMsgBuff, MsgHeadLen, iMsgTimeout))  //  send 1.1
        {
                 g_insLogMng.error("Write check status msg data to socket(%s %d) failed.",
                        sIP.c_str(), iPort);
                 insClientSocket.closeSocket();
                return -1;
        }

        //������Ӧ��Ϣ
        char sResponseCode;
        char sTmpStr[1024];
        if (0 == insClientSocket.receiveMsg(sTmpStr, 1, iMsgTimeout))  //Ϊ��ȷ��һ�³ɹ�,�ȴ�ʱ����2��
        {
                sResponseCode = sTmpStr[0];
                if (CProtocolMng::Success == sResponseCode || CProtocolMng::checkOK == sResponseCode)
                {
                         insClientSocket.closeSocket();
                         return 0;
                }
                else
                {
                         g_insLogMng.error("Ping (%s %d) failed, because recving the failed(%02x) reponse",
                                 sIP.c_str(), iPort, sResponseCode);
                         insClientSocket.closeSocket();
                         return -1;
                }
        }
        else
        {
                g_insLogMng.error("Ping (%s %d) failed, because recving the response msg failed.",
                               sIP.c_str(), iPort);
                insClientSocket.closeSocket();
                return -1;
        }

}

int CProtocolMng::sendResponse(CTCPSocket&  insSocket, ResponseCode resp)
{
        //������Ӧ��Ϣ
        char sRCode[2];
        sRCode[0] = resp;
        g_insLogMng.debug("protocol", CLogMng::LogMode2,  "Send the response msg.");
        return insSocket.sendMsg(sRCode, 1, g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout);
}

int CProtocolMng::recvResponse(CTCPSocket&  insSocket, ResponseCode& resp)
{
        //������Ӧ��Ϣ
        char sTmpStrx[1024];
        g_insLogMng.debug("protocol", CLogMng::LogMode2,  "Recv the response msg.");
        int iRet = insSocket.receiveMsg(sTmpStrx, 1, g_insConfigMng.m_tComCfg.m_iRecvFileTimeout);
        if (0 == iRet)  //Ϊ��ȷ��һ�³ɹ�, �����ý����ļ���ʱ��
        {
                resp = (ResponseCode)sTmpStrx[0];
        }
        return iRet;
}

//kevinliu add for 2.1�汾 - 2007-04-29
 void CProtocolMng::processVersion2(CTCPSocket & insTcpServer, char cProtocolType)
{
        //ͳһ����,������ɺ�,Ȼ���ٸ��ݲ�ͬ������,����,Ȼ��ͳһ����
        vector<string> vecRecvContent;  //�õ���Ϣ��Ϣ�洢
        vector<string> vecSendContent; //���ڷ����õ���Ϣ
         
        g_insLogMng.normal("Manager: recv a version2 message, prtocolType = %d.",
                cProtocolType);
         
        //(1)����
        if ( 0 != recvVersion2Msg(insTcpServer, vecRecvContent))
        {
                return;
        }
        
        //(2)������Ϣ����,���д���
        switch(cProtocolType)
        {
                case ProtocolSearchTree:  
                {
                        //��Ϣ���ݣ���һ��վ��ֵ
                        if (vecRecvContent.size() < 1)
                        {
                                g_insLogMng.error("ProtocolSearchTree: no content.");
                                return;
                        }
                        string sSitename = vecRecvContent[0];
                        //�������ļ��У����ݸ�ʽ�����ض�Ӧ�ķ������б�
                        map<string, CSiteInfo> insMapSiteTmp;
                        map<string, CSiteInfo> insMapSite;
                        g_insConfigMng.getSiteMap(insMapSiteTmp);     
                        string sTmpResult = "";
                        //����վ������
                        if (sSitename.size() == 0)
                        {
                                insMapSite = insMapSiteTmp;                                
                        }
                        else
                        {
                                //��Ҫ�ж����վ���Ƿ����
                                if (insMapSiteTmp.end() == insMapSiteTmp.find(sSitename))  //������
                                {
                                        sTmpResult = sSitename + ":null"; //ֱ���������ֵ������Ĳ�������
                                }
                                else
                                {
                                        insMapSite[sSitename] = insMapSiteTmp[sSitename]; //ֻҪ��һ��վ���
                                }
                        }
                        
                        map<string, CSiteInfo>::iterator it = insMapSite.begin();
                        for(; it != insMapSite.end(); it++)
                        {
                                if (sTmpResult != "")  //�ǵ�һ��,Ҫ��;
                                {
                                         sTmpResult += ";";  //վ����д��
                                }
                                sTmpResult += it->first;  //վ����д��                                
                                //���û�еģ�����"none" ��ʶ
                                if ( it->second.m_vecServerList.size() == 0)
                                {
                                        sTmpResult += string(":none"); 
                                }
                                else
                                {
                                        for(unsigned int i = 0; i < it->second.m_vecServerList.size(); i++)
                                        {
                                               char sTmpP[32];
                                               sprintf(sTmpP, "%d", it->second.m_vecServerList[i].m_iPort);
                                               sTmpResult += string(":") + it->second.m_vecServerList[i].m_sIP + "_" + sTmpP;
                                        }
                                }
                        }
                        //���ý����Ϣ
                        setVersion2Resp(vecSendContent, "0", "successfully",  sTmpResult);
                        break;
                }
                case ProtocolCommonCfg:
                {
                        if (vecRecvContent.size() < 1)
                        {
                                g_insLogMng.error("ProtocolCommonCfg: no content.");
                                return;
                        }
                        int iType = atoi(vecRecvContent[0].c_str());  // 0 �鿴, 1�ύ�޸�
                        string sCommonCfgName = g_sHomeDir + "/cfg/common.cfg";
                        if (iType == 0)
                        {
                                //��ȡĿ¼�µ������ļ�
                                string sTmpResult;
                                if (0 != CComFun::readWholeFileContent(sCommonCfgName, sTmpResult))
                                {
                                        setVersion2Resp(vecSendContent, "1", string ("read failed,") + sCommonCfgName,  "");
                                        break;
                                }
                                setVersion2Resp(vecSendContent, "0", "successfully",  sTmpResult);
                        }
                        else if (iType == 1)  // ����
                        {
                                string sTmpResult = vecRecvContent[1];
                                //�ȱ���һ��ԭ���������ļ�
                                CComFun::cpFile(sCommonCfgName, sCommonCfgName + ".bak"); //����
                                //���������ļ�
                                int iRet = CComFun::writeWholeFileContent(sCommonCfgName, sTmpResult);
                                if (0 != iRet)
                                {
                                        g_insLogMng.error("writeWholeFileContent content(%s) to file(%s) failed, code=%d. %s", 
                                               sTmpResult.c_str(), sCommonCfgName.c_str(), iRet, strerror(errno));
                                        setVersion2Resp(vecSendContent, "1", 
                                                "writeWholeFileContent failed, please refer log info.",  "");
                                         CComFun::cpFile(sCommonCfgName + ".bak", sCommonCfgName); 
                                         break;
                                }
                                //ˢ�µ��ڴ�
                                if (0 != g_insMaintain.refreshCommonCfg())
                                {
                                         setVersion2Resp(vecSendContent, "1", 
                                                "refreshCommonCfg failed, please refer log info.",  "");
                                         CComFun::cpFile(sCommonCfgName + ".bak", sCommonCfgName); 
                                         break;
                                }
                                setVersion2Resp(vecSendContent, "0", "successfully",  "");
                        }
                        break;
                }
                case ProtocolSiteCfg:
                {
                        if (vecRecvContent.size() < 1)
                        {
                                g_insLogMng.error("ProtocolSiteCfg: no content.");
                                return;
                        }
                        // 0 �鿴, 1�ύ�޸�,2��¡վ��������Ϣ
                        int iType = atoi(vecRecvContent[0].c_str());  
                        string sSiteCfgName = g_sHomeDir + "/cfg/siteinfo.cfg";
                        if (iType == 0)
                        {
                                //��ȡĿ¼�µ������ļ�
                                string sTmpResult;
                                if (0 != CComFun::readWholeFileContent(sSiteCfgName, sTmpResult))
                                {
                                        setVersion2Resp(vecSendContent, "1", string ("read failed,") + sSiteCfgName,  "");
                                        break;
                                }
                                setVersion2Resp(vecSendContent, "0", "successfully",  sTmpResult);
                        }
                        else if (iType == 1)  // ����
                        {
                                string sTmpResult = vecRecvContent[1];
                                //�ȱ���һ��ԭ���������ļ�
                                CComFun::cpFile(sSiteCfgName, sSiteCfgName + ".bak"); //����
                                //���������ļ�
                                if (0 != CComFun::writeWholeFileContent(sSiteCfgName, sTmpResult))
                                {
                                        setVersion2Resp(vecSendContent, "1", 
                                                "writeWholeFileContent failed, please refer log info.",  "");
                                         CComFun::cpFile(sSiteCfgName + ".bak", sSiteCfgName); 
                                         break;
                                }
                                //ˢ�µ��ڴ�
                                if (0 != g_insMaintain.refreshSiteInfo())
                                {
                                         setVersion2Resp(vecSendContent, "1", 
                                                "refreshSiteInfo failed, please refer log info.",  "");
                                         CComFun::cpFile(sSiteCfgName + ".bak", sSiteCfgName); 
                                         break;
                                }
                                setVersion2Resp(vecSendContent, "0", "successfully",  "");
                        }
                        else if (iType == 2)
                        {
                                string sBySiteName = vecRecvContent[1];
                                string sAddSiteName = vecRecvContent[2];                                
                                if (0 != cloneSiteCfg(sBySiteName, sAddSiteName))
                                {
                                         setVersion2Resp(vecSendContent, "1", 
                                                "cloneSiteCfg failed, please refer log info.",  "");
                                         break;
                                }
                                //�ٽ���Ϣת��������ȥ(���жϳɹ����)
                                forwardVersion2Msg(cProtocolType, vecRecvContent, sBySiteName);
                                setVersion2Resp(vecSendContent, "0", "successfully",  "");
                        }
                        break;
                }
                case ProtocolMobileCfg:
                {
                        if (vecRecvContent.size() < 1)
                        {
                                g_insLogMng.error("ProtocolMobileCfg: no content.");
                                return;
                        }
                        int iType = atoi(vecRecvContent[0].c_str());  // 0 �鿴, 1�ύ�޸�
                        string sMobilecfgName = g_sHomeDir + "/cfg/mobilelist.cfg";
                        if (iType == 0)
                        {
                                //��ȡĿ¼�µ������ļ�
                                string sTmpResult;
                                if (0 != CComFun::readWholeFileContent(sMobilecfgName, sTmpResult))
                                {
                                        setVersion2Resp(vecSendContent, "1", string ("read failed,") + sMobilecfgName,  "");
                                        break;
                                }
                                setVersion2Resp(vecSendContent, "0", "successfully",  sTmpResult);
                        }
                        else if (iType == 1)  // ���²�������Ϣ�����ݵ�����������еķַ�������
                        {
                                string sTmpResult = vecRecvContent[1];
                                //�ȱ���һ��ԭ���������ļ�
                                CComFun::cpFile(sMobilecfgName, sMobilecfgName + ".bak"); //����
                               //���������ļ�
                                if (0 != CComFun::writeWholeFileContent(sMobilecfgName, sTmpResult))
                                {
                                        setVersion2Resp(vecSendContent, "1", 
                                                "writeWholeFileContent failed, please refer log info.",  "");
                                         CComFun::cpFile(sMobilecfgName + ".bak", sMobilecfgName); 
                                         break;
                                }
                                //����Ϣת����ȥ����������վ��
                                if(1 == atoi(vecRecvContent[2].c_str())) //��Ҫ֪ͨ����
                                {
                                        forwardVersion2Msg(cProtocolType, vecRecvContent, "");
                                }
                                setVersion2Resp(vecSendContent, "0", "successfully",  "");
                        }
                        break;
                }
                case ProtocolLogTrace:
                {
                        if (vecRecvContent.size() < 1)
                        {
                                g_insLogMng.error("ProtocolMobileCfg: no content.");
                                return;
                        }
                        int iType = atoi(vecRecvContent[0].c_str());  // 0 �鿴, 1�ύ�޸�
                        string sLogName= g_sHomeDir + "/log/newoctopusd.";
                        if (iType == 0)
                        {
                                sLogName += "log";
                        }
                        else if (iType == 1)
                        {
                                sLogName += "error";
                        }
                        else if (iType == 2) // 2
                        {
                                 sLogName += "run";
                        }
                        else if (iType == 3)
                        {
                                string sTmpDate = "";
                                if(0 == vecRecvContent[1].size())   //�Ƿ�ָ��������
                                {
                                        sTmpDate = CComFun::GetCurDate();//Ĭ��Ϊ��ǰ����
                                }
                                else
                                {
                                        sTmpDate = vecRecvContent[1]; //ָ��������
                                }
                                sLogName = g_sHomeDir + "/log/stat/queue"+ sTmpDate +".xml";
                        }
                        else  if (iType == 4)//ֱ�Ӷ��ļ����߼�����鲻��У�飬ֱ����ǰ�����б�֤
                        {
                                //������Ŀ¼�µ��κ��ļ�
                                sLogName= g_sHomeDir + vecRecvContent[1];
                        }
                        //��ȡĿ¼�µ��ļ�
                        string sTmpResult = "none";  //���Ϊ���ֵ,˵��û��ȡ��
                        CComFun::readWholeFileContent(sLogName, sTmpResult); //�Ƿ��ܶ��������ж�
                        if (iType == 3) //�����xml,����xml����
                        {
                                sTmpResult = string("<stat>\n") + sTmpResult + "</stat>\n";
                        }
                        setVersion2Resp(vecSendContent, "0", "successfully",  sTmpResult);
                        break;
                }
                 case ProtocolShowQInfo:
                {
                        if (vecRecvContent.size() < 1)
                        {
                                g_insLogMng.error("ProtocolMobileCfg: no content.");
                                return;
                        }
                        int iType = atoi(vecRecvContent[0].c_str());  // 0 �鿴������Ϣ, 1 �鿴��������ʱΪʵ��
                        string sTmpResult = "";
                        if (iType == 0)
                        {
                                //��ȡ����
                                g_insQueueMng.getCurInfo(sTmpResult);
                                setVersion2Resp(vecSendContent, "0", "successfully",  sTmpResult);
                        }
                        else if (iType == 1)
                        {
                                //g_insMaintain.reloadFailDuty(-1); //ȫ��
                                 g_insMaintain.setFlag(CMaintain::ReloadFailDuty, -1);
                                setVersion2Resp(vecSendContent, "0", "successfully",  "");
                        }
                        else if (iType == 2)
                        {
                                g_insQueueMng.clearErrorIPPort(""); //ȫ��
                                setVersion2Resp(vecSendContent, "0", "successfully",  "");
                        }
                        else
                        {
                                setVersion2Resp(vecSendContent, "1", "unkown type.",  "");  //ʧ��
                        }
                        break;
                }
                default:
                {
                        char sTmpPrint[64];
                        sprintf(sTmpPrint, "unkown msgtype(%d)", cProtocolType);
                        setVersion2Resp(vecSendContent, "1", sTmpPrint,  "");
                        break;
                }
        }       

        //(3)���� (socket�Ļ������ϲ�)
        sendVersion2Msg(insTcpServer, cProtocolType, vecSendContent);
        
         g_insLogMng.normal("Manager: finished a version2 message, prtocolType = %d.",
                cProtocolType);
         
        return;
}

void CProtocolMng::setVersion2Resp(vector<string>& vecmsgContent, const string& sCode, 
        const string& sDesc, const string& value)
{
        vecmsgContent.clear();
        vecmsgContent.push_back(sCode);
        vecmsgContent.push_back(sDesc);
        if (sCode != "0")
        {
                g_insLogMng.error("%s.", sDesc.c_str());
                return;
        }
        if (value.size() == 0)
        {
                return;
        }
        else
        {
                vecmsgContent.push_back(value);
        }
        return;
}

int CProtocolMng::recvVersion2Msg(CTCPSocket & insTcpServer, vector<string>& vecRecvContent)
{
        int iTotalLen = 0;
        if (0 !=insTcpServer.receiveMsg((char*)&iTotalLen, sizeof(int),
                        g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout))     
        {
                g_insLogMng.error("recvVersion2Msg:receiveMsg Len failed.");
                return -1;
        }
        iTotalLen = ntohl(iTotalLen) -6; //ȥ����Ϣͷ��6���ֽ�
        //Ϊ�˷�ֹ�Է�������Ϣ������ʱ����Ϣ��λ4*65535�Ĵ�С,��Ҫʱ����չ
        if (iTotalLen > 4* 65535 || iTotalLen <= 0)
        {
                g_insLogMng.error("recvVersion2Msg:len is invalid, %d > 4* 65535 or < 0.", iTotalLen);
                return -1;
        }
        
        //Ȼ����������е�����
        char *pBuffer = new char[iTotalLen];
        if (0 !=insTcpServer.receiveMsg(pBuffer, iTotalLen,
                        g_insConfigMng.m_tComCfg.m_iRecvFileTimeout))
        {
                g_insLogMng.error("recvVersion2Msg:receiveMsg body failed, len = %d.", iTotalLen);
                delete[] pBuffer;
                return -1;
        }
         g_insLogMng.debug("protocol", CLogMng::LogMode3, "MsgLen = %d, MSGCODE:", iTotalLen);
         g_insLogMng.bin("protocol", CLogMng::LogMode3, pBuffer, iTotalLen);        

        //�����ݽ��н���       
        int iCurLen = 0;
        //cType ���������ź�
        while(iCurLen < iTotalLen)
        {
                //Type
                char cType;
                CComFun::msg_getChar(pBuffer, iCurLen, cType);
                iCurLen += sizeof(char);
                //������մ�����˴���
                if ((unsigned int)cType != vecRecvContent.size())  // ���յ�ʱ��,���Ӧ���ǵ��ڵ�ʱ�ĳ��ȵ�
                {
                        g_insLogMng.error("recvVersion2Msg:TLV's type is %d != %d, invalid, break decode.", cType, vecRecvContent.size());
                        delete[] pBuffer;
                        return -1;
                }
                
                //Length
                int iLength;
                CComFun::msg_getInt(pBuffer, iCurLen, iLength);
                iCurLen += sizeof(int);
                //����ֵ���Ϸ�
                if (iLength < 0)
                {
                       g_insLogMng.error("TLV's length is %d < 0, invalid, break decode.", iLength);
                        delete[] pBuffer;
                        return -1;
                }

                //Value
                string sValue;
                CComFun::msg_getString(pBuffer, iCurLen, sValue, iLength);
                iCurLen += iLength;

                //�洢����Ϣ������
                vecRecvContent .push_back(sValue);

                g_insLogMng.debug("protocol", CLogMng::LogMode1, "%s: Type=%d ,Value = %s",  "decode", cType, sValue.c_str());
        }

        //ɾ������Ϣ�Ŀռ�
        delete[] pBuffer;
        pBuffer = NULL;

        return 0;

}

int CProtocolMng::sendVersion2Msg(CTCPSocket & insTcpServer, char cMsgType, vector<string>& vecSendContent)
{
        g_insLogMng.debug("protocol", CLogMng::LogMode3, "Begin to Send a version2 msg.");
        
        // �ȼ���һ����Ҫ���Ŀռ�
        int iContentLen = 0;

        unsigned int iLoop;
        for(iLoop = 0; iLoop < vecSendContent.size(); iLoop++)
        {
                iContentLen += vecSendContent[iLoop].size();
        }

        int iTotalLen = 6 + vecSendContent.size() * (sizeof(char) + sizeof(int))  + iContentLen;

        //����ռ�
        char* pMsgBuff = new char[iTotalLen];
        if (NULL == pMsgBuff)
        {
                return -1;
        }

        //�����ݴ��
        int iCurLen = 0;

        CComFun::msg_putChar(pMsgBuff, Version2, iCurLen);
        iCurLen += sizeof(char);

        CComFun::msg_putChar(pMsgBuff, cMsgType, iCurLen);
        iCurLen += sizeof(char);

        CComFun::msg_putInt(pMsgBuff, iTotalLen, iCurLen);
        iCurLen += sizeof(int);

        for(iLoop = 0; iLoop < vecSendContent.size(); iLoop++)
        {
                g_insLogMng.debug("protocol", CLogMng::LogMode1, "%s: Type=%d , Value = %s",  "encode",  iLoop, vecSendContent[iLoop].c_str());
                CComFun::msg_putChar(pMsgBuff, iLoop, iCurLen);
                iCurLen += sizeof(char);

                int iTmpLen = vecSendContent[iLoop].size();
                CComFun::msg_putInt(pMsgBuff, iTmpLen, iCurLen);
                iCurLen += sizeof(int);

                CComFun::msg_putString(pMsgBuff, vecSendContent[iLoop].c_str(), iTmpLen, iCurLen);
                iCurLen += iTmpLen;
        }

        //ʵ�ʵķ�����Ϣ
        if (0 != insTcpServer.sendMsg(pMsgBuff, iTotalLen, g_insConfigMng.m_tComCfg.m_iRecvFileTimeout))
        {
                g_insLogMng.error("2 Send Response Msg failed.");
                delete[]  pMsgBuff;
                return -1;
        }
        
        delete[]  pMsgBuff;
        return 0; 
}

//����Ϣת��������ķַ���������sitenameΪ�գ���ʾ���еķ�������Ҫת����Ϣ����Ϊ�գ����ʾֻ����ڵ÷ַ�����������Ϣ
int CProtocolMng::forwardVersion2Msg(char cMsgType, vector<string>& vecmsgContent, const string& sitename)
{
        //��÷������б�
        map<string, CSiteInfo> insMapSite;
        map<string, CServerInfo> insServerList; //�������б����û�����������������Ҳ�ܱ�֤����������        
        g_insConfigMng.getSiteMap(insMapSite);   
        if (sitename.size() == 0)
        {
                //��ȡ����վ���µķ������б�
                map<string, CSiteInfo>::iterator it = insMapSite.begin();
                for(; it != insMapSite.end(); it++)
                {
                        vector<CServerInfo>::iterator it1 = (it->second.m_vecServerList).begin();
                        for(; it1 != (it->second.m_vecServerList).end(); it1++)
                        {
                                string sKey =(*it1).m_sName; //����������Ψһ
                                insServerList[sKey] = (*it1);  //��ҪȡΨһ�ķ������б�
                        }
                }
        }
        else
        {
                //�ж�վ���Ƿ����
                map<string, CSiteInfo>::iterator it = insMapSite.find(sitename);
                if (insMapSite.end() != it)  //����
                {
                        //��һվ��ķ������б�
                        vector<CServerInfo>::iterator it1 = (it->second.m_vecServerList).begin();
                        for(; it1 != (it->second.m_vecServerList).end(); it1++)
                        {
                                string sKey =(*it1).m_sName; //����������Ψһ
                                insServerList[sKey] = (*it1);  //��ҪȡΨһ�ķ������б�
                        }
                }                
        }
        insMapSite.clear();

        //��ʼ������Ϣ,�������֤��ȫ,ֻ����,��������Ӧ,
        //��Ϊ���ڶ�㼶��,�޷���֤,�����Ƿ�ɹ�,����־�Ͳ쿴����,������ֻ�ڶԶ���־����ʾ
        map<string, CServerInfo>::iterator it2 = insServerList.begin();
        for(; it2!=insServerList.end(); it2++)
        {
                //��Server��������
                CTCPSocket objClientSocket;
                objClientSocket.init(it2->second.m_sIP,  it2->second.m_iPort);
                if (0 != objClientSocket.connectServer())
                {
                        g_insLogMng.error("Connect server (%s, %d) failed,continue next.\n", 
                                it2->second.m_sIP,  it2->second.m_iPort);
                        continue;
                }

                //���жϳɹ����,Ҳ�����ܽṹ��Ϣ,ֱ�ӽ���
                sendVersion2Msg(objClientSocket, cMsgType, vecmsgContent);
                objClientSocket.closeSocket();
        }
        return 0;        
}

int CProtocolMng::cloneSiteCfg(const string& sBySiteName, const string& sAddSiteName)
{
        //����Ǳ��ο�������Ҫ���ܣ��������κλ����ϣ���������һ��վ�㸴�Ƴ�һ���µ�վ����Ϣ�ַ���Ϣ
        // ���õķ�����,���չ淶�޸ı��������ļ�,Ȼ����ˢ�µķ���;
        // ͬʱ����Ϣ��Ҫ���ݵ������վ��ȥ���Ƿ�ɹ����������û��ڿͻ��˲쿴��֤
        //[SITEINFO]
        //NrOfSite = 46
        // [SITE046]
        //SiteName = search
        //ServerGroup = OtherServerGroup
        //DocumentRoot = /data1/qq_news/htdocs (վ����Ϣ�ȼ��滻)

        //clone�Լ����账��
        if(sBySiteName == sAddSiteName)
        {
                return 0;
        }

        //������д�ļ��ķ�ʽ, ���д������д
        // siteinfo�����ļ���·�������ļ���
        string sSiteInfoCfgName = g_sHomeDir + "/cfg/siteinfo.cfg";
        g_insLogMng.normal("Begin to read %s ......", sSiteInfoCfgName.c_str());

        //��ʼ�������ļ�
        CConfig  insCfgSiteInfo(sSiteInfoCfgName);
        int iRet = insCfgSiteInfo.Init();
        if(iRet != 0)
        {
                g_insLogMng.error("Init %s failed, bad line = %d",
                        sSiteInfoCfgName.c_str(), iRet);
                return -1;
        }
        //
        // [SITEINFO]
        //   NrOfSite = 5
        char tmpstr[1024];
        char sTmpSec[64];
        int iNrOfSite = 0;
        insCfgSiteInfo.ReadItem("SITEINFO","NrOfSite","", tmpstr);
        iNrOfSite= atoi(tmpstr);  //�����жϣ���Ϊϵͳ���������϶��ǺϷ�����
        //��������Ľ����
        string sServerGroup = "";
        string sDocumentRoot= "";
        string sExistServerGroup= ""; //����Ѿ�����,�����¼���ڵ���,�Ա�ˢ��,���򲻴���

        for (int iLoop = 1; iLoop <= iNrOfSite; iLoop++)
        {
                sprintf(sTmpSec, "SITE%03d", iLoop);
                insCfgSiteInfo.ReadItem(sTmpSec, "SiteName","", tmpstr);
                string sSiteName = tmpstr;

                if (sSiteName == sBySiteName)
                {
                         insCfgSiteInfo.ReadItem(sTmpSec, "ServerGroup","", tmpstr);
                         sServerGroup = tmpstr;
                         insCfgSiteInfo.ReadItem(sTmpSec, "DocumentRoot","", tmpstr);
                         sDocumentRoot = tmpstr;
                }
                else if (sSiteName == sAddSiteName)
                {                        
                         insCfgSiteInfo.ReadItem(sTmpSec, "ServerGroup","", tmpstr);
                         sExistServerGroup = tmpstr;
                }
        }

        //�����������Դվ��
        if (sServerGroup == "")
        {
                g_insLogMng.error("source site %s not exist.", sBySiteName.c_str());
                return -1; 
        }
        if (sExistServerGroup == sServerGroup)  //�Ѿ����������޸�
        {
                return 0;
        }
        
        //��Ҫ��������
        CComFun::cpFile(sSiteInfoCfgName, sSiteInfoCfgName + ".bak"); // ����

        string sContent = "";
        string sNewContent = "";
        if (0 != CComFun::readWholeFileContent(sSiteInfoCfgName, sContent))
        {
                g_insLogMng.error("read %s failed", sSiteInfoCfgName.c_str());
                return -1;
        }
        //���뱣֤�ļ��У�ֻ����һ��NrOfSite�ֶ�
        string::size_type pos = sContent.find("NrOfSite", 0);    //�ҵ����,��ǰ��ȫ�̸���
        if (string::npos  == pos)
        {
                 g_insLogMng.error("file %s error, no NrOfSite", sSiteInfoCfgName.c_str());
                return -1;
        }
        sNewContent = sContent.substr(0, pos);  //ǰ�벿��
        if (sNewContent[sNewContent.size() -1] != '\n')
        {
                //�����������ļ�,Ӧ�ö���һ�����еĿ�ʼ (\r\n  or \n)
                g_insLogMng.error("file %s error, NrOfSite pos is error", sSiteInfoCfgName.c_str());
                return -1;    
        }
        
        if (sExistServerGroup.size() == 0)  //����
        {
                sprintf(sTmpSec, "NrOfSite = %d\n", iNrOfSite+1);
                sNewContent += string(sTmpSec);  // ����++
                //��ʼ�������е�վ��
                for (int iLoop = 1; iLoop <= iNrOfSite; iLoop++)
                {
                        sprintf(sTmpSec, "SITE%03d", iLoop);
                        sNewContent += string("\n[") +sTmpSec + "]\n";  //ǰ��Ҳ��һ�У�Ŀ��������
                        insCfgSiteInfo.ReadItem(sTmpSec, "SiteName","", tmpstr);     
                        sNewContent += string("SiteName = ") + tmpstr + "\n"; 
                        insCfgSiteInfo.ReadItem(sTmpSec, "ServerGroup","", tmpstr);
                        sNewContent += string("ServerGroup = ") + tmpstr + "\n"; 
                        insCfgSiteInfo.ReadItem(sTmpSec, "DocumentRoot","", tmpstr);
                        sNewContent += string("DocumentRoot = ") + tmpstr + "\n";
                }
                //����������Ǹ�
                sprintf(sTmpSec, "SITE%03d", iNrOfSite+1);
                sNewContent += string("\n[") +sTmpSec + "]\n";  //ǰ��Ҳ��һ�У�Ŀ��������
                sNewContent += string("SiteName = ") + sAddSiteName + "\n"; 
                sNewContent += string("ServerGroup = ") + sServerGroup + "\n"; 
                //���������,��һ�����˴��ڶ��վ��������Ŀ¼��ô����?
                //�ڴ�����Լ���ˣ����г��ֵĶ��滻��������Խ�������⣬������ͨ���ֹ��޸ĵķ�ʽ                
                //���û�г��֣���ֻ��ֱ�ӷŵ�/usr/local�½�����Ӧ��վ����Ŀ¼
                //�ȿ�����û��
                if(string::npos == sDocumentRoot.find(sBySiteName, 0)) //û��վ����Ϣ����ֻ��ֱ�ӷŵ�/usr/local��
                {
                        sNewContent += string("DocumentRoot = /usr/local/qq_") + sAddSiteName+ "/htdocs\n"; 
                }
                else //����
                {
                        CComFun::find8Replace(sDocumentRoot, sBySiteName, sAddSiteName, 2);
                        sNewContent += string("DocumentRoot = ") + sDocumentRoot+ "\n";   //���Ҫ�������վ���滻��     
                }
        }
        else  // ���� �޸ķַ�����
        {
                sprintf(sTmpSec, "NrOfSite = %d\n", iNrOfSite);
                sNewContent += string(sTmpSec); 
                //��ʼ�������е�վ��
                for (int iLoop = 1; iLoop <= iNrOfSite; iLoop++)
                {
                        sprintf(sTmpSec, "SITE%03d", iLoop);
                        sNewContent += string("\n[") +sTmpSec + "]\n";  //ǰ��Ҳ��һ�У�Ŀ��������
                        insCfgSiteInfo.ReadItem(sTmpSec, "SiteName","", tmpstr);     
                        sNewContent += string("SiteName = ") + tmpstr + "\n"; 
                         if (sAddSiteName != tmpstr)
                        {                        
                                insCfgSiteInfo.ReadItem(sTmpSec, "ServerGroup","", tmpstr);
                                sNewContent += string("ServerGroup = ") + tmpstr + "\n"; 
                        }
                        else //�滻
                        {
                                sNewContent += string("ServerGroup = ") + sServerGroup + "\n"; 
                        }
                        insCfgSiteInfo.ReadItem(sTmpSec, "DocumentRoot","", tmpstr);
                        sNewContent += string("DocumentRoot = ") + tmpstr + "\n";
                }
        }
        
        //������ļ�д��
        if( 0 != CComFun::writeWholeFileContent(sSiteInfoCfgName, sNewContent))
        {
                CComFun::cpFile(sSiteInfoCfgName + ".bak", sSiteInfoCfgName); // ����
                g_insLogMng.error("writeWholeFileContent(%s) failed, ", sSiteInfoCfgName.c_str());
                return -1;
        }

        //ˢ�µ��ڴ�
        if (0 != g_insMaintain.refreshSiteInfo())
        {
                 CComFun::cpFile(sSiteInfoCfgName + ".bak", sSiteInfoCfgName); // ����
                 g_insLogMng.error("writeWholeFileContent(%s) failed, ", sSiteInfoCfgName.c_str());
                 return -1;
        }        
        return 0;
}

