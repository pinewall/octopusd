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

//任务队列中的任务类
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
                Error = -1,   //出错，需要结束任务
                Normal = 0,  //正常的发送状态
                Giveup = 1,   //有新的能替换掉本次任务，需要放弃本次任务
                Over = 2       //任务已经结束
        };
         
public:
        //任务的状态
        SendState m_iState;  
        
        //协议类型
        int m_iProtocolType;

        //站点名
        char m_sSiteName[LenOfSiteName];

        //在服务器上的路径 相对
        char m_sDestPathFile[LenOfPathFileName];

        //时间
        char m_sSendedTime[LenOfTime];
        
        //本地路径
        char m_sLocalPathFile[LenOfPathFileName];

        //本地文件生成的时间
        int    m_iLocalTime;  
        
        // 站点服务器ip地址
        char m_sServerIP[LenOfIPAddr];
        
        // 站点服务器端口号
        int    m_iServerPort;

        // 记录该消息出错次数，超过一定次数后就不发送该消息了。
        int    m_iNrOfFail;
};

#endif /* SENDDUTY_H_HEADER_INCLUDED_BCE19785 */

