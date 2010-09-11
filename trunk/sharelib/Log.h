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

#ifndef LOG_H_HEADER_INCLUDED_BCE1FB82
#define LOG_H_HEADER_INCLUDED_BCE1FB82

#include "ComFun.h"

const int DefaultSizeOfSingleLog = 5000000;
const int DefaultNrOfLogFile = 10;

class CLog
{
public:
        CLog();
        virtual ~CLog();
        
        /**
         * \brief 初始化日志文件
         * \param void
         * \return 0 成功 -1 出错
         */
        int init(const string& sFile, long logsize = DefaultSizeOfSingleLog, int lognum = DefaultNrOfLogFile);

        /**
         * \brief 写日志文件信息
         * \param sFormat 日至信息
         * \return void
         */
        void writeLog(const char* sFormat, ...);
        void writeLog(int iRes, va_list ap, const char* sFormat );  //iRes无用

        /**
         * \brief 写二进制日志文件信息
         * \param pBuffer 日至信息 iLen 长度
         * \return void
         */
        void wirteBinLog(const char* pBuffer, const int iLen);

        /**
         * \brief 判断是否需要备份日志
         * \param void
         * \return void
         */
        void shiftFiles();

private:
        string m_sFileName;
        long m_lLogSize;
        int m_iNrOfLog;

};

#endif /* LOG_H_HEADER_INCLUDED_BCE1FB82 */

