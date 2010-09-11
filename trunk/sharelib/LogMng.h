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

//日志文件使用类
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
         * \brief 初始化日志管理类
         * \param sAppName 应用程序的名称，日志都是根据这个名字，添加.log .debug .error
         * \return 0 成功 -1 出错
         */
        int init(string sAppName);  

        /**
         * \brief 写日志函数,分别为error normal
         * \param 日志信息
         * \return void
         */
        void error(const char* sFormat, ...);
        void normal(const char* sFormat, ...);
        void runlog(const char* sFormat,  ...);


        /**
         * \brief 写日志函数,binary
         * \param sModuleName 模块名称
         * \param insLogMode 日志模式
         * \param pBuffer 日志信息
         * \param iSize 日志长度
         * \return void
         */
        void bin(const char* psModuleName, LogMode insLogMode, const char* pBuffer, const int iSize); 

        /**
         * \brief 写日志函数,debug类函数
         * \param 日志模式 日志模块 日志信息，不指定会有默认
         * \return void
         */
        /*void debug(const char* sFormat, ...);
        void debug(LogMode insLogMode, const char* sFormat, ...);*/
        void debug(const char* psModuleName, LogMode insLogMode, const char* sFormat, ...);
        void debug(int iRes, va_list ap, const char* psModuleName, LogMode insLogMode, const char* sFormat);

        /**
         * \brief 判断是否需要记录日志
         * \param sModuleName 模块名称
         * \param insLogMode 日志模式
         * \return -1不需要 0 需要
         */
        int check(const char* psModuleName, LogMode insLogMode);
        
        /**
         * \brief 转化日志名称为日志代码
         * \param sModuleName 模块名称
         * \return 0 所有 其他代表
         */
        LogModule convertLogModule(const char* psModuleName);

        /**
         * \brief 刷新日志模块信息
         * \param void
         * \return void
         */
        void refreshLogModule();

         /**
         * \brief 定期备份日志
         * \param void
         * \return void
         */
        void backuplogfile();

        //日志模块信息
        map<string, string> m_mapLogModule;

private:
        //应用程序名称
        string m_sAppName;

        //debug日志
        CLog m_insDebugLog;

        //error日志
        CLog m_insErrorLog;

        //normal日志
        CLog m_insNormalLog;

         //run日志
        CLog m_objRunLog;

};

#endif /* LOGMNG_H_HEADER_INCLUDED_BCE1B9B6 */

