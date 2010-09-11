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

//�����������ļ���Ϣ
class CComCfg
{
      public:
        CComCfg();
        CComCfg(const CComCfg& right);
        virtual ~CComCfg();
        CComCfg& operator=(const CComCfg& right);

        //�������ӵ�IP�����б������ | ����
        char m_sLoginIP[1024];
        char m_sServerFlag[1024];

        //����ʱ����ģʽ��0 �ͼ��𣬳�����־  1 ����һ�����߼�ִ����־
        // 2 ������ϸ�Ĳ������� 3 ������Ϣ������
        int m_iLogMode;

        //���ܵ��ļ���Ȩ�ޣ�������ʽ�������б�֤����ȷ�ԣ�ϵͳ��chmodִ�У�����chmod 755 /a/xyz.exe
        //Ϊ��Ĭ��Ȩ��Ϊ666��-rw-rw-rw-)����Ҫ��������CGI�ã���֤����пɶ���Ȩ�ޣ��󲿷��ļ������޸Ĵ�ָ
        char m_sFileModeOption[16];

        //�ļ��ϴ��ɹ���Ҫ��Ҫɾ���ļ�, 0 ��ʾ��Ҫɾ��, 1��ʾҪɾ��
        //����ʵ�������Ƚϸ���,���Ե�ǰ����ֻ���ڷַ���������ֻ��1̨����ʱ��������
        int m_iDelFileAfterUpload;

        //�ļ�Ϊ0�ֽ�ʱ���ϴ�ʱ��Ҫ��Ҫ�ַ���0��Ҫ�ٷַ��ˣ�1Ҫ�ַ�
        int m_iIfUploadZeroFile;

        // �����ϴ��������ļ���С�������˴�С������ַ���Ĭ��Ϊ0��������,��λΪKbyte����10M,д��10000
        int m_iMaxLimitOfUploadFile;

        //Э������ 1��ʾÿ����Ϣ�ζ�Ҫȷ����Ϣ 0��ʾ������Ϣһȷ��
        int m_iProtocolType;

        //ʧ��ʱ�����Դ�����0��ʾ������
        int m_iMaxNrOfRetry;

        //�ﵽ���Դ�������Ȼʧ�ܵ������Ƿ���Ҫд��־��0�� 1��
        int m_iIfFailLogFlag;

        //����������������Ϣ������0��ʾ������
        int m_iMaxNrOfQueueDuty;

        //������ٸ�������ͳɹ�������0��ʾ����¼�ͱ�������ɹ�������
        int m_iMaxNrOfSaveLastSucDuty;

        //��������ÿ����ô�ӡһ���̹߳�����ͳ����Ϣ,��λs, 0����ӡ
        int m_iTimeOfThreadStat;

        //��������ÿ����ü�鲢�Զ��ָ�ʧ�ܵ�����,��λs��0��ִ��
        int m_iTimeOfFailDutyRetry;

        //��������ÿ����ü����һ������״���Լ�����������״��, 0��ʾ�����
        int m_iTimeOfCheckRunStatus;

        //�������񣬴����˶���������Ϊ���쳣���ͻᱨ���������������TimeOfCheckRunStatus
        int m_iMaxNrOfDutyToAlarm;

        //�ַ�ѡ������ 0�������·ַ��ļ�; 1�ַ���������Ϣ��Դ�����ݿ�; 2��Դ�������ļ�
        int m_iDistributeFlag;

        //��DistributeFlag=1��Ч�����ݿ��������ã����û�����룬��עӵ�
        char m_sDBHost[LenOfNormal];
        char m_sDBUser[LenOfNormal];
        char m_sDBPassword[LenOfNormal];

        //�ṩ�����ַ�Ͷ˿�
        char m_sServerIP[LenOfIPAddr];
        int m_iServerPort;

        //�߳������ƣ���λ����
        int m_iRecvNrOfThread;
        int m_iSendNrOfThread;
        int m_iSendErrorNrOfThread;

        //�ɽ��յ������������0��ʾ������
        int m_iMaxNrOfCnt;

        //������Ϣ�ĳ�ʱʱ�䣬��λ��
        int m_iRecvMsgTimeout;
        int m_iRecvFileTimeout;

        //�������ƣ���ʾÿ����ֻ�ܷ��Ͷ���K�ֽڣ�0��ʾ�����ƣ�ע�ⵥλΪ KB,��СΪ10K
        int m_iSendFlowCtrl;
};

#endif /* COMCFG_H_HEADER_INCLUDED_BCE1EA59 */

