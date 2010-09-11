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
//        协议简单解释
// 采用了以消息块为单位的发送确认机制:
// 也就是说，每发送一个消息块，就必须收到其响应才决定下一步是否进行;
// 单位是消息头一部分,消息流分成多部分
// 同时也支持,一个大的消息,一个
//  *************************************************************************

void CProtocolMng::processRecv(CTCPSocket& insTcpServer)
{
        //开始处理一个新的任务
        g_insLogMng.debug("protocol", CLogMng::LogMode2, "Begin to recv protocol process.");

        //消息收发信息
        int iMsgTimeout = g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout;  //消息超时时间
        int iFileTimeout = g_insConfigMng.m_tComCfg.m_iRecvFileTimeout;  //文件超时时间
        ResponseCode eResponseCode = Success;  //响应码

        //消息头信息
        char pMsgHead[MsgHeadLen + 1];

        //接收缓冲区
        char sMsgBuff[MsgBufferSize + 1];

        //接收消息头信息
        g_insLogMng.debug("protocol", CLogMng::LogMode3, "Begin to recv a msg head info:");
        int iRecvMsgRet;
        iRecvMsgRet = insTcpServer.receiveMsg(pMsgHead, MsgHeadLen, iMsgTimeout);
        if (0 == iRecvMsgRet)
        {
                 g_insLogMng.debug("protocol", CLogMng::LogMode1, "MsgVersion = %d, MsgType = %d.", 
                        pMsgHead[0], pMsgHead[1]);
                 
                //kevinliu add for 2.1版本 - 2007-04-29
                if (pMsgHead[0] == Version2)
                {
                        processVersion2(insTcpServer, pMsgHead[1]); //单独写函数进行处理
                        return;
                }
                else  if (pMsgHead[0] != Version1)  //下面的过程只处理 Version1的
                {
                        g_insLogMng.error("Recv an error msg version(%x), discard this msg.",
                               pMsgHead[0]);
                        return;
                }                
                
                //根据消息类型,进行处理
                switch(pMsgHead[1])
                {
                case ProtocolUploadFile:
                case ProtocolPackFile:  // add by kevinliu 2007-01-10
                {
                        //接收消息信息部分
                        CFileUploadMsg insFileUploadMsg;
                        g_insLogMng.debug("protocol", CLogMng::LogMode3, "Begin to recv an info head.");
                        iRecvMsgRet = insTcpServer.receiveMsg((char*)(&insFileUploadMsg), sizeof(CFileUploadMsg), iFileTimeout);
                        if (0 != iRecvMsgRet) //用了文件时间，即较长点的时间
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
                        }//这个响应加上一块处理，因为担心由于还有后续消息，后续出错可能导致对端写错误 CONTINUE1>>
                        //<<--为了防止,发送过来的消息中字段没有结束符,因此要做char数组的结束操作
                        insFileUploadMsg.sureValidSrt();

                        g_insLogMng.debug("protocol", CLogMng::LogMode3, "End to recv an info head.");

                        insFileUploadMsg.m_iDataLength = ntohl(insFileUploadMsg.m_iDataLength);  //转化网络字节序
                        g_insLogMng.debug(insFileUploadMsg.m_sSiteName, CLogMng::LogMode1, "BEGIN<<%s>>",
                                 insFileUploadMsg.getPrintInfo().c_str());

                        //消息体长度是否有效
                        if (0 > insFileUploadMsg.m_iDataLength)
                        {
                                g_insLogMng.error("FileUploadMsg invalid because the msg body len error.");
                                eResponseCode = InfoFail;
                                break;
                        }

                        //站点是否存在
                        map<string, CSiteInfo> insMapSite;
                        g_insConfigMng.getSiteMap(insMapSite);
                        if (insMapSite.end() == insMapSite.find(insFileUploadMsg.m_sSiteName))
                        {
                                g_insLogMng.normal("FileUploadMsg invalid because site:%s does not exist.",
                                        insFileUploadMsg.m_sSiteName);
                                eResponseCode = InfoFail;
                                break;
                        }

                        //生成正在接收任务
                        CRecvingDuty insRecvingDuty;
                        insRecvingDuty.m_iState = CRecvingDuty::Normal;
                        insRecvingDuty.m_iProtocolType = pMsgHead[1];
                        strcpy(insRecvingDuty.m_sSiteName, insFileUploadMsg.m_sSiteName);
                        strcpy(insRecvingDuty.m_sDestPathFile, insFileUploadMsg.m_sDestPathFile);
                        strcpy(insRecvingDuty.m_sSendedTime, insFileUploadMsg.m_sSendedTime);

                        //检查这个消息是否需要处理，如果需要添加处理
                        //则已经在check中添加到队列中，不需要，则直接退出处理,日志在check里面写
                        if (0 > g_insQueueMng.check(insRecvingDuty))
                        {
                                eResponseCode = CancelDuty; //函数中已经写了日志，不需再写
                                //break; // --PAUSE--为了不使对待写失败导致重发,采用中断任务,收完后不处理的方式
                        }

                        //开始接收，准备生成待发送任务(1.1&2.1第一部分，公共信息)
                        CSendDuty insSendDuty;
                        insSendDuty.m_iState = CSendDuty::Normal;
                        insSendDuty.m_iProtocolType = pMsgHead[1];
                        strcpy(insSendDuty.m_sSiteName, insFileUploadMsg.m_sSiteName);
                        strcpy(insSendDuty.m_sDestPathFile, insFileUploadMsg.m_sDestPathFile);
                        strcpy(insSendDuty.m_sSendedTime, insFileUploadMsg.m_sSendedTime);

                         //判断数据流的类型
                        string sSaveLocalFileName = "";  //重要，用来记录run日志的
                        if (CFileUploadMsg::FileName == insFileUploadMsg.m_cDataType)  //文件名
                        {
                                //超过文件名的最大长度
                                if (insFileUploadMsg.m_iDataLength >= LenOfPathFileName)
                                {
                                         g_insLogMng.error("FileUploadMsg(%s) invalid, Filename too long(%d > %d).",
                                                        insFileUploadMsg.getPrintInfo().c_str(),
                                                        insFileUploadMsg.m_iDataLength ,
                                                        LenOfPathFileName);
                                        eResponseCode = MsgError;
                                        break;
                                }

                                //<<CONTINUE1 每接收一块要确认消息，保证消息包不丢,暂时不采用断点续传的方式
                                //发送位置最好在下次接受之前，或者没有接受消息的操作，这样可以防止对端写错误
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

                                //接收消息失败
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

                                // --PAUSE--为了不使对待写失败导致重发,采用中断任务,收完后不处理的方式
                                if (eResponseCode == CancelDuty)
                                {
                                        break;
                                }

                                //加上结束符号,因为用memset太浪费性能了
                                sMsgBuff[insFileUploadMsg.m_iDataLength] = '\0';
                                CComFun::trimString(sMsgBuff, sMsgBuff, 0);  //过滤空格

                                //准备生成待发送任务(1.2第二部分，文件信息)
                                time((time_t*)&insSendDuty.m_iLocalTime); //当前日期
                                strcpy(insSendDuty.m_sLocalPathFile, sMsgBuff);
                                 g_insLogMng.debug(insFileUploadMsg.m_sSiteName, CLogMng::LogMode1, "%s",
                                        insSendDuty.m_sLocalPathFile);
                                 sSaveLocalFileName = insSendDuty.m_sLocalPathFile;

                                //生成待发送任务并加入队列(1.3第三部分，服务器信息)
                                if (1 == CComFun::isDirectory(insSendDuty.m_sLocalPathFile))
                                {
                                        //目录不再转发,不再回应失败
                                        g_insLogMng.normal("The duty(%s)'s file is a directory, give up forwarding it.",
                                                 insSendDuty.getPrintInfo().c_str());

                                }
                                else  // 正常的文件处理,放入队列
                                {
                                        //判断一下文件是否超过最大限制
                                        struct stat stCheckFStat;
                                        stat(insSendDuty.m_sLocalPathFile, &stCheckFStat); //不判断结果了，因为前面其实已经判断过了
                                        if (0 != g_insConfigMng.m_tComCfg.m_iMaxLimitOfUploadFile) //为0则不限制
                                        {
                                                if (stCheckFStat.st_size >= g_insConfigMng.m_tComCfg.m_iMaxLimitOfUploadFile)
                                                {
                                                        eResponseCode = DataFail;
                                                        g_insLogMng.error("Recv FileUploadMsg(%s) blocked, File is too large(%d over %d bytes).",
                                                                insFileUploadMsg.getPrintInfo().c_str(), stCheckFStat.st_size, g_insConfigMng.m_tComCfg.m_iMaxLimitOfUploadFile);
                                                        break;
                                                }
                                        }
                                
                                        //如果是打包处理的话,这儿做一下校验,只能是那2种类型 kevinliu add 2007-01-10
                                        if (pMsgHead[1] == ProtocolPackFile)
                                        {
                                                Fun_FileType ut = CComFun::getFileType(insSendDuty.m_sLocalPathFile);
                                                if (ut != FILE_TARGZ && ut != FILE_ZIP)
                                                {
                                                        eResponseCode = DataFail; //函数中已经写了日志，不需再写
                                                         g_insLogMng.normal("The duty(%s)'s file is not tar.gz and not zip file, give up forwarding it.",
                                                                 insSendDuty.getPrintInfo().c_str());
                                                        break; 
                                                }
                                        }  // add end                                
                                        if ( 0 != g_insQueueMng.putDuty2SendQ(insSendDuty, CQueueMng::AddServerInfo))
                                        {
                                                eResponseCode = ServicePause; //函数中已经写了日志，不需再写
                                                break;
                                        }
                                }
                                //CONTINUE2<< 的成功消息,用最后的那个消息

                        }
                        else if (CFileUploadMsg::FileData == insFileUploadMsg.m_cDataType)  //数据流
                        {
                                //文件的根目录
                                string sRootPathFile = g_insConfigMng.getSiteRootDir(insFileUploadMsg.m_sSiteName);
                                if("" == sRootPathFile )
                                {
                                        g_insLogMng.error("Analyse FileUploadMsg(%s) failed because getSiteRootDir not find rootdir.",
                                                        insFileUploadMsg.getPrintInfo().c_str());
                                        eResponseCode = InfoFail;
                                        break;
                                }

                                //文件的暂存目录
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

                                //文件的实际目录
                                //生成保存的文件名称和路径(前面已经判断过了，站点到这儿已经存在)
                                string sRealPathFile = CComFun::analysePathFile(sRootPathFile, insFileUploadMsg.m_sDestPathFile, 1);
                                if ("" == sRealPathFile)
                                {
                                        g_insLogMng.error("Analyse FileUploadMsg(%s) failed because desFile(%s,%s) is dir or mkdir error",
                                                        insFileUploadMsg.getPrintInfo().c_str(), sRootPathFile.c_str(), insFileUploadMsg.m_sDestPathFile);
                                        eResponseCode = InfoFail;
                                        break;
                                }

                                //写临时文件
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

                                //写入文件临时缓冲区的变量信息
                                char sWriteFileBuff[WriteToFileSize + 1];
                                unsigned int iWriteFileBuffLen = 0;  //已经写入writefilebuff中的消息长度

                                //消息缓冲区的变量
                                int iLeaveMsgLen = insFileUploadMsg.m_iDataLength;
                                //判断一下文件是否超过最大限制
                                if (0 != g_insConfigMng.m_tComCfg.m_iMaxLimitOfUploadFile) //为0则不限制
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
                                        //每次得需要判断是否已经被取消,取消则不用再处理
                                        if (0 == iWriteLogFlag && 0 != g_insQueueMng.judgeNormal(pthread_self(), CQueueMng::RecvingQueue))
                                        {
                                                eResponseCode = CancelDuty;  //状态被别人改变，则认为成功
                                                iRecvRet = 1;
                                                g_insLogMng.normal( "Find the recving duty(%s) is canceled, give up it but need recv all datas.",
                                                        insFileUploadMsg.getPrintInfo().c_str());
                                                iWriteLogFlag = 1;
                                                //break;   // --PAUSE--为了不使对待写失败导致重发,采用中断任务,收完后不处理的方式
                                        }

                                        //<<CONTINUE 1 or while 每接收一块要确认消息，保证消息包不丢,暂时不采用断点续传的方式
                                        //发送位置最好在下次接受之前，或者没有接受消息的操作，这样可以防止对端写错误
                                        // Continue while end那次不在这
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

                                        //接收
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
                                        } //CONTINUE while>> 确认消息在下一次循环的时候发送，最后处理，用最后那个发送消息

                                        //该写文件
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
                                                iWriteFileBuffLen = 0;  //清 0
                                        }

                                        //拷贝本次接收的字符
                                        memcpy(sWriteFileBuff + iWriteFileBuffLen, sMsgBuff, iNeedRecvMsgLen);
                                        iWriteFileBuffLen = iWriteFileBuffLen + iNeedRecvMsgLen;
                                        iLeaveMsgLen = iLeaveMsgLen - iNeedRecvMsgLen;
                                }

                                //是否还有剩余的字符没有写入
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

                                //关闭
                                fclose(fp);

                                //如果上面过程中,有失败或者放弃任务,则退出
                                if (0 != iRecvRet)  //包括了中断--PAUSE--
                                {
                                        break; //日至和错误码前面已经置了,不需要在这儿再写了
                                }

                                //接收完成，将文件移动到正确位置
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

                                //判断一下是否要进行权限的更改,有值，则进行修改
                                if('\0' != g_insConfigMng.m_tComCfg.m_sFileModeOption[0])
                                {
                                        CComFun::chmodFile(sRealPathFile, g_insConfigMng.m_tComCfg.m_sFileModeOption);
                                }
                                
                                //kevinliu add 2007-01-10 将文件在对应的位置下接包
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

                                //准备生成待发送任务(2.2第二部分，文件信息)
                                time((time_t*)&insSendDuty.m_iLocalTime); //当前日期
                                strcpy(insSendDuty.m_sLocalPathFile, sRealPathFile.c_str());

                                //生成待发送任务并加入队列(2.3第三部分，服务器信息)
                                if ( 0 != g_insQueueMng.putDuty2SendQ(insSendDuty, CQueueMng::AddServerInfo, insFileUploadMsg.m_iDataLength))
                                {
                                        eResponseCode = ServicePause; //函数中已经写了日志，不需再写
                                        break;
                                }
                                //CONTINUE while end << 用最后那次消息

                        }
                        g_insLogMng.debug(insFileUploadMsg.m_sSiteName, CLogMng::LogMode1, "END_suc<<%s>>",
                                 insFileUploadMsg.getPrintInfo().c_str());
                        
                        //关键关键日志记录，不用打开开关，记录接收到的所有的文件站点和文件名，应该不会对性能影响很大
                        g_insLogMng.runlog("Type=%d,%s=>%s:%s", 
                                  pMsgHead[1], 
                                  sSaveLocalFileName.c_str(),
                                  insFileUploadMsg.m_sSiteName, insFileUploadMsg.m_sDestPathFile);
                        break;
                }
                case ProtocolDelFile:
                {
                        //接收消息信息部分
                        CFileDelMsg insFileDelMsg;
                        iRecvMsgRet = insTcpServer.receiveMsg((char*)(&insFileDelMsg), sizeof(CFileDelMsg), iFileTimeout);
                        if (0 != iRecvMsgRet) //用了文件时间，即较长点的时间
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
                        //这个响应加上一块处理，因为担心由于还有后续消息，后续出错可能导致对端写错误 CONTINUE1>><<
                        //就有一个响应消息的份子，用最后一个响应消息,所以结束了  <<
                        //<<--为了防止,发送过来的消息中字段没有结束符,因此要做char数组的结束操作
                        insFileDelMsg.sureValidSrt();

                        g_insLogMng.debug(insFileDelMsg.m_sSiteName, CLogMng::LogMode1, "START<<%s>>",
                                 insFileDelMsg.getPrintInfo().c_str());

                        //生成正在接收任务
                        CRecvingDuty insRecvingDuty;
                        insRecvingDuty.m_iState = CRecvingDuty::Normal;
                        insRecvingDuty.m_iProtocolType = ProtocolDelFile;
                        strcpy(insRecvingDuty.m_sSiteName, insFileDelMsg.m_sSiteName);
                        strcpy(insRecvingDuty.m_sDestPathFile, insFileDelMsg.m_sDestPathFile);
                        strcpy(insRecvingDuty.m_sSendedTime , insFileDelMsg.m_sSendedTime);

                        //站点是否存在
                        map<string, CSiteInfo> insMapSite;
                        g_insConfigMng.getSiteMap(insMapSite);
                        if (insMapSite.end() == insMapSite.find(insRecvingDuty.m_sSiteName))
                        {
                                g_insLogMng.normal("FileDelMsg error because site:%s does not exist.",
                                        insRecvingDuty.m_sSiteName);
                                eResponseCode = InfoFail;
                                break;
                        }

                        //检查这个消息是否需要处理，如果需要添加处理，则已经在check中添加到队列中，不需要，则直接退出处理
                        if (0 > g_insQueueMng.check(insRecvingDuty))
                        {
                                eResponseCode = CancelDuty;  //函数中已经写了日志，不需再写
                                //break; --PAUSE--为了不使对待写失败导致重发,采用中断任务,收完后不处理的方式
                        }

                        //开始接收，准备生成待发送任务(第一部分，公共信息)
                        CSendDuty insSendDuty;
                        insSendDuty.m_iState = CSendDuty::Normal;
                        insSendDuty.m_iProtocolType = ProtocolDelFile;
                        strcpy(insSendDuty.m_sSiteName, insRecvingDuty.m_sSiteName);
                        strcpy(insSendDuty.m_sDestPathFile, insRecvingDuty.m_sDestPathFile);
                        strcpy(insSendDuty.m_sSendedTime,  insRecvingDuty.m_sSendedTime);

                        //准备生成待发送任务(第二部分，文件信息)
                        time((time_t*)&insSendDuty.m_iLocalTime); //当前日期
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

                        //执行
                        if (0 != g_insQueueMng.judgeNormal((int)pthread_self(), CQueueMng::RecvingQueue))
                        {
                                eResponseCode = CancelDuty;  //状态被别人改变，则认为成功
                                //break; //--PAUSE--为了不使对待写失败导致重发,采用中断任务,收完后不处理的方式
                        }

                        // --PAUSE--为了不使对待写失败导致重发,采用中断任务,收完后不处理的方式
                        if (eResponseCode == CancelDuty)
                        {
                                g_insLogMng.debug(insFileDelMsg.m_sSiteName, CLogMng::LogMode1, "OVER_suc<<%s>>",
                                        insFileDelMsg.getPrintInfo().c_str());
                                break;
                        }

                         g_insLogMng.debug(insFileDelMsg.m_sSiteName, CLogMng::LogMode2, "delete %s.",
                                        sPathFile.c_str());
                        if (0 != CComFun::delFile(sPathFile))  //在开始的机器上也执行一下,尽管没有文件,为了处理统一,只能如此
                        {
                                g_insLogMng.error("Delete file(%s) failed, but continue.",
                                        sPathFile.c_str());  //写个日志,但是继续执行
                        }

                        //准备生成待发送任务(第三部分，服务器信息)
                        if ( 0 != g_insQueueMng.putDuty2SendQ(insSendDuty, CQueueMng::AddServerInfo))
                        {
                                eResponseCode = ServicePause;  //函数中已经写了日志，不需再写
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
                        //接收消息信息部分
                        CFileMvCpMsg insFileMvCpMsg;
                        iRecvMsgRet = insTcpServer.receiveMsg((char*)(&insFileMvCpMsg), sizeof(CFileMvCpMsg), iFileTimeout);
                        if (0 != iRecvMsgRet) //用了文件时间，即较长点的时间
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
                        //这个响应加上一块处理，因为担心由于还有后续消息，后续出错可能导致对端写错误 CONTINUE1>><<
                        //就有一个响应消息的份子，用最后一个响应消息,所以结束了  <<
                        //<<--为了防止,发送过来的消息中字段没有结束符,因此要做char数组的结束操作
                        insFileMvCpMsg.sureValidSrt();

                        g_insLogMng.debug(insFileMvCpMsg.m_sSiteName, CLogMng::LogMode1, "START<<%s>>",
                                 insFileMvCpMsg.getPrintInfo().c_str());

                        //生成正在接收任务
                        CRecvingDuty insRecvingDuty;
                        insRecvingDuty.m_iState = CRecvingDuty::Normal;
                        insRecvingDuty.m_iProtocolType = (ProtocolMvFile==pMsgHead[1])?ProtocolMvFile:ProtocolCpFile;
                        strcpy(insRecvingDuty.m_sSiteName, insFileMvCpMsg.m_sSiteName);
                        strcpy(insRecvingDuty.m_sDestPathFile, insFileMvCpMsg.m_sDestPathFile);
                        strcpy(insRecvingDuty.m_sSendedTime , insFileMvCpMsg.m_sSendedTime);

                        //站点是否存在
                        map<string, CSiteInfo> insMapSite;
                        g_insConfigMng.getSiteMap(insMapSite);
                        if (insMapSite.end() == insMapSite.find(insRecvingDuty.m_sSiteName))
                        {
                                g_insLogMng.normal("FileMvCpMsg error because site:%s does not exist.",
                                        insRecvingDuty.m_sSiteName);
                                eResponseCode = InfoFail;
                                break;
                        }

                        //检查这个消息是否需要处理，如果需要添加处理，则已经在check中添加到队列中，不需要，则直接退出处理
                        if (0 > g_insQueueMng.check(insRecvingDuty))
                        {
                                eResponseCode = CancelDuty;  //函数中已经写了日志，不需再写
                                //break; --PAUSE--为了不使对待写失败导致重发,采用中断任务,收完后不处理的方式
                        }

                        //开始接收，准备生成待发送任务(第一部分，公共信息)
                        CSendDuty insSendDuty;
                        insSendDuty.m_iState = CSendDuty::Normal;
                        insSendDuty.m_iProtocolType = insRecvingDuty.m_iProtocolType;
                        strcpy(insSendDuty.m_sSiteName, insRecvingDuty.m_sSiteName);
                        strcpy(insSendDuty.m_sDestPathFile, insRecvingDuty.m_sDestPathFile);
                        strcpy(insSendDuty.m_sSendedTime,  insRecvingDuty.m_sSendedTime);

                        //准备生成待发送任务(第二部分，文件信息)
                        time((time_t*)&insSendDuty.m_iLocalTime); //当前日期
                        strcpy(insSendDuty.m_sLocalPathFile, insFileMvCpMsg.m_sSrcPathFile);

                        //目标文件
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

                        //源文件
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

                        //执行
                        if (0 != g_insQueueMng.judgeNormal((int)pthread_self(), CQueueMng::RecvingQueue))
                        {
                                eResponseCode = CancelDuty;  //状态被别人改变，则认为成功
                                //break; //--PAUSE--为了不使对待写失败导致重发,采用中断任务,收完后不处理的方式
                        }

                        // --PAUSE--为了不使对待写失败导致重发,采用中断任务,收完后不处理的方式
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
                                                sDestPathFile.c_str(), sSrcPathFile.c_str());  //写个日志,但是继续执行
                                }
                        }
                        else  //copy
                        {
                                if (0 != CComFun::cpFile(sSrcPathFile, sDestPathFile))
                                {
                                        g_insLogMng.error("cp file(%s %s) failed, but continue.",
                                                sDestPathFile.c_str(), sSrcPathFile.c_str());  //写个日志,但是继续执行
                                }
                        }

                        //准备生成待发送任务(第三部分，服务器信息)
                        if ( 0 != g_insQueueMng.putDuty2SendQ(insSendDuty, CQueueMng::AddServerInfo))
                        {
                                eResponseCode = ServicePause;  //函数中已经写了日志，不需再写
                                break;
                        }

                        g_insLogMng.debug(insFileMvCpMsg.m_sSiteName, CLogMng::LogMode1, "OVER_suc<<%s>>",
                                 insFileMvCpMsg.getPrintInfo().c_str());
                        break;
                }
                //END kevinliu add 2006-04-21                
                case ProtocolAddServer:
                {
                        //接收消息信息部分
                        CServerAddMsg insServerAddMsg;
                        iRecvMsgRet = insTcpServer.receiveMsg((char*)(&insServerAddMsg), sizeof(CServerAddMsg), iFileTimeout);
                        if (0 != iRecvMsgRet)//用了文件时间，即较长点的时间
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
                        //<<--为了防止,发送过来的消息中字段没有结束符,因此要做char数组的结束操作
                        insServerAddMsg.sureValidSrt();

                        insServerAddMsg.m_iPort = ntohl(insServerAddMsg.m_iPort);

                        g_insLogMng.debug("protocol", CLogMng::LogMode1, "ADD SERVER<<%s>>",
                                insServerAddMsg.getPrintInfo().c_str());

                        //由于站点组和站点服务器的配置，已经在运行过程中，不会再使用，索引不再处理
                        //只增加站点服务器即可
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
                       //接收消息信息部分
                        CServerDelMsg insServerDelMsg;
                       iRecvMsgRet = insTcpServer.receiveMsg((char*)(&insServerDelMsg), sizeof(CServerDelMsg), iFileTimeout);
                        if (0 != iRecvMsgRet)//用了文件时间，即较长点的时间
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
                        //<<--为了防止,发送过来的消息中字段没有结束符,因此要做char数组的结束操作
                        insServerDelMsg.sureValidSrt();

                        insServerDelMsg.m_iPort = ntohl(insServerDelMsg.m_iPort);

                        g_insLogMng.debug("protocol", CLogMng::LogMode1, "DELSERVER<<%s>>",
                                insServerDelMsg.getPrintInfo().c_str());

                        //由于站点组和站点服务器的配置，已经在运行过程中，不会再使用，索引不再处理
                        //只增加站点服务器即可
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
                        //暂且不支持
                        g_insLogMng.error("Recv AddSubServer, current not support.");
                        eResponseCode = MsgError;
                        break;
                }
                case ProtocolDelSubServer:
                {
                        //暂且不支持
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

        //发送最后的响应消息
        if (CancelDuty == eResponseCode)  //--PAUSE--取消掉的任务不处理
        {
                eResponseCode = Success;
        }

        g_insLogMng.debug("protocol", CLogMng::LogMode2, "Send last response msg.");
        if (0 != sendResponse(insTcpServer, eResponseCode))
        {
                g_insLogMng.normal("Send last response msg failed.");
        }

        //结束一次处理
        g_insLogMng.debug("protocol", CLogMng::LogMode2, "End to protocol process.");

        return;

}


