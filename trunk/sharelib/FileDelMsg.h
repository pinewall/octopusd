//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: FileDelMsg.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef FILEDELMSG_H_HEADER_INCLUDED_BCE1E925
#define FILEDELMSG_H_HEADER_INCLUDED_BCE1E925

#include "ComFun.h"

//�ļ�ɾ����Ϣ[û��������Ϣ���Ⱥ�����]
class CFileDelMsg
{
public:
        CFileDelMsg();
        ~CFileDelMsg();

        //��ʼ���ַ�����Ϣ�Ľ�����
        void sureValidSrt();

        //��������Ŀɹ���ӡ��Ϣ
        string getPrintInfo();

        //վ������
        char  m_sSiteName[LenOfSiteName];

        //Ŀ������·��
        char  m_sDestPathFile[LenOfPathFileName];

        //��Ϣ�ķ���ʱ��
        char  m_sSendedTime[LenOfTime];        
};


//�ļ��ƶ����߿�����Ϣ[û��������Ϣ���Ⱥ�����]
class CFileMvCpMsg
{
public:
        CFileMvCpMsg();
        ~CFileMvCpMsg();

        //��ʼ���ַ�����Ϣ�Ľ�����
        void sureValidSrt();

        //��������Ŀɹ���ӡ��Ϣ
        string getPrintInfo();

        //Ŀ��վ������
        char  m_sSiteName[LenOfSiteName];

        //Ŀ������·��
        char  m_sDestPathFile[LenOfPathFileName];

        //Դ�ļ���·��,Դվ������+���·������ʽ������new:/inc/picinpic.htm
        char m_sSrcPathFile[LenOfPathFileName];

        //��Ϣ�ķ���ʱ��
        char  m_sSendedTime[LenOfTime];        
};

#endif /* FILEDELMSG_H_HEADER_INCLUDED_BCE1E925 */

