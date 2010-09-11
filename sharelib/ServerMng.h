//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ServerMng.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: Kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef SERVERMNG_H_HEADER_INCLUDED_BCE185FC
#define SERVERMNG_H_HEADER_INCLUDED_BCE185FC

#include "ComFun.h"
#include "LogMng.h"
#include "ConfigMng.h"
#include "ServerAddMsg.h"
#include "ServerDelMsg.h"

//���ͷ��������б�
class CServerMng
{
public:
        /**
         * \brief ��ӷ�����
         * \param insServerAddMsg ���ӵķ�������Ϣ
         * \return 0 �ɹ�-1ʧ��
         */ 
        static int addServer(CServerAddMsg& insServerAddMsg);

        /**
         * \brief ɾ��������
         * \param insServerDelMsg ɾ���ķ�������Ϣ
         * \return 0 �ɹ�-1ʧ��
         */
        static int delServer(CServerDelMsg& insServerDelMsg);

        /**
         * \brief ����ӷ�����
         * \param void
         * \return 0 �ɹ�-1ʧ��
         */ 
        static int addSubServer();

        /**
         * \brief ɾ���ӷ�����
         * \param void
         * \return 0 �ɹ�-1ʧ��
         */
        static int delSubServer();

};

#endif /* SERVERMNG_H_HEADER_INCLUDED_BCE185FC */


