//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *
//  *    $RCSfile: ComCfg.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "ComCfg.h"

CComCfg::CComCfg()
{
        m_sLoginIP[0] = '\0';
        m_sServerFlag[0] = '\0';
        m_iLogMode = 0;
        m_sFileModeOption[0] = '\0';
        m_iDelFileAfterUpload = 0;
        m_iIfUploadZeroFile = 0;
        m_iMaxLimitOfUploadFile = 0;
        m_iMaxNrOfRetry = 5;
        m_iProtocolType = 0;
        m_iIfFailLogFlag = 0;
        m_iMaxNrOfQueueDuty = 100000; // 10W
        m_iMaxNrOfSaveLastSucDuty = 10000;
        m_iTimeOfFailDutyRetry = 0;
        m_iTimeOfThreadStat = 0;
        m_iTimeOfCheckRunStatus = 0;
        m_iMaxNrOfDutyToAlarm = 0;
        m_iDistributeFlag = 0;
        m_sDBHost[0] = '\0';
        m_sDBUser[0] = '\0';
        m_sDBPassword [0] = '\0';
        m_sServerIP[0] = '\0';
        m_iServerPort = 0;
        m_iRecvNrOfThread = 1;
        m_iSendNrOfThread = 1;
        m_iSendErrorNrOfThread = 1;
        m_iMaxNrOfCnt = 0;
        m_iRecvMsgTimeout = 60;
        m_iRecvFileTimeout= 60;
        m_iSendFlowCtrl = 0;
}

CComCfg::CComCfg(const CComCfg& right)
{
        strcpy(m_sLoginIP, right.m_sLoginIP);
        strcpy(m_sServerFlag, right.m_sServerFlag);
        m_iLogMode = right.m_iLogMode;
        strcpy(m_sFileModeOption, right.m_sFileModeOption);
        m_iDelFileAfterUpload = right.m_iDelFileAfterUpload;
        m_iIfUploadZeroFile = right.m_iIfUploadZeroFile;
        m_iMaxLimitOfUploadFile = right.m_iMaxLimitOfUploadFile;
        m_iMaxNrOfRetry = right.m_iMaxNrOfRetry;
        m_iProtocolType = right.m_iProtocolType;
        m_iIfFailLogFlag = right.m_iIfFailLogFlag;
        m_iMaxNrOfQueueDuty = right.m_iMaxNrOfQueueDuty; // 10W
        m_iMaxNrOfSaveLastSucDuty = right.m_iMaxNrOfSaveLastSucDuty;
        m_iTimeOfFailDutyRetry = right.m_iTimeOfFailDutyRetry;
        m_iTimeOfThreadStat = right.m_iTimeOfThreadStat;
        m_iTimeOfCheckRunStatus = right.m_iTimeOfCheckRunStatus;
        m_iMaxNrOfDutyToAlarm = right.m_iMaxNrOfDutyToAlarm;
        m_iDistributeFlag = right.m_iDistributeFlag;
        strcpy(m_sDBHost, right.m_sDBHost);
        strcpy(m_sDBUser, right.m_sDBUser);
        strcpy(m_sDBPassword, right.m_sDBPassword);
        strcpy(m_sServerIP, right.m_sServerIP);
        m_iServerPort =right.m_iServerPort;
        m_iRecvNrOfThread = right.m_iRecvNrOfThread;
        m_iSendNrOfThread = right.m_iSendNrOfThread;
        m_iSendErrorNrOfThread = right.m_iSendErrorNrOfThread;
        m_iMaxNrOfCnt = right.m_iMaxNrOfCnt;
        m_iRecvMsgTimeout = right.m_iRecvMsgTimeout;
        m_iRecvFileTimeout= right.m_iRecvFileTimeout;
        m_iSendFlowCtrl = right.m_iSendFlowCtrl;
}

CComCfg::~CComCfg()
{
}

CComCfg& CComCfg::operator=(const CComCfg& right)
{
        if (this != &right)
        {
                strcpy(m_sLoginIP, right.m_sLoginIP);
                strcpy(m_sServerFlag, right.m_sServerFlag);
                m_iLogMode = right.m_iLogMode;
                strcpy(m_sFileModeOption, right.m_sFileModeOption);
                m_iDelFileAfterUpload = right.m_iDelFileAfterUpload;
                m_iIfUploadZeroFile = right.m_iIfUploadZeroFile;
                m_iMaxLimitOfUploadFile = right.m_iMaxLimitOfUploadFile;
                m_iMaxNrOfRetry = right.m_iMaxNrOfRetry;
                m_iProtocolType = right.m_iProtocolType;
                m_iIfFailLogFlag = right.m_iIfFailLogFlag;
                m_iMaxNrOfQueueDuty = right.m_iMaxNrOfQueueDuty; // 10W
                m_iMaxNrOfSaveLastSucDuty = right.m_iMaxNrOfSaveLastSucDuty;
                m_iTimeOfFailDutyRetry = right.m_iTimeOfFailDutyRetry;
                m_iTimeOfThreadStat = right.m_iTimeOfThreadStat;
                m_iTimeOfCheckRunStatus = right.m_iTimeOfCheckRunStatus;
                m_iMaxNrOfDutyToAlarm = right.m_iMaxNrOfDutyToAlarm;
                m_iDistributeFlag = right.m_iDistributeFlag;
                strcpy(m_sDBHost, right.m_sDBHost);
                strcpy(m_sDBUser, right.m_sDBUser);
                strcpy(m_sDBPassword, right.m_sDBPassword);
                strcpy(m_sServerIP, right.m_sServerIP);
                m_iServerPort =right.m_iServerPort;
                m_iRecvNrOfThread = right.m_iRecvNrOfThread;
                m_iSendNrOfThread = right.m_iSendNrOfThread;
                m_iSendErrorNrOfThread = right.m_iSendErrorNrOfThread;
                m_iMaxNrOfCnt = right.m_iMaxNrOfCnt;
                m_iRecvMsgTimeout = right.m_iRecvMsgTimeout;
                m_iRecvFileTimeout= right.m_iRecvFileTimeout;
                m_iSendFlowCtrl = right.m_iSendFlowCtrl;
        }

        return *this;
}

