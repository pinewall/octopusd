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

// ά���࣬�е�ά�����ܺ��û���ǿ����
class CMaintain
{
public:        
       CMaintain();
       ~CMaintain();

        
       //�ڹ����ڴ��е�ƫ����
       enum MaintainFlag   
       {
                ExitState = 0,  //�����Ƿ��˳� 0�� 1��
                LogMode = 1,   //��־���� 0 1 2 3
                LogModule = 2,  //ģ��� LogModule0��ʾ��Ҫ��ӡ����ģ�� �����ķֱ�������õ�վ��
                ReloadFailDuty = 3,  //����ʧ����־ 0������,-1 ��ʾ�������� -2��ʾ���Ϊ0���ļ� ����������־���ļ����
                PrintCfg = 4,  //��ӡ�����ļ���Ϣ 0 1
                PrintServerStatus = 5, //���Ը�����������״̬�����������ӡ���ļ� 0 1
                RefreshCommonCfg = 6,  //ˢ�������ļ���Ϣ 0 1
                RefreshSiteInfo = 7,  //ˢ��վ����Ϣ 0 1
                PrintQCurInfo = 8, //��ӡ���е������Ϣ 0 1
                RefreshLogModule = 9,  //ˢ����־ģ����Ϣ
                PrintThread2Pid = 10,  //��ӡһ���߳�ID��pid�Ķ�Ӧ��ϵ
                SaveDutyQueueWhenExit = 11,  //���˳�ϵͳʱ,�Ƿ񱣴�����е�����ʧ���ļ���
                ForceCloseAllSocket = 12,   //ǿ�ƹر����еĽ���socket����
                ClearErrorIPPort = 13,   //��������еĴ���IP��¼
                ForceCloseAllSocketFD = 14,   //ǿ�ƹر���������10-1024�������������������кܴ�ķ��գ�ע��ֻ����ȷ���Ѿ�й¶��fdʱʹ��
                ClearDisabledDuty = 15, //�ر���Щ�Ѿ��ڷַ�������ȥ��������
                QueueSafeMode = 16,  // ����������ģʽ�����ᶪ����Щ�Ѿ����޸ĵ��ļ��ķַ���ͬʱ����ת����Щ�Ѿ��ָ���IP
                MaxFlag = 17
       };

        /**
         * \brief ά��ģ��������Ƚ���
         * \param void
         * \return void
         */
        void process();

        /**
         * \brief ά��ģ��ĳ�ʼ������,���������ڴ�Ĵ���
         * \param void
         * \return 0 �ɹ� -1ʧ��
         */
        int init();

        /**
         * \brief ���ӵ������ڴ�
         * \param void
         * \return 0 �ɹ� -1ʧ��
         */
        int connect();

        /**
         * \brief ɾ�������ڴ�
         * \param void
         * \return 0 �ɹ� -1ʧ��
         */
        int delShareMem();

        /**
         * \brief �޸Ĺ����ڴ��ֵ
         * \param eSeq ƫ���� iValue�����ڴ���Ҫ���óɵĵ�ַ
         * \return void
         */
        void setFlag(MaintainFlag eSeq, int iValue);

        /**
         * \brief ��ù����ڴ��ֵ
         * \param eSeq ƫ����
         * \return void
         */
        int getFlag(MaintainFlag eSeq);

        /**
         * \brief �����Ƿ�Ҫ���˳�
         * \param Null
         * \return 0�� 1Ҫ
         */
        int  isExitProgram();

        /**
         * \brief ������ʧ�ܵ����񣬴��ļ��У��ٴε��뷢���б�
         * \param iLogSeq ��־�ļ������ Ϊ��ͳһ���� -1��ʾ���Ϊ0���ļ���������������ʾ�ļ������
         * \return void
         * \other ���¼���ʧ���ļ�������ʧ���ļ���10000������
                  ��ˣ�ִ�д������ע�ⲻ��ʹ����һ���ڹ���
                  �����������Ҫ��һ����ܼ��ص��ڴ棬��������server��Ϊ���е�ʱ��ִ��         
         */
        void reloadFailDuty(const int iLogSeq);

        /**
         * \brief ��ӡ�����ļ���Ϣ(�������� ������ վ��)
         * \param void
         * \return void
         */
        void printCfg();

        /**
         * \brief ���Ը�����������״̬�����������ӡ���ļ�
         * \param void
         * \return void
         */
        void printServerStatus();

        /**
         * \brief ˢ�������ļ���Ϣ(�������� ������ վ��)
         * \param void
         * \return void
         * \other ���¶�ȡ�����ļ������������е�������������¶�ȡ�ģ����ݼ����ƶ�
         */
        int refreshCommonCfg();
        int refreshSiteInfo();

        /**
         * \brief ��ӡ���еĵ�ǰ�����Ϣ
         * \param void
         * \return void
         */
        void printQCurInfo();

        /**
         * \brief ��ӡ���е�ͳ����Ϣ
         * \param void
         * \return void
         */
        void printQStatInfo();

        //��ö��е����
         int getQueueInfo();

         /**
         * \brief ˢ����־���ģ����Ϣ
         * \param void
         * \return void
         */
        void refreshLogModule();
         
        /**
         * \brief ���д�����
         * \param void
         * \return void
         */
        void routine();
        void routineFailDuty();
        
        /**
         * \brief ��������ʧ���ļ��Ƿ񻹴��ڣ����������һ�ξ�
         * \param void
         * \return 0 ������ 1����
         */
        int checkFailDutyExist();

        /**
         * \brief ������Ϣ���ֻ��б��е�����
         * \param sInfo Ҫ���͵���Ϣ
         * \return void
         */
        void sendMobileMsg(string sInfo);

        /**
         * \brief ��ӡ���й�����Ļ
         * \param void
         * \return void
         */
        void printScreen();

        /**
         * \brief ��ӡ�߳�ID��ps����id�Ķ�Ӧ��ϵ
         * \param void
         * \return void
         */
        void printThread2Pid();

         /**
         * \brief ǿ�ƹص���������
         * \param void
         * \return void
         */
        void closeAllSocket();

private:
        //�����ڴ�Ļ�ַ �����㷨 (Homedir+bin) + 0
        char* m_pShmBaseAddr;

        //���д����ʱ�����
        time_t m_iLastQStatTime;  //��һ�δ�ӡͳ����Ϣ��ʱ��
        time_t m_iLastDealFailDutyTime;  //��һ�δ���ʧ�������ʱ��    
        time_t m_iLastBackupLogTime;  //��һ�μ�鱸����־��ʱ��
        time_t m_iLastCheckRunStatusTime;  //��һ�μ������״̬��ʱ��
        time_t m_iLastCheckErrorIPTime; //��һ�μ��ʧ��IP�Ƿ�ָ���ʱ��

};

#endif /* MAINTAIN_H_HEADER_INCLUDED_BCE1B2D4 */

