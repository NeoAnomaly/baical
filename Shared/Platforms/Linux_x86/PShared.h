//******************************************************************************
// Copyright 2013  Zheltovskiy Andrey                                          *
//                                                                             *
//   Licensed under the Apache License, Version 2.0 (the "License");           *
//   you may not use this file except in compliance with the License.          *
//   You may obtain a copy of the License at                                   *
//                                                                             *
//       http://www.apache.org/licenses/LICENSE-2.0                            *
//                                                                             *
//  Unless  required  by  applicable  law  or  agreed  to in writing, software *
//  distributed  under the License is distributed on an "AS IS" BASIS, WITHOUT *
//  WARRANTIES  OR  CONDITIONS OF ANY KIND, either express or implied. See the *
//  License  for  the  specific language governing permissions and limitations *
//  under the License.                                                         *
//                                                                             *
//******************************************************************************
// Implementation of shared & named memory, memory is shared in process bounds *
//******************************************************************************

#ifndef PSHARED_H_AZH
#define PSHARED_H_AZH


////////////////////////////////////////////////////////////////////////////////
//Headers dependencies:
#include <unistd.h>
#include <fcntl.h>           /* For O_* constants */
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */


struct sShared
{
    int     iMFD;
    sem_t  *hSemaphore;
    size_t  szName;
    char   *pSemName;
    char   *pMemName;
};

typedef sShared hShared;


////////////////////////////////////////////////////////////////////////////////
//Shared_Close
static __attribute__ ((unused)) tBOOL Shared_Close(hShared *i_pShared)
{
    if (NULL == i_pShared)
    {
        return FALSE;
    }

    if (SEM_FAILED != i_pShared->hSemaphore)
    {
        sem_wait(i_pShared->hSemaphore);
    }

    if (0 <= i_pShared->iMFD)
    {
        close(i_pShared->iMFD);
        i_pShared->iMFD = -1;
        shm_unlink(i_pShared->pMemName);
    }

    if (SEM_FAILED != i_pShared->hSemaphore)
    {
        int l_iRes = -1;
        l_iRes = sem_close(i_pShared->hSemaphore);
        l_iRes = sem_unlink(i_pShared->pSemName);
        i_pShared->hSemaphore = SEM_FAILED;
        UNUSED_ARG(l_iRes);
    }

    if (i_pShared->pSemName)
    {
        free(i_pShared->pSemName);
        i_pShared->pSemName = NULL;
    }

    if (i_pShared->pMemName)
    {
        free(i_pShared->pMemName);
        i_pShared->pMemName = NULL;
    }

    i_pShared->szName = 0;

    free(i_pShared);
    i_pShared = NULL;

    return FALSE;
}//Shared_Close


////////////////////////////////////////////////////////////////////////////////
//Shared_Create
static __attribute__ ((unused)) hShared *Shared_Create(const tXCHAR *i_pName,
                                                       const void   *i_pData,
                                                       tUINT16       i_wSize
                                                      )
{
    hShared *l_pReturn  = NULL;
    tBOOL    l_bResult  = TRUE;
    tBOOL    l_bRelease = FALSE;
    void    *l_pBuffer  = NULL;


    if (    (NULL == i_pName)
         || (NULL == i_pData)
         || (0    >= i_wSize)
       )
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }

    l_pReturn = (hShared*)malloc(sizeof(hShared));
    if (NULL == l_pReturn)
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }

    memset(l_pReturn, 0, sizeof(hShared));
    l_pReturn->hSemaphore = SEM_FAILED;
    l_pReturn->iMFD       = -1;

    l_pReturn->szName    = strlen(i_pName) + 64;
    l_pReturn->pSemName  = (char*)malloc(l_pReturn->szName);
    l_pReturn->pMemName  = (char*)malloc(l_pReturn->szName);

    if (    (NULL == l_pReturn->pSemName)
         || (NULL == l_pReturn->pMemName)
       )
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }

    ////////////////////////////////////////////////////////////////////////////
    //create ssemaphore and own it
    sprintf(l_pReturn->pSemName, "/LAUS_%d_%s", getpid(), i_pName);

    l_pReturn->hSemaphore = sem_open(l_pReturn->pSemName, O_CREAT | O_EXCL, 0666, 0);
    if (SEM_FAILED == l_pReturn->hSemaphore)
    {
        l_bResult  = FALSE;
        goto l_lblExit;
    }

    l_bRelease = TRUE;

    ////////////////////////////////////////////////////////////////////////////
    //share memory
    sprintf(l_pReturn->pMemName, "/LAUM_%d_%s", getpid(), i_pName);

    l_pReturn->iMFD = shm_open(l_pReturn->pMemName, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (0 > l_pReturn->iMFD)
    {
        l_bResult  = FALSE;
        goto l_lblExit;
    }

    if (0 != ftruncate(l_pReturn->iMFD, (off_t)i_wSize))
    {
        l_bResult  = FALSE;
        goto l_lblExit;
    }

    l_pBuffer = mmap(0,
                     (size_t)i_wSize,
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED,
                     l_pReturn->iMFD,
                     0
                    );

    if (    (NULL == l_pBuffer)
         || (MAP_FAILED == l_pBuffer)
       )
    {
        l_bResult  = FALSE;
        goto l_lblExit;
    }

    memcpy(l_pBuffer, i_pData, (size_t)i_wSize);

    if (0 != munmap(l_pBuffer, (size_t)i_wSize))
    {
    }


l_lblExit:
    if (l_bRelease)
    {
        sem_post(l_pReturn->hSemaphore);
    }

    if (FALSE == l_bResult)
    {
        Shared_Close(l_pReturn);
        l_pReturn = NULL;
    }

    return l_pReturn;
}//Shared_Create


