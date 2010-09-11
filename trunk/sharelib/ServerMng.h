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

//发送服务器的列表
class CServerMng
{
public:
        /**
         * \brief 添加服务器
         * \param insServerAddMsg 增加的服务器信息
         * \return 0 成功-1失败
         */ 
        static int addServer(CServerAddMsg& insServerAddMsg);

        /**
         * \brief 删除服务器
         * \param insServerDelMsg 删除的服务器信息
         * \return 0 成功-1失败
         */
        static int delServer(CServerDelMsg& insServerDelMsg);

        /**
         * \brief 添加子服务器
         * \param void
         * \return 0 成功-1失败
         */ 
        static int addSubServer();

        /**
         * \brief 删除子服务器
         * \param void
         * \return 0 成功-1失败
         */
        static int delSubServer();

};

#endif /* SERVERMNG_H_HEADER_INCLUDED_BCE185FC */


