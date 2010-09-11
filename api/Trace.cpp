// Magic_Trace.cpp
#include "Trace.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>

using namespace std;

Magic_Trace*  Magic_Trace::m_trace = 0;

Magic_Trace* Magic_Trace::instance(void) 
{
    if(m_trace == 0) {
    	
    	m_trace = new Magic_Trace();
    }
    return m_trace;
	
}

void Magic_Trace::close(void)
{
     delete m_trace;	
     m_trace = 0;
}


Magic_Trace::Magic_Trace(void)
{
}

int Magic_Trace::Init(
  const string& tracefile,
  long lMaxLogSize,
  int  iMaxLogNum  
 )
{
    if(lMaxLogSize < 1 || iMaxLogNum < 1) {
	cerr<<"fail to init magic_trace."<<endl;
	return -1;
    }
    m_lLogSize = lMaxLogSize;
    m_iLogNum = iMaxLogNum;
    m_sfile = tracefile + string(".log");
    return 0;
}

Magic_Trace::~Magic_Trace() 
{

}

void Magic_Trace::setTraceInfo(
    long lLine,
    const char* sFile
  )
{
    m_lLine = lLine;
    if(sFile!=0) m_tracefile = sFile;

}
 

int Magic_Trace::ShiftFiles(
	char *sErrMsg
   )
{
  struct stat stStat;
  if(stat(m_sfile.c_str(), &stStat) < 0)
  {
	if (sErrMsg != NULL)
	{
		strcpy(sErrMsg, "Fail to get file status");
	}
	return -1;
  }

  if (stStat.st_size < m_lLogSize)
  {
	return 0;
  }

  
  char sLogFileName[1024];
 
  sprintf(sLogFileName,"%s_%d", m_sfile.c_str(), m_iLogNum-1);
  
  if (access(sLogFileName, F_OK) == 0)
  {
	if (remove(sLogFileName) < 0 ) {
	
		if (sErrMsg != NULL)
		{
			strcpy(sErrMsg, "Fail to remove oldest log file");
		}
		return -1;
	}
  }

  char sNewLogFileName[1024];
 
  for(int i = m_iLogNum-2; i >= 0; i--)
  {
	if (i == 0)
		sprintf(sLogFileName,"%s", m_sfile.c_str());
	else
		sprintf(sLogFileName,"%s_%d", m_sfile.c_str(), i);
			
	if (access(sLogFileName, F_OK) == 0)
	{
		sprintf(sNewLogFileName,"%s_%d", m_sfile.c_str(), i+1);
		if (rename(sLogFileName,sNewLogFileName) < 0 )
		{
         		if (sErrMsg != NULL)
			{
			strcpy(sErrMsg, "Fail to remove oldest log file");
			}
			return -1;
		}
	}
  }
  return 0;
}

int Magic_Trace::WriteMsg(
	char *sErrMsg, 
	const char *sFormat, 
	va_list ap
   )
{
	FILE  *pstFile;

   if ((pstFile = fopen(m_sfile.c_str(), "a+")) == NULL)
	{
		if (sErrMsg != NULL)
		{
			strcpy(sErrMsg, "Fail to open log file");
		}
		return -1;
	}

	fprintf(pstFile, "[%s] ", GetCurDateTimeStr());
	
	vfprintf(pstFile, sFormat, ap);
	
	fprintf(pstFile, "\n");

	fclose(pstFile);

	return ShiftFiles(sErrMsg);
}

void Magic_Trace::WriteBinTrace(const char *caBuffer,int iLen)
{ 
  WriteLog("[%s][%d][%s]",m_tracefile.c_str(),m_lLine,"print binary struct");    
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
       while(1) {
	   if ((iCurPos+i >= iLen) || (i >= iLineLen))
            {
                   break;
            }

            sprintf(sHex, "%02x ", (unsigned char)caBuffer[i+iCurPos]);
	    sHexStr+=sHex;
           if (caBuffer[i+iCurPos] < 32 || (unsigned char)caBuffer[i+iCurPos] > 127)
           {
		 sChrStr+=".";
           }
           else
          {
                sprintf(sChr, "%c", caBuffer[i+iCurPos]);
		sChrStr+=sChr;
          }
	        i++;
         }

      WriteLog("%2d| %-24s| %-16s", iCurPos, sHexStr.c_str(), sChrStr.c_str());
               // printf("%8d | %-48s| %-16s\n", iCurPos, sHex, sChr);
                iCurPos += iLineLen;
        }
}

void Magic_Trace::WriteTrace(const char*sFormat,...)
{
        va_list ap;
        va_start(ap,sFormat);
        char buf[TRACE_SIZE];
        vsnprintf(buf,TRACE_SIZE,sFormat,ap);
        va_end(ap);
        WriteLog("[%s][%d][%s]",m_tracefile.c_str(),m_lLine,buf);
}


void Magic_Trace::WriteLog(const char *sFormat, ...)
{
	va_list ap;
	
	if (getenv("MAGIC_STDERR") != 0)
	{
		printf("[%s] ", GetCurDateTimeStr());
	
		va_start(ap, sFormat);
		vprintf(sFormat, ap);
		va_end(ap);

		printf("\n");
	}

	va_start(ap, sFormat);
	WriteMsg(NULL, sFormat, ap);
	va_end(ap);
}


char *Magic_Trace::GetCurDateTimeStr()
{

    time_t	iCurTime;
    time(&iCurTime);
    static char s[50];
    struct tm *curr = localtime(&iCurTime);

	if (curr->tm_year > 50)
	{
		sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d", 
					curr->tm_year+1900, curr->tm_mon+1, curr->tm_mday,
					curr->tm_hour, curr->tm_min, curr->tm_sec);
	}
	else
	{
		sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d", 
					curr->tm_year+2000, curr->tm_mon+1, curr->tm_mday,
					curr->tm_hour, curr->tm_min, curr->tm_sec);
	}
				
	return s;
}


