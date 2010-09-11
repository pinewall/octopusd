//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ConfigMng.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************
#ifndef CONFIGMNG_H_HEADER_INCLUDED_BCE1C7C8
#define CONFIGMNG_H_HEADER_INCLUDED_BCE1C7C8

#include "ComFun.h"
#include "Config.h"

#include "ComCfg.h"
#include "SiteInfo.h"
#include "ServerGroupInfo.h"

#include "LogMng.h"

//�����ļ�������
class CConfigMng
{
public:
        CConfigMng();
        virtual ~CConfigMng();

        /**
         * \brief ��ʼ�������ļ�
         * \param void
         * \return 0 �ɹ� -1 ����
         */
        int init();

        /**
         * \brief ��ӡ������Ϣ�������ļ�
         * \param void
         * \return void
         */
        void print();

        /**
         * \brief ��ӡ������Ϣ�������ļ�
         * \param sSiteName վ������ sServerIP������IP iPort�˿�
         * \return 0 ���� -1������
         */
        int checkServerExist(string sSiteName, string sServerIP, int iPort);

        /**
         * \brief ����վ�����ƣ����վ���rootĿ¼
         * \param sSiteName վ�������
         * \return վ���rootĿ¼�����Ϊ""���ʾ����ʧ��
         */
        string getSiteRootDir(string sSiteName);

        /**
         * \brief �����ݿ��л�����ݿ�
         * \param void
         * \return 0 �ɹ� -1 ����
         */
        int readInfoFromDB(string sSiteId);

        /**
         * \brief ���վ����Ϣ, �����̲߳���ȫ,����Ҫ����������
         * \param void
         * \return 0 �ɹ� -1 ����
         */
        void getSiteMap(map<string, CSiteInfo>& m_mapSite);
        void getSiteServer(vector<CServerInfo>& vecServerInfo, const string& sSiteName);
        
        //����������
        CComCfg m_tComCfg;

        //������
        CThreadMutex  m_insShareMutex;

        //�������б�
        map<string, CServerInfo> m_mapServer;
        
        //���������б�
        map<string, CServerGroupInfo> m_mapGroup;
        
        //վ����Ϣ�б�
        map<string, CSiteInfo> m_mapSite;
};

#endif /* CONFIGMNG_H_HEADER_INCLUDED_BCE1C7C8 */

