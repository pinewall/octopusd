//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: QueueMng.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: Kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef QUEUEMNG_H_HEADER_INCLUDED_BCE1B9DA
#define QUEUEMNG_H_HEADER_INCLUDED_BCE1B9DA

#include "ComFun.h"
#include "LogMng.h"
#include "TCPSocket.h"
#include "RecvingDuty.h"
#include "SendDuty.h"
#include "ThreadMutex.h"

const int MaxSeqOfFailFileName = 9999;
const int MaxSeqOfFailRecord = 10000;
const unsigned int MaxTimesToErrorIPList = 3;
const unsigned int DefaultMaxOfErrorQueue = 300000; //30w

//队列调度类，包括正在接收队列，发送队列，正在发送的队列
class CQueueMng
{
public:
        CQueueMng();
        virtual ~CQueueMng();

        //队列类型
        enum QueueType
        {
                RecvingQueue = 0,
                SendQueue = 1,
                SendingQueue = 2,
                LastSucQueue = 3
        };

        //队列类型
        enum PutSendType
        {
                AddServerInfo = 0,
                Direct = 1
        };

        /**
         * \brief 队列的初始化
         * \param void
         * \return 0 成功其他失败
         */ 
         int init();

        /**
         * \brief 测试消息是否需要处理
         * \param sSiteName 站点名称
         * \param sTime 消息的时间
         * \param sPathFile 文件的名称带路径
         * \param insRecvingDuty 如果加入了正在接收队列,则是加入的任务
         * \return 0 表示取消掉了较老的任务或者没有老的同等任务，需要处理本次消息
                      -1 表示找到更新的消息，本次消息直接放弃并返回
         */ 
        int check(CRecvingDuty& insRecvingDuty);

        /**
         * \brief 判断自己任务的状态是否为正常态normal
         * \param pID 线程ID
         * \param eQueueType 队列类型
         * \return 0正常态 其他非正常态
         */
         int judgeNormal(int pID, QueueType eQueueType);

        /**
         * \brief 判断自己任务的状态为结束态
         * \param pID 线程ID
         * \param eQueueType 队列类型
         * \return 0成功 其他失败
         */
         int setOver(int pID, QueueType eQueueType);

        /**
         * \brief 取和设置发送队列
         * \ param iType 0代表处理常规任务，1代表处理失败任务
         * \param insSendDuty 发送任务
         * \return 0 成功其他失败
         */ 
        int getDutyFSendQ(int iType, CSendDuty& insSendingDuty);  // 2007-05-21 add itype
        int putDuty2SendQ(CSendDuty& insSendDuty, PutSendType ePutSendType, int iRecvFileSize = 0);

        /**
         * \brief 将任务发送最近成功队列
         * \param insSendDuty 发送任务
         * \return 0 成功其他失败
         */ 
        int putDuty2LastSucQ(CSendDuty& insLastSucDuty);

        /**
         * \brief 记录发送失败的任务
         * \param insFailDuty 任务的内容
         * \return void
         * \other 记录文件每个文件只记录10000个任务，
                  文件名为failduty.xxxx（4位序号，例如0000，启动的时候控制），
                  循环使用,到了9999返回来重新使用的时候，发现0000还没有被处理掉，则删除之
         */
        void putFailDuty2File(CSendDuty& insFailDuty);
        void getFailDutyFFile2DutyQ(int iFileSeq);

        /**
         * \brief 打印队列的当前时间的状况信息
         * \param void
         * \return void
         */ 
        void printCurInfo();
        void getCurInfo(string& sInfo);

        /**
         * \brief 打印队列的统计信息，间隔一段时间打印一次
         * \param void
         * \return void
         */ 
        void printStatInfo();
        void printStatInfoXML();

        //返回队列中的个数，当超长时，要打印队列中的信息
        int getQueueInfo();

         /**
         * \brief 初始化一下统计队列
         * \param iThreadType iThreadID
         * \return void
         */ 
        void initThreadStatInfo(int iThreadType, int iThreadID);

        /**
         * \brief 将队列中的所有任务都保存到文件中
         * \param void
         * \return void
         */ 
         void saveDutyQueue2File();

        /**
         * \brief 获得列表
         * \param m_mapIPPort 存放错误ip,port的列表
         * \return void
         */
        void getErrorIPPort(map<string, unsigned int>& m_mapIPPort);

        /**
         * \brief 当任务成功发送后，进行失败IP的计数器处理
                  添加是放在putDuty2SendQ中，失败时的处理
         * \param ipport 为空 则表示清空，其他的为具体的IP
         * \return void
         */
        void clearErrorIPPort(string ipport = "");

        //为了不持有长时间的锁，每次只处理一个，处理的频率由外部来控制
        void mvErrorQ2SendQ();

        //清除无效任务
        void clearDisabledDuty();

private:
        //===正在接收的任务队列<threadID, duty>===
        map<int, CRecvingDuty> m_mapRecvingQ;
        
        //===任务索引产生的序列号,队列为空时,即重新从0开始计数,不会出现长到最大值的情况===
        //int m_iDutySeq;
        //任务队列<dutyIndex, duty> (正常的任务队列)
        //map<int, CSendDuty> m_mapSendQ;
        //任务队列的索引<site +File + IP + port, dutyIndex>
        //map<string, int> m_mapSendQIndex;
        list<CSendDuty> m_lstSendQ;

        // BEGIN 2007-05-21 kevinliu add
        //发送失败的IP_端口,只是为了查找方便用map，后面的int没有意义
        map<string, unsigned int> m_mapErrorIPPort; 
        
        //任务队列<duty> 由于失败队列有可能比较大，map的操作会变得比较慢，因此采用list的方式
        //尽管无法再合并相同的任务，但是可以靠页面的时间来实际上并不重复发送
        list<CSendDuty> m_lstSendErrorQ;
        // END 2007-05-21 kevinliu add

        //===正在发送的任务队列<threadID, duty>===
        map<int, CSendDuty> m_mapSendingQ;

        //===最近发送成功发送成功的任务的队列　大小DefaultQueueSize  <site +File, 时间>===
        map<string, string> m_mapLastSucQ;                
        //保存LastSuc队列中进入队列的顺序,供清理队列的时候用 <seq, site+file>
        int m_iLastSucSeq;
        map<int, string> m_mapLastSucIndexQ;

        //===进行共享操作时，需要加锁处理===
        CThreadMutex m_insMutex;

        //===记录错误日志时的序号 文件名 记录序号
        int m_iFileSeq; 
        int m_iRecordSeq; 

        //===统计相关信息
        long m_lTotalSizeOfRecvFile;
        int m_iNrOfRecvUploadFile;
        int m_iNrOfSendUploadFile;
        int m_iNrOfRecvDelFile;
        int m_iNrOfSendDelFile;
        int m_iNrOfRecvMvFile;
        int m_iNrOfSendMvFile;
        int m_iNrOfRecvCpFile;
        int m_iNrOfSendCpFile;
        map<int, int> m_mapRecvWorkTime;  //<threadid, worktimes>
        map<int, int> m_mapSendWorkTime;  //<threadid, worktimes>
        
};

#endif /* QUEUEMNG_H_HEADER_INCLUDED_BCE1B9DA */