void CProtocolMng::processSend(int iType)   // 2007-05-21 add itype
{
        //在队列中取出一个任务
        CSendDuty insSendingDuty;
        if (0 != g_insQueueMng.getDutyFSendQ(iType, insSendingDuty))  //已经放入了正在发送的队列
        {
                return;  //没有任务,或者取不到
        }
        // BEGIN kevinliu 2007-06-14 list队列，可以靠判断文件的本地时间来决定是否本任务还有效
        if ((0 == g_insMaintain.getFlag(CMaintain::QueueSafeMode)) &&
                (insSendingDuty.m_iProtocolType == (int)ProtocolUploadFile)) 
        //做成可配置的,防止这块有问题,条件也要求只对于上传文件
        //有一种可能就是bj78上，文件被修改了(比如预览),这种将会导致被丢弃，这种概率很小了
        //一旦发现明显，则启动本开关为1
        {
                //提前放到这儿，也可以在下面，但是那样会多雨一个connectserver的操作
                //同时这个本应该也可以放到getDutyFSendQ中实现，但是考虑到一旦存在多个任务被修改，
                //会使锁持有过长，因此
                struct stat tmpStat;
                if(0 <= stat( insSendingDuty.m_sLocalPathFile, &tmpStat))
                {
                        //m_iLocalTime 时间会在每次收到消息生成文件时，打上这个时间绰，本地文件必须先于它
                        //如果时间都比这个时间靠后了，说明文件被修改过了，同时肯定有了新的消息，则暂且不再发送
                        if (tmpStat.st_mtime > insSendingDuty.m_iLocalTime)
                        {
                                //写normal日志,属于关键的放弃文件，记录日志为好
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

        //在insSendDuty 中取出IP和端口，进行连接和发送
        //注意每次要判断一下，是否进行了取消操作
        int iMsgTimeout = g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout;  //消息超时时间
        int iFileTimeout = g_insConfigMng.m_tComCfg.m_iRecvFileTimeout;  //文件超时时间

        CTCPSocket  insClientSocket(insSendingDuty.m_sServerIP, insSendingDuty.m_iServerPort);
        if ( 0 != insClientSocket.connectServer())
        {
                g_insLogMng.error("Connect to server failed when sending duty(%s).",
                        insSendingDuty.getPrintInfo().c_str());
                insSendingDuty.m_iNrOfFail++;
                g_insQueueMng.putDuty2SendQ(insSendingDuty, CQueueMng::Direct); //重新发入队列
                g_insQueueMng.setOver((int)pthread_self(), CQueueMng::SendingQueue);//add1025
                return;
        }
        //添加报警，在connectServer里面实现最好，但是考虑到代码，暂时加到这儿，其他小地方connect的，暂且不处理
        if(insClientSocket.m_iFd > 900)
        {
                if (0 != access((g_sHomeDir + "/cfg/fd.if").c_str(),  F_OK))  //不存在就报警，如果确认系统可以支持到1000，可天添加此文件不来报警
                {
                        g_insMaintain.sendMobileMsg("建立连接文件描述符FD>900,系统已经自动清理前500个，请检查系统是否有FD泄露。");
                        //for(int ixx = 10; ixx < 500; ixx++)
                        //{
                        //        close(ixx); //为了防止系统导致的故障，这儿强行加入这个逻辑，如果确认不需要，请在cfg添加fd.if文件
                        //}
                }
                //写alarm跟踪文件 [ALARM:4]
                CComFun::writeWholeFileContent((g_sHomeDir + "/dat/maxfd.alm"), "fd>900"); 
        }
        //else
        //{
        //        CComFun::delFile(g_sHomeDir + "/dat/maxfd.alm"); 
        //}
        
        //消息缓冲区和文件暂存缓冲区
        char sMsgBuff[MsgBufferSize + 1];
        char sFileBuff[WriteToFileSize + 1];

        //发送消息头信息
        sMsgBuff[0] = Version1;
        sMsgBuff[1] = insSendingDuty.m_iProtocolType;

         g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode3, "Begin to send the msg head.");
        if (0 != insClientSocket.sendMsg(sMsgBuff, MsgHeadLen, iMsgTimeout))  //  send 1.1
        {
                 g_insLogMng.error("Write msg head data to socket failed when sending duty(%s).",
                        insSendingDuty.getPrintInfo().c_str());
                 insClientSocket.closeSocket();
                 insSendingDuty.m_iNrOfFail++;
                g_insQueueMng.putDuty2SendQ(insSendingDuty, CQueueMng::Direct); //重新发入队列
                g_insQueueMng.setOver((int)pthread_self(), CQueueMng::SendingQueue);
                return;
        }

        //根据不同的协议，分别进行不同的处理
        int iNotWaitResultMsg = 0; // 需要特殊处理的地方的标志,0无需,其他,需要处理
        ResponseCode eLastResponseCode = UnknownCode;
        switch(insSendingDuty.m_iProtocolType)
        {
        case ProtocolUploadFile:
        case ProtocolPackFile:  // add by kevinliu 2007-01-10
        {
                //是否存在且不是目录
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
                //文件大小
                long iFileSize = stSendFileStat.st_size;
                if (0 == iFileSize && 0 == g_insConfigMng.m_tComCfg.m_iIfUploadZeroFile )
                {
                         g_insLogMng.error("%s size is zero where send the duty(%s).",
                                insSendingDuty.m_sLocalPathFile,
                                insSendingDuty.getPrintInfo().c_str());
                         iNotWaitResultMsg = 1;
                        break;
                }


                //开始发送
                FILE *fp = fopen(insSendingDuty.m_sLocalPathFile, "r");
                if (NULL == fp)
                {
                         g_insLogMng.error("Open %s failed where send the duty(%s).",
                                insSendingDuty.m_sLocalPathFile,
                                insSendingDuty.getPrintInfo().c_str());
                         iNotWaitResultMsg = 1;
                        break;
                }

                //拼装并发送消息的info信息
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
                //CONTINUE每发送一块要收取确认消息，保证消息包不丢,暂时不采用断点续传的方式
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
                                        //继续
                                        eLastResponseCode = Success;
                                }
                                else //失败
                                {
                                        fclose(fp);
                                        eLastResponseCode = eResponseCode;
                                        g_insLogMng.error("Send Duty(%s)'s FileUploadMsg head block to socket,recv %x responecode.",
                                                                insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                        break;
                                }
                        }
                }

                //开始发送文件流信息
                int iLeftSize = iFileSize;
                int iWantBlockSize = 0;
                int iReadLen = 0;
                int iSendLen = 0;
                int iHadSendLen = 0;
                g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode3,
                        "Begin to send the msg data stream,filestream len = %ld", iFileSize);

                //计算一下，发送10需要休息多少毫秒
                int iCtrlTime = 0;
                if (0 != g_insConfigMng.m_tComCfg.m_iSendFlowCtrl)
                {
                        //有多少个10K
                        int iSizeCtrl = g_insConfigMng.m_tComCfg.m_iSendFlowCtrl/10;

                        //每块需要耗费多少毫秒，这个算法中，当流量大于每秒10M时，就无法控制流量了
                        if (0 == iSizeCtrl)
                        {
                                iCtrlTime = 1000;  //如果流量控制不足10K，则
                        }
                        else
                        {
                                iCtrlTime = 1000/iSizeCtrl;
                        }
                        g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode1,
                                "Send msg data need control flow(sizeCtrl = %d, CtrlTime = %d)", iSizeCtrl, iCtrlTime);
                }

                //开始发送
                while(iLeftSize > 0)
                {
                        //判断本次该读多少
                        if (iLeftSize >= WriteToFileSize)
                        {
                                iReadLen = WriteToFileSize;
                        }
                        else
                        {
                                iReadLen = iLeftSize;
                        }

                        //读取
                        iWantBlockSize = fread(sFileBuff, iReadLen, 1, fp);
                        //认为成功,唯一失败的情况可能是文件读取中被改写,记录日志备跟踪
                        if (1 != iWantBlockSize)
                        {
                                g_insLogMng.normal("fread(size=%d) != 1 when send the duty(%s).",
                                        iReadLen, insSendingDuty.getPrintInfo().c_str());
                                break;
                        }
                         iLeftSize = iLeftSize - iReadLen;  //改写剩余字节

                        iHadSendLen = 0;
                        while(iReadLen > 0)
                        {
                                iSendLen = (iReadLen >= MsgBufferSize)?MsgBufferSize:iReadLen;
                                memcpy(sMsgBuff, sFileBuff+ iHadSendLen, iSendLen);

                                //判断一下任务是否被取消,主动取消的任务无所谓，因为对端即使接收失败，也无须再进行后续处理了
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
                                //CONTINUE每发送一块要收取确认消息，保证消息包不丢,暂时不采用断点续传的方式
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
                                                        //继续
                                                }
                                                else //失败
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

                                //进行流量控制
                                if (0 != iCtrlTime)
                                {
                                        g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode2,
                                                "Send msg data need control flow, sleep(CtrlTime = %d ms)", iCtrlTime);
                                        if (iCtrlTime >= 1000)  //超过了秒
                                        {
                                                int iTmpCtrlTime = iCtrlTime/1000;
                                                CComFun::sleep_msec(iTmpCtrlTime, 0);
                                        }
                                        else
                                        {
                                                CComFun::sleep_msec(0, iCtrlTime);  //休息对应的毫秒数
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
                //拼装并发送消息的info信息
                CFileDelMsg insFileDelMsg;
                strcpy(insFileDelMsg.m_sSiteName , insSendingDuty.m_sSiteName);
                strcpy(insFileDelMsg.m_sDestPathFile, insSendingDuty.m_sDestPathFile);
                strcpy(insFileDelMsg.m_sSendedTime, insSendingDuty.m_sSendedTime);
                g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode3,  "Begin to send the info head");
                //判断一下任务是否被取消,主动取消的任务无所谓，因为对端即使接收失败，也无须再进行后续处理了
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
                //CONTINUE每发送一块要收取确认消息，保证消息包不丢,暂时不采用断点续传的方式
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
                                        //继续
                                }
                                else //失败
                                {
                                        eLastResponseCode = eResponseCode;
                                        g_insLogMng.error("Send Duty(%s)'s FileDelMsg to socket,recv %x responecode.",
                                                                insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                                        break;
                                }
                        }
                }
                break;  //跳出delete
        }
        //BEGIN kevinliu add 2006-04-21
        case ProtocolMvFile:  // mv 和 copy的协议是一样的,所以发送时可以统一处理
        case ProtocolCpFile:
        {
                //拼装并发送消息的info信息
                CFileMvCpMsg insFileMvCpMsg;
                strcpy(insFileMvCpMsg.m_sSiteName , insSendingDuty.m_sSiteName);
                strcpy(insFileMvCpMsg.m_sDestPathFile, insSendingDuty.m_sDestPathFile);
                strcpy(insFileMvCpMsg.m_sSrcPathFile, insSendingDuty.m_sLocalPathFile);
                strcpy(insFileMvCpMsg.m_sSendedTime, insSendingDuty.m_sSendedTime);
                g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode3,  "Begin to send the info head");
                //判断一下任务是否被取消,主动取消的任务无所谓，因为对端即使接收失败，也无须再进行后续处理了
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
                //CONTINUE每发送一块要收取确认消息，保证消息包不丢,暂时不采用断点续传的方式
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
                                        //继续
                                }
                                else //失败
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

        //结束处理，修改一下状态
        g_insQueueMng.setOver((int)pthread_self(), CQueueMng::SendingQueue);

        // 1 表示是目录或者open发送文件失败，此时可能是输入的文件不存在，也可能是被删，所以不处理
        if (1 == iNotWaitResultMsg)
        {
                g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode1, "END_failed_end<<%s>>",
                                 insSendingDuty.getPrintInfo().c_str());
                insClientSocket.closeSocket();
                return;
        }
        // 2 主动取消
        else if (2 == iNotWaitResultMsg)
        {
                g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode1, "END_suc_cancel<<%s>>",
                                 insSendingDuty.getPrintInfo().c_str());
                insClientSocket.closeSocket();
                return;
        } //else  0成功， 其他，发送完成或者不可预知的错误,可能是对方导致，尝试接受结果消息

        //当采用最后确认协议时,需要最后接收一下消息
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
                        else //失败
                        {
                                eLastResponseCode = eResponseCode;
                                g_insLogMng.error("Send Duty(%s) recv %x responecode.",
                                                        insSendingDuty.getPrintInfo().c_str(), eResponseCode);
                        }
                }
        }

        //分析响应消息
        if (Success == eLastResponseCode )
        {
                 g_insLogMng.debug(insSendingDuty.m_sSiteName, CLogMng::LogMode1, "END_suc<<%s>>",
                         insSendingDuty.getPrintInfo().c_str());
                //需要加入，最近成功队列
                g_insQueueMng.putDuty2LastSucQ(insSendingDuty);

                //如果发送完成需要删除文件，注意此时只能存在一个分发机器时使用，否则会出现很严重的错误
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
                 //重新放入队列
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
                iMsgTimeout = 2*g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout;  //消息超时时间
        }
        else 
        {
                iMsgTimeout = g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout/iTimeOut;  //指定了时间
                if (iMsgTimeout <= 0)
                {
                        iMsgTimeout = 2;  //如果不合法，默认为2秒
                }
        }
        CTCPSocket  insClientSocket(sIP.c_str(), iPort);
        if ( 0 != insClientSocket.connectServer())
        {
                g_insLogMng.debug("protocol", CLogMng::LogMode2, "Connect to server(%s %d) failed when checkServerStaus.",
                        sIP.c_str(), iPort);
                return -1;
        }

        //发送消息头信息
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

        //接受响应消息
        char sResponseCode;
        char sTmpStr[1024];
        if (0 == insClientSocket.receiveMsg(sTmpStr, 1, iMsgTimeout))  //为了确保一下成功,等待时间用2倍
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
        //发送响应消息
        char sRCode[2];
        sRCode[0] = resp;
        g_insLogMng.debug("protocol", CLogMng::LogMode2,  "Send the response msg.");
        return insSocket.sendMsg(sRCode, 1, g_insConfigMng.m_tComCfg.m_iRecvMsgTimeout);
}

