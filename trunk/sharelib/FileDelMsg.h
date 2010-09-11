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

//文件删除消息[没有纳入消息长度和类型]
class CFileDelMsg
{
public:
        CFileDelMsg();
        ~CFileDelMsg();

        //初始化字符串信息的结束符
        void sureValidSrt();

        //获得这个类的可供打印信息
        string getPrintInfo();

        //站点名称
        char  m_sSiteName[LenOfSiteName];

        //目标的相对路径
        char  m_sDestPathFile[LenOfPathFileName];

        //消息的发送时间
        char  m_sSendedTime[LenOfTime];        
};


//文件移动或者拷贝消息[没有纳入消息长度和类型]
class CFileMvCpMsg
{
public:
        CFileMvCpMsg();
        ~CFileMvCpMsg();

        //初始化字符串信息的结束符
        void sureValidSrt();

        //获得这个类的可供打印信息
        string getPrintInfo();

        //目标站点名称
        char  m_sSiteName[LenOfSiteName];

        //目标的相对路径
        char  m_sDestPathFile[LenOfPathFileName];

        //源文件的路径,源站点名称+相对路径的形式，例如new:/inc/picinpic.htm
        char m_sSrcPathFile[LenOfPathFileName];

        //消息的发送时间
        char  m_sSendedTime[LenOfTime];        
};

#endif /* FILEDELMSG_H_HEADER_INCLUDED_BCE1E925 */

