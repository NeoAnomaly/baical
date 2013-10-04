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
#ifndef PTHREADSHELL_H_AZH
#define PTHREADSHELL_H_AZH


#define THSHELL_CALL_TYPE
#define THSHELL_RET_TYPE void*

#define THSHELL_RET_OK   NULL
#define THSHELL_RET_NOK  0xDEAD

#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
class CThShell
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //Base types
    typedef pthread_t tTHREAD;
    typedef THSHELL_RET_TYPE (THSHELL_CALL_TYPE *tpThreadProc)(void *i_pParameter);


    ////////////////////////////////////////////////////////////////////////////
    //Create
    static tBOOL Create(tpThreadProc  i_pThread_Proc, 
                        void         *i_pPrm, 
                        tTHREAD      *o_pThread,
                        const tXCHAR *i_pName
                       )
    {
        UNUSED_ARG(i_pName);
        if (    ( NULL == i_pThread_Proc )
             || ( NULL == o_pThread )
           )
        {
            return FALSE;
        }

        *o_pThread = 0;
        
        pthread_attr_t l_sAttr;
        int            l_iResult = -1;
        
        pthread_attr_init(&l_sAttr);
        pthread_attr_setdetachstate(&l_sAttr, PTHREAD_CREATE_JOINABLE);
        
        l_iResult = pthread_create(o_pThread, &l_sAttr, i_pThread_Proc, i_pPrm);
        
        pthread_attr_destroy(&l_sAttr);
        
#ifdef __USE_GNU
        //if (    (0 == l_iResult)
        //     && (i_pName)    
        //   )
        //{
        //    pthread_setname_np(pthread_self(), i_pName);
        //}
#endif
        
        return (0 == l_iResult) ? TRUE : FALSE;
    }//Create

    ////////////////////////////////////////////////////////////////////////////
    //Cleanup
    static void Cleanup()
    {
    }//Thread_Cleanup


    ////////////////////////////////////////////////////////////////////////////
    //Close
    static tBOOL Close(tTHREAD i_hThread, tUINT32 i_dwTimeOut)
    {
        UNUSED_ARG(i_dwTimeOut);

        void *l_pRet = THSHELL_RET_OK;

        tUINT32 l_iResult = pthread_join(i_hThread, &l_pRet);
        
        return (( 0 == l_iResult ) && (THSHELL_RET_OK == l_pRet)) ? TRUE : FALSE;
    }//Close

    ////////////////////////////////////////////////////////////////////////////
    //Sleep
    static void Sleep(tUINT32 i_dwTimeOut)
    {
        usleep(1000 * i_dwTimeOut);
    }//Sleep

};

#endif //PTHREADSHELL_H_AZH
