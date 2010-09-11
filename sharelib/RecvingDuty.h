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
                Error = -1,  //���ճ�����Ҫ��������
                Normal = 0, //�����Ľ���״̬
                Giveup = 1,   //���µ����滻������������Ҫ������������
                Over = 2  //�����Ѿ�����
        };

public:
        //����״̬
        RevingState m_iState;

        //Э������
        int m_iProtocolType;

        //վ������
        char m_sSiteName[LenOfSiteName];

        //�ļ���(����·��)
        char m_sDestPathFile[LenOfPathFileName];

        //��Ϣ�ķ���ʱ��
        char m_sSendedTime[LenOfTime];
};

#endif /* RECVINGDUTY_H_HEADER_INCLUDED_BCE19785 */

