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

//章鱼侧添加服务器的消息[没有纳入消息长度和类型]
class CServerAddMsg
{
public:
        CServerAddMsg();
        ~CServerAddMsg();

        //消息的打印信息
        string getPrintInfo();

        //初始化字符串信息的结束符
        void sureValidSrt();

        //站点名称
        char m_sSiteName[LenOfSiteName];

        //服务器IP
        char m_sServerIP[LenOfIPAddr];

        //端口
        int  m_iPort;

        //远端还是近端,已经废弃,为了保持接口不变,暂时保留
        int  m_isRemote;

        //消息的时间
        char m_sTime[LenOfTime];
};

#endif /* SERVERADDMSG_H_HEADER_INCLUDED_BCE1B9C2 */

