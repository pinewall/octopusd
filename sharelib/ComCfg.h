//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *
//  *    $RCSfile: ComCfg.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef COMCFG_H_HEADER_INCLUDED_BCE1EA59
#define COMCFG_H_HEADER_INCLUDED_BCE1EA59

#include "ComFun.h"

//公共的配置文件信息
class CComCfg
{
      public:
        CComCfg();
        CComCfg(const CComCfg& right);
        virtual ~CComCfg();
        CComCfg& operator=(const CComCfg& right);

        //允许连接的IP配置列表，多个以 | 隔开
        char m_sLoginIP[1024];
        char m_sServerFlag[1024];

        //启动时日至模式，0 低级别，常规日志  1 出现一定的逻辑执行日志
        // 2 包含详细的操作日至 3 包含消息类码流
        int m_iLogMode;

        //接受的文件的权限，数字形式，请自行保证其正确性，系统用chmod执行，比如chmod 755 /a/xyz.exe
        //为空默认权限为666（-rw-rw-rw-)，主要用来部署CGI用，保证其具有可读性权限，大部分文件无需修改此指
        char m_sFileModeOption[16];

        //文件上穿成功后要不要删除文件, 0 表示不要删除, 1表示要删除
        //由于实现起来比较复杂,所以当前开关只能在分发下属机器只有1台机器时才起作用
        int m_iDelFileAfterUpload;

        //文件为0字节时，上传时还要不要分发，0不要再分发了，1要分发
        int m_iIfUploadZeroFile;

        // 允许上传的最大的文件大小，超过此大小，不予分发，默认为0，不限制,单位为Kbyte，如10M,写成10000
        int m_iMaxLimitOfUploadFile;

        //协议类型 1表示每个消息段都要确认消息 0表示整个消息一确认
        int m_iProtocolType;

        //失败时的重试次数，0表示无限制
        int m_iMaxNrOfRetry;

        //达到重试次数，仍然失败的任务，是否需要写日志，0否 1是
        int m_iIfFailLogFlag;

        //队列任务中最大的消息个数，0表示无限制
        int m_iMaxNrOfQueueDuty;

        //保存多少个最近发送成功的任务，0表示不记录和保存最近成功的任务
        int m_iMaxNrOfSaveLastSucDuty;

        //例行任务：每隔多久打印一次线程工作的统计信息,单位s, 0不打印
        int m_iTimeOfThreadStat;

        //例行任务：每隔多久检查并自动恢复失败的任务,单位s，0不执行
        int m_iTimeOfFailDutyRetry;

        //例行任务：每隔多久检查检查一下运行状况以及下属机器的状况, 0表示不检查
        int m_iTimeOfCheckRunStatus;

        //队列任务，存在了多少任务认为有异常，就会报警，报警间隔依赖TimeOfCheckRunStatus
        int m_iMaxNrOfDutyToAlarm;

        //分发选项配置 0不再向下分发文件; 1分发服务器信息来源于数据库; 2来源于配置文件
        int m_iDistributeFlag;

        //当DistributeFlag=1有效，数据库服务的配置，如果没有密码，请注拥�
        char m_sDBHost[LenOfNormal];
        char m_sDBUser[LenOfNormal];
        char m_sDBPassword[LenOfNormal];

        //提供服务地址和端口
        char m_sServerIP[LenOfIPAddr];
        int m_iServerPort;

        //线程数控制，单位个数
        int m_iRecvNrOfThread;
        int m_iSendNrOfThread;
        int m_iSendErrorNrOfThread;

        //可接收的最大连接数，0表示无限制
        int m_iMaxNrOfCnt;

        //接收消息的超时时间，单位秒
        int m_iRecvMsgTimeout;
        int m_iRecvFileTimeout;

        //流量控制，表示每秒钟只能发送多少K字节，0表示不限制，注意单位为 KB,最小为10K
        int m_iSendFlowCtrl;
};

#endif /* COMCFG_H_HEADER_INCLUDED_BCE1EA59 */

