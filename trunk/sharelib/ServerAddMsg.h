//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ServerAddMsg.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef SERVERADDMSG_H_HEADER_INCLUDED_BCE1B9C2
#define SERVERADDMSG_H_HEADER_INCLUDED_BCE1B9C2

#include "ComFun.h"

//�������ӷ���������Ϣ[û��������Ϣ���Ⱥ�����]
class CServerAddMsg
{
public:
        CServerAddMsg();
        ~CServerAddMsg();

        //��Ϣ�Ĵ�ӡ��Ϣ
        string getPrintInfo();

        //��ʼ���ַ�����Ϣ�Ľ�����
        void sureValidSrt();

        //վ������
        char m_sSiteName[LenOfSiteName];

        //������IP
        char m_sServerIP[LenOfIPAddr];

        //�˿�
        int  m_iPort;

        //Զ�˻��ǽ���,�Ѿ�����,Ϊ�˱��ֽӿڲ���,��ʱ����
        int  m_isRemote;

        //��Ϣ��ʱ��
        char m_sTime[LenOfTime];
};

#endif /* SERVERADDMSG_H_HEADER_INCLUDED_BCE1B9C2 */

