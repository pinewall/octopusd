//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ServerGroupInfo.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: Kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef SERVERGROUPINFO_H_HEADER_INCLUDED_BCE1B619
#define SERVERGROUPINFO_H_HEADER_INCLUDED_BCE1B619

#include "ComFun.h"

// ������������Ϣ
class CServerGroupInfo
{
      public:
        CServerGroupInfo();
        CServerGroupInfo(const CServerGroupInfo& right);
        virtual ~CServerGroupInfo();
        CServerGroupInfo& operator=(const CServerGroupInfo& right);

        // ����������
        string m_sName;

        // �����ڵķ������б�
        vector<string> m_vecServerList;
};

#endif /* SERVERGROUPINFO_H_HEADER_INCLUDED_BCE1B619 */

