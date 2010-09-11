//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: FileUploadMsg.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef FILEUPLOADMSG_H_HEADER_INCLUDED_BCE1B37A
#define FILEUPLOADMSG_H_HEADER_INCLUDED_BCE1B37A

#include "ComFun.h"

//�������ļ��ϴ���Ϣ[û��������Ϣ���Ⱥ�����,�Լ����ݿ�]
class CFileUploadMsg
{
public:
        CFileUploadMsg();
        ~CFileUploadMsg();

        //��ʼ���ַ�����Ϣ�Ľ�����
        void sureValidSrt();

        //��������Ŀɹ���ӡ��Ϣ
        string getPrintInfo();

        enum UploadFileType
        {
                FileName = 0x1,  //�ļ�����
                FileData = 0x2     //�ļ���
        };

        //վ������
        char    m_sSiteName[LenOfSiteName];

        //�ļ���(����·��)
        char m_sDestPathFile[LenOfPathFileName];

        //��Ϣ�ķ���ʱ��
        char m_sSendedTime[LenOfTime];

        //������������
        char    m_cDataType;

        //�����ֶ�
        char    m_sReserve[3];

        //�������ĳ���
        int      m_iDataLength;
        
};

#endif /* FILEUPLOADMSG_H_HEADER_INCLUDED_BCE1B37A */