////////////////////////////////////////////////////////////////////////////////
//Shared_Read
static __attribute__ ((unused)) tBOOL Shared_Read(const tXCHAR  *i_pName,
                                                  void          *o_pData,
                                                  tUINT16        i_wSize
                                                 )
{
    tBOOL       l_bResult  = TRUE;
    size_t      l_szName   = 0;
    char       *l_pName    = NULL;
    tBOOL       l_bRelease = FALSE;
    void       *l_pBuffer  = NULL;
    const int   l_i10ms    = 10000;
    int         l_iWait    = l_i10ms * 25;

    int         l_iMFD     = -1;
    sem_t      *l_hSem     = SEM_FAILED;
    struct stat l_sStat;


    if (    (NULL == i_pName)
         || (NULL == o_pData)
         || (0    >= i_wSize)
       )
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }

    l_szName = strlen(i_pName) + 64;
    l_pName = (char*)malloc(l_szName);

    if (NULL == l_pName)
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }

    ////////////////////////////////////////////////////////////////////////////
    //open semaphore
    sprintf(l_pName, "/LAUS_%d_%s", getpid(), i_pName);

    //check is it existing or not
    l_hSem = sem_open(l_pName, O_CREAT | O_EXCL);
    if (SEM_FAILED != l_hSem) //it wasn't existing = ERROR
    {
        sem_close(l_hSem);
        sem_unlink(l_pName);
        l_hSem = SEM_FAILED;

        l_bResult  = FALSE;
        goto l_lblExit;
    }

    l_hSem = sem_open(l_pName, O_CREAT);
    if (SEM_FAILED == l_hSem)
    {
        l_bResult  = FALSE;
        goto l_lblExit;
    }

    while (0 < l_iWait)
    {
        if (0 == sem_trywait(l_hSem))
        {
            l_bRelease = TRUE;
            break;
        }
        else
        {
            usleep(l_i10ms); //10 ms
            l_iWait -= l_i10ms;
        }
    }

    if (FALSE == l_bRelease)
    {
        l_bResult  = FALSE;
        goto l_lblExit;
    }

    ////////////////////////////////////////////////////////////////////////////
    //open shared memory object
    sprintf(l_pName, "/LAUM_%d_%s", getpid(), i_pName);

    l_iMFD = shm_open(l_pName, O_RDONLY, 0444);

    if (0 > l_iMFD)
    {
        l_bResult  = FALSE;
        goto l_lblExit;
    }

    memset(&l_sStat, 0, sizeof(l_sStat));
    if (-1 == fstat(l_iMFD, &l_sStat))
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }

    if ((size_t)l_sStat.st_size > (size_t)i_wSize)
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }

    l_pBuffer = mmap(0,
                     (size_t)l_sStat.st_size,
                     PROT_READ,
                     MAP_SHARED,
                     l_iMFD,
                     0
                    );

    if (    (NULL == l_pBuffer)
         || (MAP_FAILED == l_pBuffer)
       )
    {
        l_bResult  = FALSE;
        goto l_lblExit;
    }

    memcpy(o_pData, l_pBuffer, (size_t)l_sStat.st_size);

    if (0 == munmap(l_pBuffer, (size_t)l_sStat.st_size))
    {
        l_pBuffer = NULL;
    }


l_lblExit:
    if (l_pName)
    {
        free(l_pName);
        l_pName = NULL;
    }

    if (0 <= l_iMFD)
    {
        close(l_iMFD);
        l_iMFD = -1;
    }

    if (l_bRelease)
    {
        sem_post(l_hSem);
    }

    if (SEM_FAILED != l_hSem)
    {
        sem_close(l_hSem);
        l_hSem = SEM_FAILED;
    }

    return l_bResult;
}//Shared_Read

#endif //PSHARED_H_AZH
