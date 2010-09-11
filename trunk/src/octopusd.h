//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: octopusd.h $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef OCTOPUSD_H_HEADER_INCLUDED_BD617B65
#define OCTOPUSD_H_HEADER_INCLUDED_BD617B65

#include "ComFun.h"
#include "ProtocolMng.h"
#include "ServerMng.h"
#include "QueueMng.h"
#include "ConfigMng.h"
#include "LogMng.h"
#include "TCPSocket.h"
#include "Maintain.h"

/**
 *  \brief 使进程后台以damon方式运行
 *  \return <0 错误 >=0 成功 
 */
int daemon_init(void);

/**
 *  \brief 调用系统函数fcntl(2)给文件设置读写锁
 *  
 *  fcntl(2)基本调用格式:<br>
 *      int fcntl( int fd, int cmd, struct * flock );
 *  <ul> 
 *  <li> 命令:
 *      <ol>
 *      <li> F_SETLK    获取或释放flcok结构指定的锁，如果不成功立即返回EACCES或EAGAIN
 *      <li> F_SETLKW   获取或释放flock结构指定的锁，如果不成功则调用进程被阻塞
 *      <li> F_GETLK    获取flock结构指定的锁是否存在，以及相关进程pid，锁类型的信息
 *      </ol>
 *  <li> struct flock           //文件锁参数结构
 *      <ul>
 *      <li> short l_type;      //F_RDLCK, F_WRLCK, F_UNLCK 分别是读锁，写锁，无锁
 *      <li> short l_whence;    //SEEK_SET, SEEK_CUR, SEEK_END 文件偏移起始计算位置，文件头，当前，文件尾
 *      <li> off_t l_start;     //relative starting offset in bytes,偏移字节数
 *      <li> off_t l_len;       //#bytes; 0 means until end-of-file, 锁的长度，0为整个文件
 *      <li> pid_t l_pid;       //pid returned by F_GETLK, 在使用F_GETLK命令时返回相关锁进程的pid
 *      </ul>
 *  </ul> 
 */
int lock_reg (int fd, int cmd, int type, off_t offset, int whence, off_t len);

/**
 *  \brief 控制进程仅有一个实例运行的函数,使用一个文件写锁，进程退出，锁自然释放
 *  \param filepath 由于建立文件锁的文件名,该文件将被截断,并被填入进程id
 *  \param appname  运行程序的名称,一般由argv[0]得到
 *  \return 0 执行加锁成功 如果加锁不成功则, 当前进程被强制中止
 */
int run_single(char * filepath, char * appname,  int iLogFlag);

/**
 *  \brief 信号处理函数
 *  \brief 进程收到的信号
 */
void sighandler(int signum);


/**
 *  \brief 检查和设置子线程的状态
 *  \param void
 *  \return check 0表示全部退出了 1表示还有; set无返回值
 */
int checkChildExitStat();
void setChildExitStat();
int checkMyExitStat(int iMyID);
void setMyExitStat(int iMyID, int iStatus);

/**
 *  \brief 接收连接的线程
 *  \param void
 *  \return 始终返回NULL
 */
void * pthread_connect(void *p);

/**
 *  \brief 处理接收消息的线程函数
 *  \param void
 *  \return 始终返回NULL
 */
void * pthread_recv(void *p);

/**
 *  \brief 处理发送的线程函数
 *  \param void
 *  \return 始终返回NULL
 */
void * pthread_send(void *p);

/**
 *  \brief 处理发送的线程函数(重试队列)
 *  \param void
 *  \return 始终返回NULL
 */
void* pthread_sendError(void *p);

/**
 *  \brief 处理维护的线程函数
 *  \param void
 *  \return 始终返回NULL
 */
void * pthread_maintain_p(void *p);  //process
void * pthread_maintain_r(void *p); //routine
void * pthread_maintain_f(void * p);  //routinuefailduty

/**
 *  \brief 处理失败队列 1.慢IP任务的恢复 2.无效IP任务的删除
 *  \param void
 *  \return 始终返回NULL
 */
void * pthread_check(void *p); //routine

#endif // OCTOPUSD_H_HEADER_INCLUDED_BD617B65

