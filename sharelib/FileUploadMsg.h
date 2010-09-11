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

//章鱼侧的文件上传消息[没有纳入消息长度和类型,以及数据库]
class CFileUploadMsg
{
public:
        CFileUploadMsg();
        ~CFileUploadMsg();

        //初始化字符串信息的结束符
        void sureValidSrt();

        //获得这个类的可供打印信息
        string getPrintInfo();

        enum UploadFileType
        {
                FileName = 0x1,  //文件名称
                FileData = 0x2     //文件流
        };

        //站点名称
        char    m_sSiteName[LenOfSiteName];

        //文件名(包括路径)
        char m_sDestPathFile[LenOfPathFileName];

        //消息的发送时间
        char m_sSendedTime[LenOfTime];

        //数据流的类型
        char    m_cDataType;

        //保留字段
        char    m_sReserve[3];

        //数据流的长度
        int      m_iDataLength;
        
};

#endif /* FILEUPLOADMSG_H_HEADER_INCLUDED_BCE1B37A */

