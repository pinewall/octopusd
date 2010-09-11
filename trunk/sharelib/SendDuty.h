//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: SendDuty.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: Kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef SENDDUTY_H_HEADER_INCLUDED_BCE19785
#define SENDDUTY_H_HEADER_INCLUDED_BCE19785

#include "ComFun.h"

//��������е�������
class CSendDuty
{
public:
        CSendDuty();
        CSendDuty(const CSendDuty& right);
        ~CSendDuty();
        CSendDuty& operator=(const CSendDuty& right);
        string getPrintInfo();

         enum SendState
        {
                Error = -1,   //������Ҫ��������
                Normal = 0,  //�����ķ���״̬
                Giveup = 1,   //���µ����滻������������Ҫ������������
                Over = 2       //�����Ѿ�����
        };
         
public:
        //�����״̬
        SendState m_iState;  
        
        //Э������
        int m_iProtocolType;

        //վ����
        char m_sSiteName[LenOfSiteName];

        //�ڷ������ϵ�·�� ���
        char m_sDestPathFile[LenOfPathFileName];

        //ʱ��
        char m_sSendedTime[LenOfTime];
        
        //����·��
        char m_sLocalPathFile[LenOfPathFileName];

        //�����ļ����ɵ�ʱ��
        int    m_iLocalTime;  
        
        // վ�������ip��ַ
        char m_sServerIP[LenOfIPAddr];
        
        // վ��������˿ں�
        int    m_iServerPort;

        // ��¼����Ϣ�������������һ��������Ͳ����͸���Ϣ�ˡ�
        int    m_iNrOfFail;
};

#endif /* SENDDUTY_H_HEADER_INCLUDED_BCE19785 */

