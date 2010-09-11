//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: Log.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "Log.h"

CLog::CLog()
{
        m_sFileName = "";
        m_lLogSize = 0;
        m_iNrOfLog = 0;
}

CLog::~CLog()
{
}

int CLog::init(const string& sFile, long logsize, int lognum)
{
        if(logsize < 1 || lognum < 1)
        {
                return -1;
        }
        
        m_lLogSize = logsize;
        m_iNrOfLog = lognum;
        m_sFileName= sFile;

        return 0;
}

void CLog::writeLog(const char* sFormat, ...)
{
        //�����������
        va_list ap;
        va_start(ap, sFormat);

        //д���ļ�
        writeLog(0, ap, sFormat);
        
        //����д��
        va_end(ap);

        return;
}
void CLog::writeLog(int iRes, va_list ap, const char* sFormat )
{        
        //kevinliu test
        //return; //��д��־
        
        iRes = 0; //����,��Ϊ���ֺ�������
        //���ļ��������ļ�д
        FILE* fp;
        if ((fp = fopen(m_sFileName.c_str(), "a+")) == NULL)
        {
                return;
        }

        //ʱ����߳�ID
        fprintf(fp, "[%s][%06u] ", CComFun::GetExactCurDateTime().c_str(), (unsigned int)pthread_self());
        vfprintf(fp, sFormat, ap);
        fprintf(fp, "\n");
        fclose(fp);

        //�ж��ļ��Ƿ񳬴���Ҫת��
        //shiftFiles();

        return;
}

void CLog::wirteBinLog(const char* pBuffer, const int iLen)
{
        int i, iCurPos, iLineLen;
        char sHex[10], sChr[10];
        iLineLen = 12;
        iCurPos = 0;
        string sHexStr,sChrStr;
        while(1)
        {
                if (iCurPos >= iLen)
                {
                        break;
                }

                strcpy(sHex, "");
                strcpy(sChr, "");
                sHexStr="";
                sChrStr="";

                i = 0;
                while(1) 
                {
                        if ((iCurPos+i >= iLen) || (i >= iLineLen))
                        {
                                break;
                        }

                        sprintf(sHex, "%02x ", (unsigned char)pBuffer[i+iCurPos]);
                        sHexStr+=sHex;
                        if (pBuffer[i+iCurPos] < 32 || (unsigned char)pBuffer[i+iCurPos] > 127)
                        {
                                 sChrStr+=".";
                        }
                        else
                        {
                                sprintf(sChr, "%c", pBuffer[i+iCurPos]);
                                sChrStr+=sChr;
                        }
                        i++;
                }

                writeLog("%2d| %-24s| %-16s", iCurPos, sHexStr.c_str(), sChrStr.c_str());
                iCurPos += iLineLen;
        }

        return;
}

void CLog::shiftFiles()
{        
        struct stat stStat;
        if(stat(m_sFileName.c_str(), &stStat) < 0)
        {
                return;
        }
        if (stStat.st_size < m_lLogSize)
        {                
                return;
        }

        //�Ȱ���������ļ���ת�Ƴ�tmp�ļ�������ʡȥ�˷�ֹ�ڴ�������У��ļ��������Ŀ���
        //����ʱ��,Ҳ��ֹ����
        CComFun::mvFile(m_sFileName, m_sFileName + ".tmp");

        int iLoop = 0;
        char sOldLogFileName[1024];
        char sNewLogFileName[1024];
        for(iLoop = m_iNrOfLog - 2; iLoop >= 0; iLoop--)
        {
                if (iLoop == 0)
                {
                        //������ļ����Ǹղŵ��Ǹ��ļ�
                        sprintf(sOldLogFileName, "%s.tmp", m_sFileName.c_str());  
                }
                else
                {
                        sprintf(sOldLogFileName,"%s_%d", m_sFileName.c_str(), iLoop);
                }
                sprintf(sNewLogFileName,"%s_%d", m_sFileName.c_str(), iLoop + 1);      
                
                CComFun::mvFile(sOldLogFileName, sNewLogFileName); //�����жϷ���ֵ,������������
        }
        return;
}


