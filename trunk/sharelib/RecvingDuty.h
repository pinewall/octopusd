//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: RecvingDuty.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef RECVINGDUTY_H_HEADER_INCLUDED_BCE19785
#define RECVINGDUTY_H_HEADER_INCLUDED_BCE19785

#include "ComFun.h"

class CRecvingDuty
{
public:
        CRecvingDuty();
        CRecvingDuty(const CRecvingDuty& right);
        ~CRecvingDuty();
        CRecvingDuty& operator=(const CRecvingDuty& right);
        string getPrintInfo();

        enum RevingState
        {
                Error = -1,  //接收出错，需要结束任务
                Normal = 0, //正常的接收状态
                Giveup = 1,   //有新的能替换掉本次任务，需要放弃本次任务
                Over = 2  //任务已经结束
        };

public:
        //任务状态
        RevingState m_iState;

        //协议类型
        int m_iProtocolType;

        //站点名称
        char m_sSiteName[LenOfSiteName];

        //文件名(包括路径)
        char m_sDestPathFile[LenOfPathFileName];

        //消息的发送时间
        char m_sSendedTime[LenOfTime];
};

#endif /* RECVINGDUTY_H_HEADER_INCLUDED_BCE19785 */

