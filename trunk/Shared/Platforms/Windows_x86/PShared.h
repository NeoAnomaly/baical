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
// Implementation is not thread safe !                                         *
//******************************************************************************

#pragma once


struct sShared
{
    HANDLE  hMemory;
    HANDLE  hMutex;
};

typedef sShared hShared;


////////////////////////////////////////////////////////////////////////////////
//Shared_Close
static tBOOL Shared_Close(hShared *i_pShared)
{
    if (NULL == i_pShared)
    {
        return FALSE;
    }

    if (i_pShared->hMutex)
    {
        CloseHandle(i_pShared->hMutex);
        i_pShared->hMutex = NULL;
    }

    if (i_pShared->hMemory)
    {
        CloseHandle(i_pShared->hMemory);
        i_pShared->hMemory = NULL;
    }

    delete i_pShared;
    i_pShared = NULL;

    return TRUE;
}//Shared_Close


////////////////////////////////////////////////////////////////////////////////
//Shared_Create
static hShared *Shared_Create(const tXCHAR *i_pName, 
                              tUINT8       *i_pData, 
                              tUINT16       i_wSize
                             )
{
    hShared *l_pReturn  = NULL;
    tBOOL    l_bResult  = TRUE;
    DWORD    l_dwLen    = 0;
    wchar_t *l_pName    = NULL;
    BOOL     l_bRelease = FALSE;
    tUINT8  *l_pBuffer  = NULL;


    if (    (NULL == i_pName)
         || (NULL == i_pData)
         || (0    >= i_wSize)
       )
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }

    l_pReturn = new hShared;
    if (NULL == l_pReturn)
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }

    memset(l_pReturn, 0, sizeof(hShared));

    l_dwLen = (DWORD)wcslen(i_pName) + 128;
    l_pName = new wchar_t[l_dwLen];

    if (NULL == l_pName)
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }

    ////////////////////////////////////////////////////////////////////////////
    //create mutex and own it
    swprintf_s(l_pName, l_dwLen, L"Local\\m%d%s", GetCurrentProcessId(), i_pName);

    l_pReturn->hMutex = CreateMutex(NULL, TRUE, l_pName);
    if (    (NULL == l_pReturn->hMutex)
         || (ERROR_ALREADY_EXISTS == GetLastError())
       )
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }

    l_bRelease = TRUE;

    ////////////////////////////////////////////////////////////////////////////
    //create shared memory object
    swprintf_s(l_pName, l_dwLen, L"Local\\f%d%s", GetCurrentProcessId(), i_pName);

    l_pReturn->hMemory = CreateFileMappingW(INVALID_HANDLE_VALUE, // use paging file
                                            NULL,                 // default security
                                            PAGE_READWRITE,       // read/write access
                                            0,                    // maximum object size (high-order tUINT32)
                                            i_wSize,              // maximum object size (low-order tUINT32)
                                            l_pName               // name of mapping object
                                           );     
    
    if (    (NULL == l_pReturn->hMemory)
         || (ERROR_ALREADY_EXISTS == GetLastError())
       )
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }
    
    l_pBuffer = (tUINT8*)MapViewOfFile(l_pReturn->hMemory,    
                                       FILE_MAP_ALL_ACCESS,
                                       0,
                                       0,
                                       i_wSize
                                      );
    
    if (NULL == l_pBuffer)
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }
    
    __try
    {
        memcpy(l_pBuffer, i_pData, i_wSize);
    }
    
    __except (   GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR 
                ? EXCEPTION_EXECUTE_HANDLER 
                : EXCEPTION_CONTINUE_SEARCH
                )
    {
        l_bResult = FALSE;
        goto l_lblExit;
    }

l_lblExit:
    if (l_pName)
    {
        delete [] l_pName;
        l_pName = NULL;
    }

    if (l_pBuffer)
    {
        UnmapViewOfFile(l_pBuffer);
        l_pBuffer = NULL;
    }

    if (l_bRelease)
    {
        ReleaseMutex(l_pReturn->hMutex);
    }

    if (FALSE == l_bResult)
    {
        Shared_Close(l_pReturn);
    }

    return l_pReturn;
}//Shared_Create


////////////////////////////////////////////////////////////////////////////////
//Shared_Get
static tBOOL Shared_Read(const tXCHAR  *i_pName, 
                         tUINT8        *o_pData,
                         tUINT16        i_wSize
                        )
{
    hShared *l_pShared  = NULL;
    tBOOL    l_bReturn  = TRUE;
    DWORD    l_dwLen    = 0;
    wchar_t *l_pName    = NULL;
    BOOL     l_bRelease = FALSE;
    tUINT8  *l_pBuffer  = NULL;

    if (    (NULL == i_pName)
         || (NULL == o_pData)
         || (0    >= i_wSize)
       )
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    l_pShared = new hShared;
    if (NULL == l_pShared)
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    memset(l_pShared, 0, sizeof(hShared));

    l_dwLen = (DWORD)wcslen(i_pName) + 128;
    l_pName = new wchar_t[l_dwLen];

    if (NULL == l_pName)
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    ////////////////////////////////////////////////////////////////////////////
    //open mutex and own it
    swprintf_s(l_pName, l_dwLen, L"Local\\m%d%s", GetCurrentProcessId(), i_pName);

    l_pShared->hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, l_pName);
    if (NULL == l_pShared->hMutex)
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    if (WAIT_OBJECT_0 != WaitForSingleObject(l_pShared->hMutex, 250))
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

    l_bRelease = TRUE;

    ////////////////////////////////////////////////////////////////////////////
    //open shared memory object
    swprintf_s(l_pName, l_dwLen, L"Local\\f%d%s", GetCurrentProcessId(), i_pName);

    l_pShared->hMemory = OpenFileMapping(FILE_MAP_ALL_ACCESS,  // read/write access
                                         FALSE,                // do not inherit the name
                                         l_pName               // name of mapping object
                                        );           

    if (NULL == l_pShared->hMemory)
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }
    
    l_pBuffer = (tUINT8*)MapViewOfFile(l_pShared->hMemory,    
                                       FILE_MAP_READ,
                                       0,
                                       0,
                                       i_wSize
                                      );
    
    if (NULL == l_pBuffer)
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }
    
    __try
    {
        memcpy(o_pData, l_pBuffer, i_wSize);
    }
    
    __except (   GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR 
                ? EXCEPTION_EXECUTE_HANDLER 
                : EXCEPTION_CONTINUE_SEARCH
                )
    {
        l_bReturn = FALSE;
        goto l_lblExit;
    }

l_lblExit:
    if (l_pName)
    {
        delete [] l_pName;
        l_pName = NULL;
    }

    if (l_pBuffer)
    {
        UnmapViewOfFile(l_pBuffer);
        l_pBuffer = NULL;
    }

    if (l_bRelease)
    {
        ReleaseMutex(l_pShared->hMutex);
    }

    Shared_Close(l_pShared);
    l_pShared = NULL;

    return l_bReturn;
}//Shared_Get
