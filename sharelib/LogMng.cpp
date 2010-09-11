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

#include "LogMng.h"

extern string g_sHomeDir;
extern CConfigMng g_insConfigMng;
extern CMaintain g_insMaintain;

CLogMng::CLogMng()
{
        m_sAppName = "";
}

CLogMng::CLogMng(string sAppName)
{
        m_sAppName = sAppName;        
}

CLogMng::~CLogMng()
{
}

int CLogMng::init(string sAppName)
{
        //Ӧ�ó��������
        if ("" != sAppName)
        {
                m_sAppName = sAppName;
        }
        else
        {
                return -1;
        }

        //��ʼ��3����־�ļ�
        if (0 != m_insNormalLog.init(g_sHomeDir + "/log/"+ m_sAppName + ".log",
                DefaultSizeOfSingleLog, DefaultNrOfLogFile))
        {
                return -1;
        }

        if (0 != m_insErrorLog.init(g_sHomeDir + "/log/"+ m_sAppName + ".error",
                DefaultSizeOfSingleLog, DefaultNrOfLogFile))
        {
                return -1;
        }

        //debug��־��һ��������
        if (0 != m_insDebugLog.init(g_sHomeDir + "/log/"+ m_sAppName + ".debug", 
                DefaultSizeOfSingleLog, 2*DefaultNrOfLogFile))
        {
                return -1;
        }

        //run ��־
        if (0 != m_objRunLog.init(g_sHomeDir + "/log/"+ m_sAppName + ".run",
                DefaultSizeOfSingleLog, DefaultNrOfLogFile))
        {
                return -1;
        }

        //�ɹ������Ҫ,��Ӱ��������ʹ��
        m_mapLogModule.clear();
        string sPathFile = g_sHomeDir + "/cfg/" + "logmodule.cfg";        
        CComFun::readEqualValueFile(sPathFile, m_mapLogModule);

        return 0;
}

void CLogMng::normal(const char* sFormat,  ...)
{
        //�����������
        va_list ap;
        va_start(ap, sFormat);

        //д���ļ� normal debug
        m_insNormalLog.writeLog(0, ap, sFormat);
        debug(0, ap, "", LogMode0, sFormat);
        
        //����д��
        va_end(ap);

        return;
}

void CLogMng::error(const char* sFormat, ...)
{
        //�����������
        va_list ap;
        va_start(ap, sFormat);

        //д���ļ� error debug normal
        m_insNormalLog.writeLog(0, ap, sFormat);
        m_insErrorLog.writeLog(0, ap, sFormat);
        debug(0, ap, "", LogMode0, sFormat);
        
        //����д��
        va_end(ap);

        return;
}

void CLogMng::runlog(const char* sFormat,  ...)
{
        //ֱ���жϣ�����Լ����ֱ�Ӵ�ӡ
        //if (1 > g_insMaintain.getFlag(CMaintain::LogMode))
        //{
        //        return; 
        //}
        
        //�����������
        va_list ap;
        va_start(ap, sFormat);

        //д���ļ� run
        m_objRunLog.writeLog(0, ap, sFormat);

        //����д��
        va_end(ap);

        return;
}


void CLogMng::bin(const char* psModuleName, LogMode insLogMode, const char* pBuffer, const int iSize)
{
        if (0 == check(psModuleName, insLogMode))
        {
                m_insDebugLog.wirteBinLog(pBuffer, iSize); 
        }
        return;
}
/*
void CLogMng::debug(const char* sFormat, ...)
{
        //�����������
        va_list ap;
        va_start(ap, sFormat);

        //д���ļ� debug Ĭ��������ģ�� �� ��־����1
        debug(0, ap, "", LogMode1, sFormat);
        
        //����д��
        va_end(ap);

        return;
}

void CLogMng::debug(LogMode insLogMode, const char* sFormat, ...)
{
        //�����������
        va_list ap;
        va_start(ap, sFormat);

        //д���ļ� debug Ĭ������ģ��
        debug(0, ap, "", insLogMode, sFormat);
        
        //����д��
        va_end(ap);

        return;
}
*/

void CLogMng::debug(const char* psModuleName, LogMode insLogMode, const char* sFormat, ...)
{
        //�����������
        va_list ap;
        va_start(ap, sFormat);

        //д���ļ� debug
        debug(0, ap, psModuleName, insLogMode, sFormat);
        
        //����д��
        va_end(ap);
        
        return;
}

void CLogMng::debug(int iRes, va_list ap, const char* psModuleName, LogMode insLogMode, const char* sFormat)
{      
        iRes = 0;
        if (0 == check(psModuleName, insLogMode))
        {
                //д���ļ� debug
                m_insDebugLog.writeLog(0, ap, sFormat);
        }
        return;        
}

int CLogMng::check(const char* psModuleName, LogMode insLogMode)
{        
        int iLogModeFlag = g_insMaintain.getFlag(CMaintain::LogMode);
        if (LogMode0 == insLogMode)  //����д����Ҫ�Ǹ�normal��־��error��־��ͬ��дdebug��־���õ�
        {
                if (iLogModeFlag == LogMode0)  // �����־û�д�,����дdebug
                {
                        return -1;
                }
        }
        else
        {
                if (insLogMode > iLogModeFlag)   //�����־���𲻵�,��ֱ�ӷ���,����д��־
                {
                        return -1;
                }
        }
          
        int iLogModuleFlag = g_insMaintain.getFlag(CMaintain::LogModule);
        // 0 ��������ģ�� 
        if (0 != iLogModuleFlag)
        {
                 LogModule tmpLogModule;
                 tmpLogModule = convertLogModule(psModuleName);

                 // ����趨��ģ�飬ֻ�ܴ�ӡ�����Լ���ģ����Ϣ
                 if (iLogModuleFlag != tmpLogModule)
                 {
                        return -1;
                 }
        }

        return 0;
}

void CLogMng::refreshLogModule()
{
        string sPathFile = g_sHomeDir + "/cfg/" + "logmodule.cfg";
        m_mapLogModule.clear();
        CComFun::readEqualValueFile(sPathFile, m_mapLogModule);
        
        char sTmpStr[1024] = "";
        sprintf(sTmpStr, "\tModuleName\tModuleID\n");
        for(map<string, string>::iterator it = m_mapLogModule.begin();
                it != m_mapLogModule.end(); it++)
        {
                sprintf(sTmpStr, "%s\t%s\t\t%s\n", sTmpStr, it->first.c_str(), it->second.c_str());                
        }
                
        string myFile = g_sHomeDir + "/log/prt/logmodule.prt";
        FILE *fp = fopen(myFile.c_str(), "w");
        if (NULL == fp)
        {
                return;
        }

        fputs(sTmpStr, fp);
        fclose(fp);
        
        return;   
        return;
}

LogModule CLogMng::convertLogModule(const char* psModuleName)
{
        if ('\0' == psModuleName[0])
        {
                return 0;
        }
        string sTmp = string(psModuleName);
        if ("" != m_mapLogModule[sTmp])
        {
                return (LogModule)atoi(m_mapLogModule[sTmp].c_str());
        }
        return 0;
}

void CLogMng::backuplogfile()
{
        m_insDebugLog.shiftFiles();
        m_insErrorLog.shiftFiles();
        m_insNormalLog.shiftFiles();
        m_objRunLog.shiftFiles();  // add 07-30
        return;
}

