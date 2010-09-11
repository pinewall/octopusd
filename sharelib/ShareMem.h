//ShareMem.h

#ifndef __SHAREMEM__HH__
#define __SHAREMEM__HH__ 

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


/**
 * \file ShareMem.hh
 * \brief declare operation function for share memory.
 */
 
/**
 * Used to create share memory.
 */ 

class Magic_ShareMem
{
public:
	Magic_ShareMem(void);
	virtual ~Magic_ShareMem();
	
	int  GetShm(key_t tKey,int iSize, int iMode = 0666);
	int AttachShm();
	int DetachShm();
	int RemoveShm();
        inline bool isCreate()
        {
	    return m_bCreate;
        }

        inline char* Address(void)
	{
	    return m_pchShm;
        }

	void InitShm();
	
protected:
	 key_t m_tShmKey;
	 int m_iShmSize;
	int m_iShmId;
        bool m_bCreate;
	char* m_pchShm;
};

#endif


