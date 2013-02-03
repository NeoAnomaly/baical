//******************************************************************************
// Copyright 2011 Zheltovskiy Andrey                                           *
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
#pragma once

#include <process.h>

#define THSHELL_CALL_TYPE     __stdcall
#define THSHELL_RET_TYPE      tUINT32

#define THSHELL_RET_OK        0
#define THSHELL_RET_NOK       1


////////////////////////////////////////////////////////////////////////////////
class CThShell
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //Base types
    typedef HANDLE  tTHREAD;
    typedef THSHELL_RET_TYPE (THSHELL_CALL_TYPE *tpThreadProc)(void *i_pParameter);


    ////////////////////////////////////////////////////////////////////////////
    //Create
    static tBOOL Create(tpThreadProc  i_pThread_Proc, 
                        void         *i_pPrm, 
                        tTHREAD      *o_pThread,
                        const tXCHAR *i_pName
                       )
    {
        if (    ( NULL == i_pThread_Proc )
             || ( NULL == o_pThread )
           )
        {
            return FALSE;
        }

        *o_pThread = (tTHREAD)_beginthreadex( NULL, 
                                              0, 
                                              i_pThread_Proc,
                                              i_pPrm, 
                                              0, 
                                              NULL
                                            );

        //SetThreadName( *pThread_id, thread_name);

        return (*o_pThread) ? TRUE : FALSE;
    }//Create


    ////////////////////////////////////////////////////////////////////////////
    //Cleanup
    static void Cleanup()
    {
        _endthreadex( 0 );
    }//Thread_Cleanup


    ////////////////////////////////////////////////////////////////////////////
    //Close
    static tBOOL Close(tTHREAD i_hThread, tUINT32 i_dwTimeOut)
    {
        if (NULL == i_hThread)
        {
            return TRUE;
        }

        if ( WAIT_OBJECT_0 == WaitForSingleObject( i_hThread, i_dwTimeOut ) )
        {
            CloseHandle(i_hThread);
            return TRUE;
        }

        return FALSE;
    }//Close


    ////////////////////////////////////////////////////////////////////////////
    //Sleep
    static void Sleep(tUINT32 i_dwTimeOut)
    {
        ::Sleep(i_dwTimeOut);
    }//Sleep
};