int CProtocolMng::recvResponse(CTCPSocket&  insSocket, ResponseCode& resp)
{
        //接受响应消息
        char sTmpStrx[1024];
        g_insLogMng.debug("protocol", CLogMng::LogMode2,  "Recv the response msg.");
        int iRet = insSocket.receiveMsg(sTmpStrx, 1, g_insConfigMng.m_tComCfg.m_iRecvFileTimeout);
        if (0 == iRet)  //为了确保一下成功, 还是用接收文件的时间
        {
                resp = (ResponseCode)sTmpStrx[0];
        }
        return iRet;
}

//kevinliu add for 2.1版本 - 2007-04-29
 void CProtocolMng::processVersion2(CTCPSocket & insTcpServer, char cProtocolType)
{
        //统一接收,接受完成后,然后再根据不同的内容,处理,然后统一发送
        vector<string> vecRecvContent;  //用到消息消息存储
        vector<string> vecSendContent; //用于发送用的消息
         
        g_insLogMng.normal("Manager: recv a version2 message, prtocolType = %d.",
                cProtocolType);
         
        //(1)接受
        if ( 0 != recvVersion2Msg(insTcpServer, vecRecvContent))
        {
                return;
        }
        
        //(2)根据消息类型,进行处理
        switch(cProtocolType)
        {
                case ProtocolSearchTree:  
                {
                        //消息内容，就一个站点值
                        if (vecRecvContent.size() < 1)
                        {
                                g_insLogMng.error("ProtocolSearchTree: no content.");
                                return;
                        }
                        string sSitename = vecRecvContent[0];
                        //从配置文件中，根据格式，返回对应的服务器列表
                        map<string, CSiteInfo> insMapSiteTmp;
                        map<string, CSiteInfo> insMapSite;
                        g_insConfigMng.getSiteMap(insMapSiteTmp);     
                        string sTmpResult = "";
                        //所有站点的情况
                        if (sSitename.size() == 0)
                        {
                                insMapSite = insMapSiteTmp;                                
                        }
                        else
                        {
                                //需要判断这个站点是否存在
                                if (insMapSiteTmp.end() == insMapSiteTmp.find(sSitename))  //不存在
                                {
                                        sTmpResult = sSitename + ":null"; //直接在这儿赋值，后面的不会走了
                                }
                                else
                                {
                                        insMapSite[sSitename] = insMapSiteTmp[sSitename]; //只要这一个站点的
                                }
                        }
                        
                        map<string, CSiteInfo>::iterator it = insMapSite.begin();
                        for(; it != insMapSite.end(); it++)
                        {
                                if (sTmpResult != "")  //非第一个,要加;
                                {
                                         sTmpResult += ";";  //站点先写上
                                }
                                sTmpResult += it->first;  //站点先写上                                
                                //如果没有的，加入"none" 标识
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
                        //设置结果消息
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
                        int iType = atoi(vecRecvContent[0].c_str());  // 0 查看, 1提交修改
                        string sCommonCfgName = g_sHomeDir + "/cfg/common.cfg";
                        if (iType == 0)
                        {
                                //读取目录下的配置文件
                                string sTmpResult;
                                if (0 != CComFun::readWholeFileContent(sCommonCfgName, sTmpResult))
                                {
                                        setVersion2Resp(vecSendContent, "1", string ("read failed,") + sCommonCfgName,  "");
                                        break;
                                }
                                setVersion2Resp(vecSendContent, "0", "successfully",  sTmpResult);
                        }
                        else if (iType == 1)  // 更新
                        {
                                string sTmpResult = vecRecvContent[1];
                                //先备份一下原来的配置文件
                                CComFun::cpFile(sCommonCfgName, sCommonCfgName + ".bak"); //备份
                                //更新配置文件
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
                                //刷新到内存
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
                        // 0 查看, 1提交修改,2克隆站点配置信息
                        int iType = atoi(vecRecvContent[0].c_str());  
                        string sSiteCfgName = g_sHomeDir + "/cfg/siteinfo.cfg";
                        if (iType == 0)
                        {
                                //读取目录下的配置文件
                                string sTmpResult;
                                if (0 != CComFun::readWholeFileContent(sSiteCfgName, sTmpResult))
                                {
                                        setVersion2Resp(vecSendContent, "1", string ("read failed,") + sSiteCfgName,  "");
                                        break;
                                }
                                setVersion2Resp(vecSendContent, "0", "successfully",  sTmpResult);
                        }
                        else if (iType == 1)  // 更新
                        {
                                string sTmpResult = vecRecvContent[1];
                                //先备份一下原来的配置文件
                                CComFun::cpFile(sSiteCfgName, sSiteCfgName + ".bak"); //备份
                                //更新配置文件
                                if (0 != CComFun::writeWholeFileContent(sSiteCfgName, sTmpResult))
                                {
                                        setVersion2Resp(vecSendContent, "1", 
                                                "writeWholeFileContent failed, please refer log info.",  "");
                                         CComFun::cpFile(sSiteCfgName + ".bak", sSiteCfgName); 
                                         break;
                                }
                                //刷新到内存
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
                                //再将消息转发到下属去(不判断成功与否)
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
                        int iType = atoi(vecRecvContent[0].c_str());  // 0 查看, 1提交修改
                        string sMobilecfgName = g_sHomeDir + "/cfg/mobilelist.cfg";
                        if (iType == 0)
                        {
                                //读取目录下的配置文件
                                string sTmpResult;
                                if (0 != CComFun::readWholeFileContent(sMobilecfgName, sTmpResult))
                                {
                                        setVersion2Resp(vecSendContent, "1", string ("read failed,") + sMobilecfgName,  "");
                                        break;
                                }
                                setVersion2Resp(vecSendContent, "0", "successfully",  sTmpResult);
                        }
                        else if (iType == 1)  // 更新并将这消息，传递到下下面的所有的分发服务器
                        {
                                string sTmpResult = vecRecvContent[1];
                                //先备份一下原来的配置文件
                                CComFun::cpFile(sMobilecfgName, sMobilecfgName + ".bak"); //备份
                               //更新配置文件
                                if (0 != CComFun::writeWholeFileContent(sMobilecfgName, sTmpResult))
                                {
                                        setVersion2Resp(vecSendContent, "1", 
                                                "writeWholeFileContent failed, please refer log info.",  "");
                                         CComFun::cpFile(sMobilecfgName + ".bak", sMobilecfgName); 
                                         break;
                                }
                                //将消息转发下去，所有下属站点
                                if(1 == atoi(vecRecvContent[2].c_str())) //需要通知下面
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
                        int iType = atoi(vecRecvContent[0].c_str());  // 0 查看, 1提交修改
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
                                if(0 == vecRecvContent[1].size())   //是否指定了日期
                                {
                                        sTmpDate = CComFun::GetCurDate();//默认为当前日期
                                }
                                else
                                {
                                        sTmpDate = vecRecvContent[1]; //指定的日期
                                }
                                sLogName = g_sHomeDir + "/log/stat/queue"+ sTmpDate +".xml";
                        }
                        else  if (iType == 4)//直接度文件的逻辑，这块不做校验，直接有前端自行保证
                        {
                                //所在主目录下的任何文件
                                sLogName= g_sHomeDir + vecRecvContent[1];
                        }
                        //读取目录下的文件
                        string sTmpResult = "none";  //如果为这个值,说明没有取到
                        CComFun::readWholeFileContent(sLogName, sTmpResult); //是否能读到不再判断
                        if (iType == 3) //如果是xml,加上xml框子
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
                        int iType = atoi(vecRecvContent[0].c_str());  // 0 查看队列信息, 1 查看其他，暂时为实现
                        string sTmpResult = "";
                        if (iType == 0)
                        {
                                //获取内容
                                g_insQueueMng.getCurInfo(sTmpResult);
                                setVersion2Resp(vecSendContent, "0", "successfully",  sTmpResult);
                        }
                        else if (iType == 1)
                        {
                                //g_insMaintain.reloadFailDuty(-1); //全部
                                 g_insMaintain.setFlag(CMaintain::ReloadFailDuty, -1);
                                setVersion2Resp(vecSendContent, "0", "successfully",  "");
                        }
                        else if (iType == 2)
                        {
                                g_insQueueMng.clearErrorIPPort(""); //全部
                                setVersion2Resp(vecSendContent, "0", "successfully",  "");
                        }
                        else
                        {
                                setVersion2Resp(vecSendContent, "1", "unkown type.",  "");  //失败
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

        //(3)发送 (socket的回收再上层)
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
        iTotalLen = ntohl(iTotalLen) -6; //去掉消息头的6个字节
        //为了防止对方构造消息错误，暂时将消息定位4*65535的大小,需要时再扩展
        if (iTotalLen > 4* 65535 || iTotalLen <= 0)
        {
                g_insLogMng.error("recvVersion2Msg:len is invalid, %d > 4* 65535 or < 0.", iTotalLen);
                return -1;
        }
        
        //然后接受完所有的内容
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

        //对内容进行解码       
        int iCurLen = 0;
        //cType 必须依次排号
        while(iCurLen < iTotalLen)
        {
                //Type
                char cType;
                CComFun::msg_getChar(pBuffer, iCurLen, cType);
                iCurLen += sizeof(char);
                //如果接收次序出了错误
                if ((unsigned int)cType != vecRecvContent.size())  // 接收的时候,序号应该是等于当时的长度的
                {
                        g_insLogMng.error("recvVersion2Msg:TLV's type is %d != %d, invalid, break decode.", cType, vecRecvContent.size());
                        delete[] pBuffer;
                        return -1;
                }
                
                //Length
                int iLength;
                CComFun::msg_getInt(pBuffer, iCurLen, iLength);
                iCurLen += sizeof(int);
                //长度值不合法
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

                //存储起消息内容来
                vecRecvContent .push_back(sValue);

                g_insLogMng.debug("protocol", CLogMng::LogMode1, "%s: Type=%d ,Value = %s",  "decode", cType, sValue.c_str());
        }

        //删除掉消息的空间
        delete[] pBuffer;
        pBuffer = NULL;

        return 0;

}

int CProtocolMng::sendVersion2Msg(CTCPSocket & insTcpServer, char cMsgType, vector<string>& vecSendContent)
{
        g_insLogMng.debug("protocol", CLogMng::LogMode3, "Begin to Send a version2 msg.");
        
        // 先计算一下需要多大的空间
        int iContentLen = 0;

        unsigned int iLoop;
        for(iLoop = 0; iLoop < vecSendContent.size(); iLoop++)
        {
                iContentLen += vecSendContent[iLoop].size();
        }

        int iTotalLen = 6 + vecSendContent.size() * (sizeof(char) + sizeof(int))  + iContentLen;

        //申请空间
        char* pMsgBuff = new char[iTotalLen];
        if (NULL == pMsgBuff)
        {
                return -1;
        }

        //将内容打包
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

        //实际的发送消息
        if (0 != insTcpServer.sendMsg(pMsgBuff, iTotalLen, g_insConfigMng.m_tComCfg.m_iRecvFileTimeout))
        {
                g_insLogMng.error("2 Send Response Msg failed.");
                delete[]  pMsgBuff;
                return -1;
        }
        
        delete[]  pMsgBuff;
        return 0; 
}

//将消息转发到下面的分发服务器，sitename为空，表示所有的服务器都要转发消息，不为空，则表示只向对于得分发服务器发消息
int CProtocolMng::forwardVersion2Msg(char cMsgType, vector<string>& vecmsgContent, const string& sitename)
{
        //获得服务器列表
        map<string, CSiteInfo> insMapSite;
        map<string, CServerInfo> insServerList; //服务器列表，如果没有下属服务器，这儿也能保证不会有问题        
        g_insConfigMng.getSiteMap(insMapSite);   
        if (sitename.size() == 0)
        {
                //提取所有站点下的服务器列表
                map<string, CSiteInfo>::iterator it = insMapSite.begin();
                for(; it != insMapSite.end(); it++)
                {
                        vector<CServerInfo>::iterator it1 = (it->second.m_vecServerList).begin();
                        for(; it1 != (it->second.m_vecServerList).end(); it1++)
                        {
                                string sKey =(*it1).m_sName; //机器的名称唯一
                                insServerList[sKey] = (*it1);  //主要取唯一的服务器列表
                        }
                }
        }
        else
        {
                //判断站点是否存在
                map<string, CSiteInfo>::iterator it = insMapSite.find(sitename);
                if (insMapSite.end() != it)  //存在
                {
                        //单一站点的服务器列表
                        vector<CServerInfo>::iterator it1 = (it->second.m_vecServerList).begin();
                        for(; it1 != (it->second.m_vecServerList).end(); it1++)
                        {
                                string sKey =(*it1).m_sName; //机器的名称唯一
                                insServerList[sKey] = (*it1);  //主要取唯一的服务器列表
                        }
                }                
        }
        insMapSite.clear();

        //开始发送消息,这儿不保证安全,只发送,不接受响应,
        //因为存在多层级联,无法保证,所以是否成功,靠日志和察看网络,处理结果只在对端日志中显示
        map<string, CServerInfo>::iterator it2 = insServerList.begin();
        for(; it2!=insServerList.end(); it2++)
        {
                //和Server建立连接
                CTCPSocket objClientSocket;
                objClientSocket.init(it2->second.m_sIP,  it2->second.m_iPort);
                if (0 != objClientSocket.connectServer())
                {
                        g_insLogMng.error("Connect server (%s, %d) failed,continue next.\n", 
                                it2->second.m_sIP,  it2->second.m_iPort);
                        continue;
                }

                //不判断成功与否,也不接受结构消息,直接结束
                sendVersion2Msg(objClientSocket, cMsgType, vecmsgContent);
                objClientSocket.closeSocket();
        }
        return 0;        
}

int CProtocolMng::cloneSiteCfg(const string& sBySiteName, const string& sAddSiteName)
{
        //这个是本次开发的主要功能，就是在任何机器上，根据另外一个站点复制出一个新的站点信息分发信息
        // 采用的方法是,按照规范修改本地配置文件,然后再刷新的方法;
        // 同时此消息需要传递到下面的站点去，是否成功，依赖于用户在客户端察看验证
        //[SITEINFO]
        //NrOfSite = 46
        // [SITE046]
        //SiteName = search
        //ServerGroup = OtherServerGroup
        //DocumentRoot = /data1/qq_news/htdocs (站点信息等价替换)

        //clone自己无需处理
        if(sBySiteName == sAddSiteName)
        {
                return 0;
        }

        //采用重写文件的方式, 逐行处理和重写
        // siteinfo配置文件的路径及其文件名
        string sSiteInfoCfgName = g_sHomeDir + "/cfg/siteinfo.cfg";
        g_insLogMng.normal("Begin to read %s ......", sSiteInfoCfgName.c_str());

        //初始化配置文件
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
        iNrOfSite= atoi(tmpstr);  //不用判断，因为系统能启动，肯定是合法的了
        //存放新增的结果的
        string sServerGroup = "";
        string sDocumentRoot= "";
        string sExistServerGroup= ""; //如果已经存在,这儿记录存在的组,以备刷新,否则不处理

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

        //如果并不存在源站点
        if (sServerGroup == "")
        {
                g_insLogMng.error("source site %s not exist.", sBySiteName.c_str());
                return -1; 
        }
        if (sExistServerGroup == sServerGroup)  //已经存在无需修改
        {
                return 0;
        }
        
        //需要处理的情况
        CComFun::cpFile(sSiteInfoCfgName, sSiteInfoCfgName + ".bak"); // 备份

        string sContent = "";
        string sNewContent = "";
        if (0 != CComFun::readWholeFileContent(sSiteInfoCfgName, sContent))
        {
                g_insLogMng.error("read %s failed", sSiteInfoCfgName.c_str());
                return -1;
        }
        //必须保证文件中，只能有一个NrOfSite字段
        string::size_type pos = sContent.find("NrOfSite", 0);    //找到这块,此前的全盘复制
        if (string::npos  == pos)
        {
                 g_insLogMng.error("file %s error, no NrOfSite", sSiteInfoCfgName.c_str());
                return -1;
        }
        sNewContent = sContent.substr(0, pos);  //前半部分
        if (sNewContent[sNewContent.size() -1] != '\n')
        {
                //不管是哪种文件,应该都是一个新行的开始 (\r\n  or \n)
                g_insLogMng.error("file %s error, NrOfSite pos is error", sSiteInfoCfgName.c_str());
                return -1;    
        }
        
        if (sExistServerGroup.size() == 0)  //新增
        {
                sprintf(sTmpSec, "NrOfSite = %d\n", iNrOfSite+1);
                sNewContent += string(sTmpSec);  // 数字++
                //开始处理所有的站点
                for (int iLoop = 1; iLoop <= iNrOfSite; iLoop++)
                {
                        sprintf(sTmpSec, "SITE%03d", iLoop);
                        sNewContent += string("\n[") +sTmpSec + "]\n";  //前面也加一行，目的是美观
                        insCfgSiteInfo.ReadItem(sTmpSec, "SiteName","", tmpstr);     
                        sNewContent += string("SiteName = ") + tmpstr + "\n"; 
                        insCfgSiteInfo.ReadItem(sTmpSec, "ServerGroup","", tmpstr);
                        sNewContent += string("ServerGroup = ") + tmpstr + "\n"; 
                        insCfgSiteInfo.ReadItem(sTmpSec, "DocumentRoot","", tmpstr);
                        sNewContent += string("DocumentRoot = ") + tmpstr + "\n";
                }
                //最后新增的那个
                sprintf(sTmpSec, "SITE%03d", iNrOfSite+1);
                sNewContent += string("\n[") +sTmpSec + "]\n";  //前面也加一行，目的是美观
                sNewContent += string("SiteName = ") + sAddSiteName + "\n"; 
                sNewContent += string("ServerGroup = ") + sServerGroup + "\n"; 
                //这个得慎重,万一出现了存在多个站点名的主目录怎么办呢?
                //在此做个约束了，所有出现的都替换掉，如果对结果不满意，可以再通过手工修改的方式                
                //如果没有出现，那只能直接放到/usr/local下建立对应的站点主目录
                //先看看有没有
                if(string::npos == sDocumentRoot.find(sBySiteName, 0)) //没有站点信息，那只能直接放到/usr/local下
                {
                        sNewContent += string("DocumentRoot = /usr/local/qq_") + sAddSiteName+ "/htdocs\n"; 
                }
                else //存在
                {
                        CComFun::find8Replace(sDocumentRoot, sBySiteName, sAddSiteName, 2);
                        sNewContent += string("DocumentRoot = ") + sDocumentRoot+ "\n";   //这个要把里面的站点替换掉     
                }
        }
        else  // 存在 修改分发组名
        {
                sprintf(sTmpSec, "NrOfSite = %d\n", iNrOfSite);
                sNewContent += string(sTmpSec); 
                //开始处理所有的站点
                for (int iLoop = 1; iLoop <= iNrOfSite; iLoop++)
                {
                        sprintf(sTmpSec, "SITE%03d", iLoop);
                        sNewContent += string("\n[") +sTmpSec + "]\n";  //前面也加一行，目的是美观
                        insCfgSiteInfo.ReadItem(sTmpSec, "SiteName","", tmpstr);     
                        sNewContent += string("SiteName = ") + tmpstr + "\n"; 
                         if (sAddSiteName != tmpstr)
                        {                        
                                insCfgSiteInfo.ReadItem(sTmpSec, "ServerGroup","", tmpstr);
                                sNewContent += string("ServerGroup = ") + tmpstr + "\n"; 
                        }
                        else //替换
                        {
                                sNewContent += string("ServerGroup = ") + sServerGroup + "\n"; 
                        }
                        insCfgSiteInfo.ReadItem(sTmpSec, "DocumentRoot","", tmpstr);
                        sNewContent += string("DocumentRoot = ") + tmpstr + "\n";
                }
        }
        
        //将这个文件写入
        if( 0 != CComFun::writeWholeFileContent(sSiteInfoCfgName, sNewContent))
        {
                CComFun::cpFile(sSiteInfoCfgName + ".bak", sSiteInfoCfgName); // 备份
                g_insLogMng.error("writeWholeFileContent(%s) failed, ", sSiteInfoCfgName.c_str());
                return -1;
        }

        //刷新到内存
        if (0 != g_insMaintain.refreshSiteInfo())
        {
                 CComFun::cpFile(sSiteInfoCfgName + ".bak", sSiteInfoCfgName); // 备份
                 g_insLogMng.error("writeWholeFileContent(%s) failed, ", sSiteInfoCfgName.c_str());
                 return -1;
        }        
        return 0;
}

