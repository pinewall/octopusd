//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: LogMng.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef LOGMNG_H_HEADER_INCLUDED_BCE1B9B6
#define LOGMNG_H_HEADER_INCLUDED_BCE1B9B6

#include "Log.h"
#include "ComFun.h"

#include "ConfigMng.h"
#include "Maintain.h"

typedef int LogModule;

//��־�ļ�ʹ����
class CLogMng
{
public:

        enum LogMode
        {
                LogMode0 = 0,
                LogMode1 = 1,
                LogMode2 = 2,
                LogMode3 = 3
        };
        
        CLogMng();
         CLogMng(string sAppName);
        virtual ~CLogMng();

         /**
         * \brief ��ʼ����־������
         * \param sAppName Ӧ�ó�������ƣ���־���Ǹ���������֣����.log .debug .error
         * \return 0 �ɹ� -1 ����
         */
        int init(string sAppName);  

        /**
         * \brief д��־����,�ֱ�Ϊerror normal
         * \param ��־��Ϣ
         * \return void
         */
        void error(const char* sFormat, ...);
        void normal(const char* sFormat, ...);
        void runlog(const char* sFormat,  ...);


        /**
         * \brief д��־����,binary
         * \param sModuleName ģ������
         * \param insLogMode ��־ģʽ
         * \param pBuffer ��־��Ϣ
         * \param iSize ��־����
         * \return void
         */
        void bin(const char* psModuleName, LogMode insLogMode, const char* pBuffer, const int iSize); 

        /**
         * \brief д��־����,debug�ຯ��
         * \param ��־ģʽ ��־ģ�� ��־��Ϣ����ָ������Ĭ��
         * \return void
         */
        /*void debug(const char* sFormat, ...);
        void debug(LogMode insLogMode, const char* sFormat, ...);*/
        void debug(const char* psModuleName, LogMode insLogMode, const char* sFormat, ...);
        void debug(int iRes, va_list ap, const char* psModuleName, LogMode insLogMode, const char* sFormat);

        /**
         * \brief �ж��Ƿ���Ҫ��¼��־
         * \param sModuleName ģ������
         * \param insLogMode ��־ģʽ
         * \return -1����Ҫ 0 ��Ҫ
         */
        int check(const char* psModuleName, LogMode insLogMode);
        
        /**
         * \brief ת����־����Ϊ��־����
         * \param sModuleName ģ������
         * \return 0 ���� ��������
         */
        LogModule convertLogModule(const char* psModuleName);

        /**
         * \brief ˢ����־ģ����Ϣ
         * \param void
         * \return void
         */
        void refreshLogModule();

         /**
         * \brief ���ڱ�����־
         * \param void
         * \return void
         */
        void backuplogfile();

        //��־ģ����Ϣ
        map<string, string> m_mapLogModule;

private:
        //Ӧ�ó�������
        string m_sAppName;

        //debug��־
        CLog m_insDebugLog;

        //error��־
        CLog m_insErrorLog;

        //normal��־
        CLog m_insNormalLog;

         //run��־
        CLog m_objRunLog;

};

#endif /* LOGMNG_H_HEADER_INCLUDED_BCE1B9B6 */

