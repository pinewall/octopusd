//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: Maintain.h,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef MAINTAIN_H_HEADER_INCLUDED_BCE1B2D4
#define MAINTAIN_H_HEADER_INCLUDED_BCE1B2D4

#include "ComFun.h"
#include "LogMng.h"
#include "ConfigMng.h"
#include "QueueMng.h"
#include "ServerMng.h"
#include "ShareMem.h"
#include "Mobileinterface.h"

const int SizeOfShareMem = 100;

// 维护类，承担维护功能和用户增强功能
class CMaintain
{
public:        
       CMaintain();
       ~CMaintain();

        
       //在共享内存中的偏移量
       enum MaintainFlag   
       {
                ExitState = 0,  //程序是否退出 0否 1是
                LogMode = 1,   //日志级别 0 1 2 3
                LogModule = 2,  //模块号 LogModule0表示需要打印所有模块 其他的分别代表配置的站点
                ReloadFailDuty = 3,  //重载失败日志 0不处理,-1 表示所有任务 -2表示序号为0的文件 其他正数日志的文件序号
                PrintCfg = 4,  //打印配置文件信息 0 1
                PrintServerStatus = 5, //测试各个服务器的状态，并将结果打印到文件 0 1
                RefreshCommonCfg = 6,  //刷新配置文件信息 0 1
                RefreshSiteInfo = 7,  //刷新站点信息 0 1
                PrintQCurInfo = 8, //打印队列的情况信息 0 1
                RefreshLogModule = 9,  //刷新日志模块信息
                PrintThread2Pid = 10,  //打印一下线程ID和pid的对应关系
                SaveDutyQueueWhenExit = 11,  //当退出系统时,是否保存队列中的任务到失败文件中
                ForceCloseAllSocket = 12,   //强制关闭所有的接收socket链接
                ClearErrorIPPort = 13,   //清除掉所有的错误IP记录
                ForceCloseAllSocketFD = 14,   //强制关闭描述符从10-1024的所有描述符，可能有很大的风险，注意只能在确认已经泄露完fd时使用
                ClearDisabledDuty = 15, //关闭那些已经在分发队列中去掉的任务
                QueueSafeMode = 16,  // 如果其用这个模式，将会丢弃那些已经被修改的文件的分发，同时不再转移那些已经恢复的IP
                MaxFlag = 17
       };

        /**
         * \brief 维护模块的主调度进程
         * \param void
         * \return void
         */
        void process();

        /**
         * \brief 维护模块的初始化处理,包括共享内存的创建
         * \param void
         * \return 0 成功 -1失败
         */
        int init();

        /**
         * \brief 连接到共享内存
         * \param void
         * \return 0 成功 -1失败
         */
        int connect();

        /**
         * \brief 删除共享内存
         * \param void
         * \return 0 成功 -1失败
         */
        int delShareMem();

        /**
         * \brief 修改共享内存的值
         * \param eSeq 偏移量 iValue共享内存需要设置成的地址
         * \return void
         */
        void setFlag(MaintainFlag eSeq, int iValue);

        /**
         * \brief 获得共享内存的值
         * \param eSeq 偏移量
         * \return void
         */
        int getFlag(MaintainFlag eSeq);

        /**
         * \brief 程序是否要求退出
         * \param Null
         * \return 0否 1要
         */
        int  isExitProgram();

        /**
         * \brief 将发送失败的任务，从文件中，再次导入发送列表
         * \param iLogSeq 日志文件的序号 为了统一处理 -1表示序号为0的文件，而其他正数表示文件的序号
         * \return void
         * \other 重新加载失败文件，由于失败文件中10000个任务，
                  因此，执行此命令，须注意不能使队列一下在过长
                  由于这个操作要久一点才能加载到内存，因此最好在server较为清闲的时候执行         
         */
        void reloadFailDuty(const int iLogSeq);

        /**
         * \brief 打印配置文件信息(公共配置 服务器 站点)
         * \param void
         * \return void
         */
        void printCfg();

        /**
         * \brief 测试各个服务器的状态，并将结果打印到文件
         * \param void
         * \return void
         */
        void printServerStatus();

        /**
         * \brief 刷新配置文件信息(公共配置 服务器 站点)
         * \param void
         * \return void
         * \other 重新读取配置文件，并不是所有的配置项都可以重新读取的，根据级别制定
         */
        int refreshCommonCfg();
        int refreshSiteInfo();

        /**
         * \brief 打印队列的当前情况信息
         * \param void
         * \return void
         */
        void printQCurInfo();

        /**
         * \brief 打印队列的统计信息
         * \param void
         * \return void
         */
        void printQStatInfo();

        //获得队列的情况
         int getQueueInfo();

         /**
         * \brief 刷新日志类的模块信息
         * \param void
         * \return void
         */
        void refreshLogModule();
         
        /**
         * \brief 例行处理函数
         * \param void
         * \return void
         */
        void routine();
        void routineFailDuty();
        
        /**
         * \brief 检查最近的失败文件是否还存在，如果存在则报一次警
         * \param void
         * \return 0 不存在 1存在
         */
        int checkFailDutyExist();

        /**
         * \brief 发送消息给手机列表中的数据
         * \param sInfo 要发送的消息
         * \return void
         */
        void sendMobileMsg(string sInfo);

        /**
         * \brief 打印所有共享到屏幕
         * \param void
         * \return void
         */
        void printScreen();

        /**
         * \brief 打印线程ID和ps进程id的对应关系
         * \param void
         * \return void
         */
        void printThread2Pid();

         /**
         * \brief 强制关掉所有连接
         * \param void
         * \return void
         */
        void closeAllSocket();

private:
        //共享内存的基址 生成算法 (Homedir+bin) + 0
        char* m_pShmBaseAddr;

        //例行处理的时间控制
        time_t m_iLastQStatTime;  //上一次打印统计信息的时间
        time_t m_iLastDealFailDutyTime;  //上一次处理失败任务的时间    
        time_t m_iLastBackupLogTime;  //上一次检查备份日志的时间
        time_t m_iLastCheckRunStatusTime;  //上一次检查运行状态的时间
        time_t m_iLastCheckErrorIPTime; //上一次检查失败IP是否恢复的时间

};

#endif /* MAINTAIN_H_HEADER_INCLUDED_BCE1B2D4 */

