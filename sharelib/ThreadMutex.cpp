//  ************************************************************************
//  *  This is the source of an commercial software; you can't redistribute 
//  *  it and/or modify it without the author's permit. All rights reserved.
//  *  You may get support by contact support@tencent.com if you really need.
//  *                                                                       
//  *    $RCSfile: ThreadMutex.cpp,v $
//  *    $Revision: 1.0 $
//  *    $Author: kevinliu $
//  *    $Name:  $
//  *    $Date: 2005/09/07 $
//  *    $ModifidList: $date $yourname $Comment
//  ***********************************************************************

#include "ThreadMutex.h"

CThreadMutex::CThreadMutex()
{
         pthread_mutex_init(&m_threadMutex, NULL);
}

CThreadMutex::~CThreadMutex()
{
        pthread_mutex_destroy(&m_threadMutex);
}

void CThreadMutex::setLock()
{
        pthread_mutex_lock(&m_threadMutex);
}

void CThreadMutex::unLock()
{
        pthread_mutex_unlock(&m_threadMutex);
}

