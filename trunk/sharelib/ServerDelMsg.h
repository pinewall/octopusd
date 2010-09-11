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

//章鱼侧删除服务器的消息[没有纳入消息长度和类型]
class CServerDelMsg
{
public:
        CServerDelMsg();
        ~CServerDelMsg();

        //消息的打印信息
        string getPrintInfo();

        //初始化字符串信息的结束符
        void sureValidSrt();

        //站定名称
        char m_sSiteName[LenOfSiteName];

        //站点IP
        char m_sServerIP[LenOfIPAddr];

        //端口
        int  m_iPort; 

        //消息的时间
        char m_sTime[LenOfTime];
};

#endif /* SERVERDELMSG_H_HEADER_INCLUDED_BCE18528 */

