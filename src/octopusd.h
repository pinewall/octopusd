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
 *  \brief ʹ���̺�̨��damon��ʽ����
 *  \return <0 ���� >=0 �ɹ� 
 */
int daemon_init(void);

/**
 *  \brief ����ϵͳ����fcntl(2)���ļ����ö�д��
 *  
 *  fcntl(2)�������ø�ʽ:<br>
 *      int fcntl( int fd, int cmd, struct * flock );
 *  <ul> 
 *  <li> ����:
 *      <ol>
 *      <li> F_SETLK    ��ȡ���ͷ�flcok�ṹָ��������������ɹ���������EACCES��EAGAIN
 *      <li> F_SETLKW   ��ȡ���ͷ�flock�ṹָ��������������ɹ�����ý��̱�����
 *      <li> F_GETLK    ��ȡflock�ṹָ�������Ƿ���ڣ��Լ���ؽ���pid�������͵���Ϣ
 *      </ol>
 *  <li> struct flock           //�ļ��������ṹ
 *      <ul>
 *      <li> short l_type;      //F_RDLCK, F_WRLCK, F_UNLCK �ֱ��Ƕ�����д��������
 *      <li> short l_whence;    //SEEK_SET, SEEK_CUR, SEEK_END �ļ�ƫ����ʼ����λ�ã��ļ�ͷ����ǰ���ļ�β
 *      <li> off_t l_start;     //relative starting offset in bytes,ƫ���ֽ���
 *      <li> off_t l_len;       //#bytes; 0 means until end-of-file, ���ĳ��ȣ�0Ϊ�����ļ�
 *      <li> pid_t l_pid;       //pid returned by F_GETLK, ��ʹ��F_GETLK����ʱ������������̵�pid
 *      </ul>
 *  </ul> 
 */
int lock_reg (int fd, int cmd, int type, off_t offset, int whence, off_t len);

/**
 *  \brief ���ƽ��̽���һ��ʵ�����еĺ���,ʹ��һ���ļ�д���������˳�������Ȼ�ͷ�
 *  \param filepath ���ڽ����ļ������ļ���,���ļ������ض�,�����������id
 *  \param appname  ���г��������,һ����argv[0]�õ�
 *  \return 0 ִ�м����ɹ� ����������ɹ���, ��ǰ���̱�ǿ����ֹ
 */
int run_single(char * filepath, char * appname,  int iLogFlag);

/**
 *  \brief �źŴ�����
 *  \brief �����յ����ź�
 */
void sighandler(int signum);


/**
 *  \brief �����������̵߳�״̬
 *  \param void
 *  \return check 0��ʾȫ���˳��� 1��ʾ����; set�޷���ֵ
 */
int checkChildExitStat();
void setChildExitStat();
int checkMyExitStat(int iMyID);
void setMyExitStat(int iMyID, int iStatus);

/**
 *  \brief �������ӵ��߳�
 *  \param void
 *  \return ʼ�շ���NULL
 */
void * pthread_connect(void *p);

/**
 *  \brief ���������Ϣ���̺߳���
 *  \param void
 *  \return ʼ�շ���NULL
 */
void * pthread_recv(void *p);

/**
 *  \brief �����͵��̺߳���
 *  \param void
 *  \return ʼ�շ���NULL
 */
void * pthread_send(void *p);

/**
 *  \brief �����͵��̺߳���(���Զ���)
 *  \param void
 *  \return ʼ�շ���NULL
 */
void* pthread_sendError(void *p);

/**
 *  \brief ����ά�����̺߳���
 *  \param void
 *  \return ʼ�շ���NULL
 */
void * pthread_maintain_p(void *p);  //process
void * pthread_maintain_r(void *p); //routine
void * pthread_maintain_f(void * p);  //routinuefailduty

/**
 *  \brief ����ʧ�ܶ��� 1.��IP����Ļָ� 2.��ЧIP�����ɾ��
 *  \param void
 *  \return ʼ�շ���NULL
 */
void * pthread_check(void *p); //routine

#endif // OCTOPUSD_H_HEADER_INCLUDED_BD617B65

