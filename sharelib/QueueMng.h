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

//���е����࣬�������ڽ��ն��У����Ͷ��У����ڷ��͵Ķ���
class CQueueMng
{
public:
        CQueueMng();
        virtual ~CQueueMng();

        //��������
        enum QueueType
        {
                RecvingQueue = 0,
                SendQueue = 1,
                SendingQueue = 2,
                LastSucQueue = 3
        };

        //��������
        enum PutSendType
        {
                AddServerInfo = 0,
                Direct = 1
        };

        /**
         * \brief ���еĳ�ʼ��
         * \param void
         * \return 0 �ɹ�����ʧ��
         */ 
         int init();

        /**
         * \brief ������Ϣ�Ƿ���Ҫ����
         * \param sSiteName վ������
         * \param sTime ��Ϣ��ʱ��
         * \param sPathFile �ļ������ƴ�·��
         * \param insRecvingDuty ������������ڽ��ն���,���Ǽ��������
         * \return 0 ��ʾȡ�����˽��ϵ��������û���ϵ�ͬ��������Ҫ��������Ϣ
                      -1 ��ʾ�ҵ����µ���Ϣ��������Ϣֱ�ӷ���������
         */ 
        int check(CRecvingDuty& insRecvingDuty);

        /**
         * \brief �ж��Լ������״̬�Ƿ�Ϊ����̬normal
         * \param pID �߳�ID
         * \param eQueueType ��������
         * \return 0����̬ ����������̬
         */
         int judgeNormal(int pID, QueueType eQueueType);

        /**
         * \brief �ж��Լ������״̬Ϊ����̬
         * \param pID �߳�ID
         * \param eQueueType ��������
         * \return 0�ɹ� ����ʧ��
         */
         int setOver(int pID, QueueType eQueueType);

        /**
         * \brief ȡ�����÷��Ͷ���
         * \ param iType 0������������1������ʧ������
         * \param insSendDuty ��������
         * \return 0 �ɹ�����ʧ��
         */ 
        int getDutyFSendQ(int iType, CSendDuty& insSendingDuty);  // 2007-05-21 add itype
        int putDuty2SendQ(CSendDuty& insSendDuty, PutSendType ePutSendType, int iRecvFileSize = 0);

        /**
         * \brief ������������ɹ�����
         * \param insSendDuty ��������
         * \return 0 �ɹ�����ʧ��
         */ 
        int putDuty2LastSucQ(CSendDuty& insLastSucDuty);

        /**
         * \brief ��¼����ʧ�ܵ�����
         * \param insFailDuty ���������
         * \return void
         * \other ��¼�ļ�ÿ���ļ�ֻ��¼10000������
                  �ļ���Ϊfailduty.xxxx��4λ��ţ�����0000��������ʱ����ƣ���
                  ѭ��ʹ��,����9999����������ʹ�õ�ʱ�򣬷���0000��û�б����������ɾ��֮
         */
        void putFailDuty2File(CSendDuty& insFailDuty);
        void getFailDutyFFile2DutyQ(int iFileSeq);

        /**
         * \brief ��ӡ���еĵ�ǰʱ���״����Ϣ
         * \param void
         * \return void
         */ 
        void printCurInfo();
        void getCurInfo(string& sInfo);

        /**
         * \brief ��ӡ���е�ͳ����Ϣ�����һ��ʱ���ӡһ��
         * \param void
         * \return void
         */ 
        void printStatInfo();
        void printStatInfoXML();

        //���ض����еĸ�����������ʱ��Ҫ��ӡ�����е���Ϣ
        int getQueueInfo();

         /**
         * \brief ��ʼ��һ��ͳ�ƶ���
         * \param iThreadType iThreadID
         * \return void
         */ 
        void initThreadStatInfo(int iThreadType, int iThreadID);

        /**
         * \brief �������е��������񶼱��浽�ļ���
         * \param void
         * \return void
         */ 
         void saveDutyQueue2File();

        /**
         * \brief ����б�
         * \param m_mapIPPort ��Ŵ���ip,port���б�
         * \return void
         */
        void getErrorIPPort(map<string, unsigned int>& m_mapIPPort);

        /**
         * \brief ������ɹ����ͺ󣬽���ʧ��IP�ļ���������
                  ����Ƿ���putDuty2SendQ�У�ʧ��ʱ�Ĵ���
         * \param ipport Ϊ�� ���ʾ��գ�������Ϊ�����IP
         * \return void
         */
        void clearErrorIPPort(string ipport = "");

        //Ϊ�˲����г�ʱ�������ÿ��ֻ����һ���������Ƶ�����ⲿ������
        void mvErrorQ2SendQ();

        //�����Ч����
        void clearDisabledDuty();

private:
        //===���ڽ��յ��������<threadID, duty>===
        map<int, CRecvingDuty> m_mapRecvingQ;
        
        //===�����������������к�,����Ϊ��ʱ,�����´�0��ʼ����,������ֳ������ֵ�����===
        //int m_iDutySeq;
        //�������<dutyIndex, duty> (�������������)
        //map<int, CSendDuty> m_mapSendQ;
        //������е�����<site +File + IP + port, dutyIndex>
        //map<string, int> m_mapSendQIndex;
        list<CSendDuty> m_lstSendQ;

        // BEGIN 2007-05-21 kevinliu add
        //����ʧ�ܵ�IP_�˿�,ֻ��Ϊ�˲��ҷ�����map�������intû������
        map<string, unsigned int> m_mapErrorIPPort; 
        
        //�������<duty> ����ʧ�ܶ����п��ܱȽϴ�map�Ĳ������ñȽ�������˲���list�ķ�ʽ
        //�����޷��ٺϲ���ͬ�����񣬵��ǿ��Կ�ҳ���ʱ����ʵ���ϲ����ظ�����
        list<CSendDuty> m_lstSendErrorQ;
        // END 2007-05-21 kevinliu add

        //===���ڷ��͵��������<threadID, duty>===
        map<int, CSendDuty> m_mapSendingQ;

        //===������ͳɹ����ͳɹ�������Ķ��С���СDefaultQueueSize  <site +File, ʱ��>===
        map<string, string> m_mapLastSucQ;                
        //����LastSuc�����н�����е�˳��,��������е�ʱ���� <seq, site+file>
        int m_iLastSucSeq;
        map<int, string> m_mapLastSucIndexQ;

        //===���й������ʱ����Ҫ��������===
        CThreadMutex m_insMutex;

        //===��¼������־ʱ����� �ļ��� ��¼���
        int m_iFileSeq; 
        int m_iRecordSeq; 

        //===ͳ�������Ϣ
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

