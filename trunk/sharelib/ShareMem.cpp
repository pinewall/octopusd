
#include "ShareMem.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>


Magic_ShareMem::Magic_ShareMem(void)
{
}


Magic_ShareMem::~Magic_ShareMem()
{
}


int Magic_ShareMem::GetShm(key_t tKey,int iSize,int iMode)
{
        m_tShmKey = tKey;
        m_iShmSize = iSize;
        
	if ((m_iShmId = shmget(m_tShmKey, m_iShmSize, IPC_CREAT | IPC_EXCL | iMode)) < 0)	//try to create
	{
		if (errno != EEXIST)
			return -1;

           if ((m_iShmId = shmget(m_tShmKey, m_iShmSize, iMode)) < 0)
		return -1;
			
		m_bCreate = false;
		return 1;
	}
	m_bCreate = true;
	return 0;
}


int  Magic_ShareMem::AttachShm()
{
  if ((m_pchShm = (char*)shmat(m_iShmId, NULL, 0)) < 0)
		return -1;

  return 0;
}


int Magic_ShareMem::DetachShm()
{
	if (shmdt(m_pchShm) < 0)
		return -1;
   return 0;
}


int Magic_ShareMem::RemoveShm()
{
	if (shmctl(m_iShmId, IPC_RMID, NULL) < 0)
		return -1;
	return 0;
}


void Magic_ShareMem::InitShm()
{
	memset(m_pchShm, 0, m_iShmSize);
}
