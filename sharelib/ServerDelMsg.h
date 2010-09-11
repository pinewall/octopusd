//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ServerDelMsg.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef SERVERDELMSG_H_HEADER_INCLUDED_BCE18528
#define SERVERDELMSG_H_HEADER_INCLUDED_BCE18528

#include "ComFun.h"

//�����ɾ������������Ϣ[û��������Ϣ���Ⱥ�����]
class CServerDelMsg
{
public:
        CServerDelMsg();
        ~CServerDelMsg();

        //��Ϣ�Ĵ�ӡ��Ϣ
        string getPrintInfo();

        //��ʼ���ַ�����Ϣ�Ľ�����
        void sureValidSrt();

        //վ������
        char m_sSiteName[LenOfSiteName];

        //վ��IP
        char m_sServerIP[LenOfIPAddr];

        //�˿�
        int  m_iPort; 

        //��Ϣ��ʱ��
        char m_sTime[LenOfTime];
};

#endif /* SERVERDELMSG_H_HEADER_INCLUDED_BCE18528 */

