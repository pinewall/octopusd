//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ThreadMutex.h,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#ifndef THREADMUTEX_H_HEADER_INCLUDED_BCE1A133
#define THREADMUTEX_H_HEADER_INCLUDED_BCE1A133

#include "pthread.h"

//线程锁
class CThreadMutex
{
public:
        CThreadMutex();
        virtual ~CThreadMutex();

        /**
         * \brief 加锁
         * \param void
         * \return void
         */
        void setLock();

        /**
         * \brief 解锁
         * \param void
         * \return void
         */
        void unLock();

private:
        pthread_mutex_t m_threadMutex;
};

#endif /* THREADMUTEX_H_HEADER_INCLUDED_BCE1A133 */

