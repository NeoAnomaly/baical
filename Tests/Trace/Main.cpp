//******************************************************************************
// Copyright 2010, 2011, 2012  Zheltovskiy Andrey                              *
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
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>

#include "PTypes.h"
#include "GTypes.h"

#include "Length.h"

#include "PAtomic.h"
#include "PLock.h"
#include "PString.h"
#include "PTime.h"
#include "IMEvent.h"
#include "PMEvent.h"
#include "PThreadShell.h"
#include "CRC32.h"
#include "RBTree.h"
#include "AList.h"

#include "Ticks.h"
#include "PWString.h"
#include "PFileSystem.h"
#include "PConsole.h"

#include "PProcess.h"
#include "IJournal.h"
#include "PJournal.h"

#include "P7_Client.h"
#include "P7_Trace.h"
#include "P7_Telemetry.h"


#if defined(_MSC_VER) 

    #pragma warning(disable : 4267)
    #pragma warning(disable : 4995)
    #pragma warning(disable : 4996)
    //allow to see dump of memory leaks at the end of test in VS debug output
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>

    #define DUMP_MEMORY_LEAKS()  _CrtDumpMemoryLeaks()
    #define SPRINTF              swprintf
    #define STRNICMP             strnicmp

#elif defined (__GNUC__)

    #define DUMP_MEMORY_LEAKS()  printf("memory leaks detection is missing !")
    #define SPRINTF              sprintf
    #define STRNICMP             strncasecmp

#endif



#define TEST_HELP                                                      "/?"
#define TEST_NUMBER                                                    "/Test="


////////////////////////////////////////////////////////////////////////////////
//                                   TEST 1                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we made simple test Create and destroy P7 client and P7 trace         //
////////////////////////////////////////////////////////////////////////////////

#define TEST_01_COUNT                                                  "/Count="


////////////////////////////////////////////////////////////////////////////////
//Test_01
int Test_01(int i_iArgC, char* i_pArgV[])
{
    IP7_Client *l_pClient  = NULL;
    IP7_Trace  *l_pTrace   = NULL;
    tUINT32     l_dwCount  = 1000;
    tXCHAR      l_pChannel[128];
    tBOOL       l_bError   = FALSE;

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test create and destroy P7 client and P7 trace in loop\n");
            printf("Command line arguments:\n");
            printf("/Count    : Count iterations\n");
            printf("            Default value - 1000, min 100, max - uint32\n");
            printf("            Example: /Count=10000\n");
            goto l_lExit;
        }
        else if (0 == STRNICMP(i_pArgV[l_iI], TEST_01_COUNT, LENGTH(TEST_01_COUNT)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_01_COUNT)-1, "%d", &l_dwCount);
            if (100 > l_dwCount)
            {
                l_dwCount = 100;
            }
        }
    }

    printf("Start: Iterations count %d\n", l_dwCount);
    
    for (tUINT32 l_dwI = 0; l_dwI < l_dwCount; l_dwI++)
    {
       printf("Iteration = %d\n", l_dwI);

        l_pClient = P7_Create_Client(NULL);

        if (NULL == l_pClient)
        {
            printf("Error: P7 engine was not initialized. Iteration = %d, Exit\n", l_dwI);
            l_bError = TRUE;
        }
        else
        {
            for (tUINT32 l_dwJ = 0; l_dwJ < 2; l_dwJ++)
            {
                SPRINTF(l_pChannel, TM("Chanel[%03d - %02d]"), l_dwI, l_dwJ);
                l_pTrace = P7_Create_Trace(l_pClient, l_pChannel);

                if (l_pTrace)
                {
                    for (tUINT32 l_dwJ = 0; l_dwJ < 5000; l_dwJ ++)
                    {
                        l_pTrace->P7_INFO(0, TM("Iteration %d"), l_dwJ);
                    }

                    l_pTrace->Release();
                    l_pTrace = NULL;

                    CThShell::Sleep(500);
                }
                else
                {
                    printf("Error: P7_Create_Trace failed. Iteration = %d, Exit\n", l_dwI);
                    l_bError = TRUE;
                }
            }

            l_pClient->Release();
            l_pClient = NULL;
        }

        if (l_bError)
        {
            break;
        }
    }

l_lExit:
    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }

    return 0;
}//Test_01


////////////////////////////////////////////////////////////////////////////////
//                                   TEST 2                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we check ability of few P7 trace to work in parallel                  //
////////////////////////////////////////////////////////////////////////////////

#define TEST_02_CHANNELS_COUNT_MAX                                          (10)

//from 1(~1000 traces per second) to 10(~10 000 traces per second)
#define TEST_02_SPEED_MAX                                                   (10)


#define TEST_02_CONSOLE_ARG_COUNT                                  "/Count="
#define TEST_02_CONSOLE_ARG_SPEED                                  "/Speed="
#define TEST_02_CONSOLE_ARG_DURATION                               "/Duration="


struct sTest_02_Thread
{
    IP7_Client       *pClient;
    IMEvent          *iEvent;
    tUINT32           dwIndex;
    tUINT32           dwSpeed;
    CThShell::tTHREAD hThread;
    tBOOL             bThread;
};


////////////////////////////////////////////////////////////////////////////////
//Test_02_Embedded_Trace
static void Test_02_Embedded_Trace(IP7_Trace    *i_pTrace, 
                                   const tXCHAR *i_pFormat, 
                                   ...
                                  )
{
    if (i_pTrace)
    {
        i_pTrace->Trace_Embedded(0, 
                                 EP7TRACE_LEVEL_INFO, 
                                 1, 
                                 (tUINT16)__LINE__,
                                 __FILE__,
                                 __FUNCTION__,
                                 &i_pFormat
                                );
    }
}//Test_02_Embedded_Trace


////////////////////////////////////////////////////////////////////////////////
//Test_02_Routine
static THSHELL_RET_TYPE THSHELL_CALL_TYPE Test_02_Routine(void *i_pContext)
{
    sTest_02_Thread *l_pParam     = (sTest_02_Thread*)i_pContext;
    IP7_Trace       *l_pTrace     = NULL;
    tUINT32          l_dwSent     = 0;
    tUINT32          l_dwRejected = 0;
    tUINT64          l_qwIDX      = 0;
    tUINT32          l_dwTime     = GetTickCount();
    tXCHAR           l_pName[256];

    if (NULL == l_pParam)
    {
        printf("Error: thread input parameter is NULL\n");
        return 0;
    }

    SPRINTF(l_pName, TM("Trace Channel %d"), l_pParam->dwIndex);

    l_pTrace = P7_Create_Trace(l_pParam->pClient, l_pName);

    if (NULL == l_pTrace)
    {
        printf("Thread %d: Error: P7_Create_Trace() failed\n", l_pParam->dwIndex);
        return 0;
    }

    l_pTrace->Set_Verbosity(EP7TRACE_LEVEL_TRACE);

    Test_02_Embedded_Trace(l_pTrace, 
                           TM("Create thread %X"), 
                           CProc::Get_Thread_Id()
                          );

    while (MEVENT_TIME_OUT == l_pParam->iEvent->Wait(10))
    {
        for (tUINT32 l_dwI = 0; l_dwI < 10 * l_pParam->dwSpeed; l_dwI++)
        {
            l_qwIDX++;
            if (l_pTrace->P7_QTRACE(1, 
                                    0,
                                    TM("Test 1[%I64d] %d, %d, %s %d"), 
                                    l_qwIDX, 
                                    10, 
                                    20, 
                                    TM("Ups01"), 
                                    l_dwSent
                                   )
               )
            {
                l_dwSent ++;
            }
            else
            {
                l_dwRejected ++;
            }

            if (0 == (l_qwIDX % 10))
            {
                l_qwIDX ++;
                if (l_pTrace->P7_DEBUG(1, TM("Debug message(3) %s %d, %I64d"), TM("P7_DEBUG"), l_dwSent, l_qwIDX))
                {
                    l_dwSent ++;
                }
                else
                {
                    l_dwRejected ++;
                }
            }

            if (0 == (l_qwIDX % 22))
            {
                l_qwIDX ++;
                if (l_pTrace->P7_INFO(2, TM("Info message(1) %s"), TM("P7_INFO")))
                {
                    l_dwSent ++;
                }
                else
                {
                    l_dwRejected ++;
                }
            }

            if (0 == (l_qwIDX % 153))
            {
                l_qwIDX ++;
                if (l_pTrace->P7_WARNING(3, TM("Warning message(1) %d"), l_dwSent))
                {
                    l_dwSent ++;
                }
                else
                {
                    l_dwRejected ++;
                }
            }


            if (0 == (l_qwIDX % 155))
            {
                l_qwIDX ++;
                if (l_pTrace->P7_ERROR(4, TM("ERROR message(2) %I64d, %s"), l_qwIDX, TM("P7_ERROR")))
                {
                    l_dwSent ++;
                }
                else
                {
                    l_dwRejected ++;
                }
            }
        }//for (tUINT32 l_dwI = 0; l_dwI < 10 * TEST_SPEED; l_dwI++)


        if (CTicks::Difference(GetTickCount(), l_dwTime) >= 1000)
        {
            printf("Thread %d: Traces %d (%d) / %d ms\n",
                    l_pParam->dwIndex,
                    l_dwSent,
                    l_dwRejected,
                    CTicks::Difference(GetTickCount(), l_dwTime)
                   );
            l_dwTime = GetTickCount();
            l_dwSent  = 0;
            l_dwRejected  = 0;
        }
    }//while (WAIT_TIMEOUT == WaitForSingleObject(g_pEvent_Exit, 10))

    Test_02_Embedded_Trace(l_pTrace, 
                           TM("Leave thread %X"), 
                           CProc::Get_Thread_Id()
                          );

    l_pTrace->P7_CRITICAL(5, TM("All done, bye bye"), 0);

    if (l_pTrace)
    {
        l_pTrace->Release();
        l_pTrace = NULL;
    }

    CThShell::Cleanup();

    return THSHELL_RET_OK;
}//Test_02_Routine


////////////////////////////////////////////////////////////////////////////////
//Test_02
int Test_02(int i_iArgC, char* i_pArgV[])
{
    IP7_Client     *l_pClient    = NULL;
    tBOOL           l_bExit      = FALSE;
    tUINT32         l_dwCount    = 1;
    tUINT32         l_dwSpeed    = 1;
    tUINT32         l_dwDuration = 0;
    tUINT32         l_dwStart    = GetTickCount();
    CMEvent        *l_pEvent     = NULL; 
    sTest_02_Thread l_pThreads[TEST_02_CHANNELS_COUNT_MAX];

    memset(l_pThreads, 0, sizeof(l_pThreads));

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test create one P7 client and defined by user amount of threads\n");
            printf("each thread create own P7 Trace object and start sending traces with\n");
            printf("defined by used speed\n");
            printf("Command line arguments:\n");
            printf("/Count    : Count of simultaneously opened streams\n");
            printf("            Default value - 2, min 1, max 10\n");
            printf("            Example: /Count=10\n");
            printf("/Speed    : Delivery Speed, min 1, max 10000\n");
            printf("            Default value - 1\n");
            printf("            Example: /Speed=5\n");
            printf("/Duration : Working duration in seconds\n");
            printf("            Default value is 0 - infinite\n");
            printf("            Example: /Duration=20\n");
            printf("/P7.Help  : P7 engine command line help\n");
            goto l_lExit;
        }
        else if (0 == STRNICMP(i_pArgV[l_iI], TEST_02_CONSOLE_ARG_COUNT, LENGTH(TEST_02_CONSOLE_ARG_COUNT)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_02_CONSOLE_ARG_COUNT)-1, "%d", &l_dwCount);
            if (    (1 > l_dwCount)
                 || (10 < l_dwCount)
               )
            {
                l_dwCount = 2;
            }
        }
        else if (0 == STRNICMP(i_pArgV[l_iI], TEST_02_CONSOLE_ARG_SPEED, LENGTH(TEST_02_CONSOLE_ARG_SPEED)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_02_CONSOLE_ARG_SPEED)-1, "%d", &l_dwSpeed);
            if (    (1 > l_dwSpeed)
                 || (10000 < l_dwSpeed)
               )
            {
                l_dwSpeed = 1;
            }
        }
        else if (0 == STRNICMP(i_pArgV[l_iI], TEST_02_CONSOLE_ARG_DURATION, LENGTH(TEST_02_CONSOLE_ARG_DURATION)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_02_CONSOLE_ARG_DURATION)-1, "%d", &l_dwDuration);
        }
    }

    l_pClient = P7_Create_Client(NULL);
    if (NULL == l_pClient)
    {
        printf("Error: P7 engine was not initialized. Exit\n");
        goto l_lExit;
    }

    l_pEvent = new CMEvent(); 

    if (    (NULL == l_pEvent)
         || (FALSE == l_pEvent->Init(1, EMEVENT_SINGLE_MANUAL))
       )
    {
        printf("Error: Event was not created\n");
        goto l_lExit;
    }

    printf("Start: Duration %d, Delivery speed %d, Channels count %d. Press Esc to exit\n",
           l_dwDuration,
           l_dwSpeed,
           l_dwCount
          );

    for (tUINT32 l_dwI = 0; l_dwI < l_dwCount; l_dwI++)
    {
        l_pThreads[l_dwI].dwIndex = l_dwI;
        l_pThreads[l_dwI].pClient = l_pClient;
        l_pThreads[l_dwI].dwSpeed = l_dwSpeed;
        l_pThreads[l_dwI].iEvent  = l_pEvent;
        l_pThreads[l_dwI].bThread = TRUE;

        if (FALSE == CThShell::Create(&Test_02_Routine, 
                                      &l_pThreads[l_dwI], 
                                      &l_pThreads[l_dwI].hThread,
                                      TM("Test 02")           
                                     )
           )
        {
            l_pThreads[l_dwI].bThread = FALSE;
            printf("Failed to create thread\n");
        }
    }

    while (FALSE == l_bExit)
    {
        CThShell::Sleep(500);

        if (    (Is_Key_Hit())
             && (27 == Get_Char())
           )
        {
            printf("Esc ... exiting\n");
            l_bExit = TRUE;
        }

        if (    (l_dwDuration)
             && (CTicks::Difference(GetTickCount(), l_dwStart) >= (l_dwDuration * 1000))
           )
        {
            printf("Working time is expired ... exiting\n");
            l_bExit = TRUE;
        }
    }

    l_pEvent->Set(0);

l_lExit:

    for (tUINT32 l_dwI = 0; l_dwI < l_dwCount; l_dwI++)
    {
        if (l_pThreads[l_dwI].bThread)
        {
            if (FALSE == CThShell::Close(l_pThreads[l_dwI].hThread, 60000))
            {
                printf("ERROR: thread %d timeout !\n", l_dwI);
            }
        }
    }

    if (l_pEvent)
    {
        delete l_pEvent;
        l_pEvent = NULL;
    }

    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }

    return 0;
}//Test_02



////////////////////////////////////////////////////////////////////////////////
//                                   TEST 3                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we check ability of few P7 trace to work with different formats       //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//Test_03
int Test_03(int i_iArgC, char* i_pArgV[])
{
    IP7_Client     *l_pClient    = NULL;
    tBOOL           l_bExit      = FALSE;
    IP7_Trace      *l_pTrace     = NULL;
    const tXCHAR   *l_pName      = TM("Formats test");

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test create one P7 client and one P7.Trace for sending\n");
            printf("different traces with all possible argument's formats\n");
            printf("There is no additional command line arguments for this test\n");
            goto l_lExit;
        }
    }

    l_pClient = P7_Create_Client(NULL);
    if (NULL == l_pClient)
    {
        printf("Error: P7 engine was not initialized. Exit\n");
        goto l_lExit;
    }

    l_pTrace = P7_Create_Trace(l_pClient, l_pName);
    if (NULL == l_pClient)
    {
        printf("Error: P7.Trace engine was not initialized. Exit\n");
        goto l_lExit;
    }


    ////////////////////////////////////////////////////////////////////////////
    l_pTrace->P7_DEBUG(1, TM("Strings test"), 0);
    l_pTrace->P7_INFO(1, 
                      TM("String: %s (%%s = 12345 abcde .. z // \\ ABCDE .. Z)"), 
                      TM("12345 abcde .. z // \\ ABCDE .. Z")
                     );

    l_pTrace->P7_INFO(1, 
                      TM("String: %ls (%%ls = 12345 abcde .. z // \\ ABCDE .. Z)"), 
                      TM("12345 abcde .. z // \\ ABCDE .. Z")
                     );

    l_pTrace->P7_INFO(1, 
                      TM("String: %hs (%%hs = 12345 abcde .. z // \\ ABCDE .. Z)"), 
                      "12345 abcde .. z // \\ ABCDE .. Z"
                     );

    l_pTrace->P7_INFO(1, 
                      TM("String: %ws (%%ws = 12345 abcde .. z // \\ ABCDE .. Z)"), 
                      TM("12345 abcde .. z // \\ ABCDE .. Z")
                     );

    l_pTrace->P7_INFO(1, 
                      TM("String: %hS (%%hS = 12345 abcde .. z // \\ ABCDE .. Z)"), 
                      "12345 abcde .. z // \\ ABCDE .. Z"
                     );


    ////////////////////////////////////////////////////////////////////////////
    l_pTrace->P7_DEBUG(2, TM("Characters test"), 0);

    l_pTrace->P7_INFO(2, 
                      TM("Char: %c (%%c = G)"), 
                      TM('G')
                     );

    l_pTrace->P7_INFO(2, 
                      TM("Char: %hc (%%hc = G)"), 
                      'G'
                     );

    l_pTrace->P7_INFO(2, 
                      TM("Char: %C (%%C = F)"), 
                      'F'
                     );

    l_pTrace->P7_INFO(2, 
                      TM("Char: %hC (%%hC = R)"), 
                      'R'
                     );


    l_pTrace->P7_INFO(2, 
                      TM("Char: %lc (%%lc = S)"), 
                      TM('S')
                     );

    l_pTrace->P7_INFO(2, 
                      TM("Char: %wc (%%wc = Z)"), 
                      TM('Z')
                     );


    ////////////////////////////////////////////////////////////////////////////
    l_pTrace->P7_DEBUG(3, TM("Integer values"), 0);

    l_pTrace->P7_INFO(3, 
                      TM("Decamical %d (%%d = 1234567890) | %i (%%i = 1234567890) | %u (%%u = 1234567890) "), 
                      (tINT32)1234567890,
                      (tINT32)1234567890,
                      (tUINT32)1234567890
                     );

    l_pTrace->P7_INFO(3, 
                      TM("Octal  %o (%%o = 11145401322)"), 
                      (tUINT32)1234567890
                     );


    l_pTrace->P7_INFO(3, 
                      TM("Hex  %x (%%x = 499602d2) | %X (%%X = 499602D2)"), 
                      (tUINT32)1234567890,
                      (tUINT32)1234567890
                     );

    l_pTrace->P7_INFO(3, 
                      TM("Decamical %ld (%%ld = 1234567890) | %li (%%li = 1234567890) | %lu (%%lu = 1234567890) "), 
                      (tINT32)1234567890,
                      (tINT32)1234567890,
                      (tUINT32)1234567890
                     );

    l_pTrace->P7_INFO(3, 
                      TM("Octal  %lo (%%lo = 11145401322)"), 
                      (tUINT32)1234567890
                     );


    l_pTrace->P7_INFO(3, 
                      TM("Hex  %lx (%%lx = 499602d2) | %lX (%%lX = 499602D2)"), 
                      (tUINT32)1234567890,
                      (tUINT32)1234567890
                     );


    l_pTrace->P7_INFO(3, 
                      TM("Decamical %lld (%%lld = 1234567890987) | %lli (%%lli = 1234567890987ULL)"), 
                      (tINT64)1234567890987ULL,
                      (tINT64)1234567890987ULL
                     );


    l_pTrace->P7_INFO(3, 
                      TM("Octal  %llo (%%llo = 21756176604053ULL)"), 
                      (tUINT64)1234567890987ULL
                     );


    l_pTrace->P7_INFO(3, 
                      TM("Hex  %llx (%%llx = ffaabbccee12) | %llX (%%llX = FFAABBCCEE12)"), 
                      (tUINT64)0xffaabbccee12ULL,
                      (tUINT64)0xFFAABBCCEE12ULL
                     );



    l_pTrace->P7_INFO(3, 
                      TM("Decamical %hd (%%hd = 32700) | %hi (%%hi = 32700) | %hu (%%hu = 65500) "), 
                      (tINT16)32700,
                      (tINT16)32700,
                      (tUINT16)65500
                     );

    l_pTrace->P7_INFO(3, 
                      TM("Octal  %ho (%%ho = 77674)"), 
                      (tUINT16)32700
                     );


    l_pTrace->P7_INFO(3, 
                      TM("Hex  %hx (%%hx = AABB) | %hX (%%hX = AABB)"), 
                      (tUINT16)0xAABB,
                      (tUINT16)0xAABB
                     );



    l_pTrace->P7_INFO(3, 
                      TM("Decamical %I32d (%%I32d = 1234567890) | %I32i (%%I32i = 1234567890) | %I32u (%%I32u = 1234567890) "), 
                      (tINT32)1234567890,
                      (tINT32)1234567890,
                      (tUINT32)1234567890
                     );

    l_pTrace->P7_INFO(3, 
                      TM("Octal  %I32o (%%I32o = 11145401322)"), 
                      (tUINT32)1234567890
                     );


    l_pTrace->P7_INFO(3, 
                      TM("Hex  %I32x (%%I32x = 499602d2) | %I32X (%%I32X = 499602D2)"), 
                      (tUINT32)1234567890,
                      (tUINT32)1234567890
                     );



    l_pTrace->P7_INFO(3, 
                      TM("Decamical %I64d (%%I64d = 1234567890987) | %I64i (%%I64i = 1234567890987)"), 
                      (tINT64)1234567890987ULL,
                      (tINT64)1234567890987ULL
                     );


    l_pTrace->P7_INFO(3, 
                      TM("Octal  %I64o (%%I64o = 21756176604053)"), 
                      (tUINT64)1234567890987ULL
                     );


    l_pTrace->P7_INFO(3, 
                      TM("Hex  %I64x (%%I64x = ffaabbccee12) | %I64X (%%I64X = FFAABBCCEE12)"), 
                      (tUINT64)0xffaabbccee12ULL,
                      (tUINT64)0xFFAABBCCEE12ULL
                     );


    ////////////////////////////////////////////////////////////////////////////
    l_pTrace->P7_DEBUG(4, TM("Floating point"), 0);


    l_pTrace->P7_INFO(4, 
                      TM("%f (%%f 123456.7890) | %e (%%e) | %E (%%E) | %G (%%G) | %g (%%g) | %A (%%A) | %a (%%a)"), 
                      (tDOUBLE)123456.7890,
                      (tDOUBLE)123456.7890,
                      (tDOUBLE)123456.7890,
                      (tDOUBLE)123456.7890,
                      (tDOUBLE)123456.7890,
                      (tDOUBLE)123456.7890,
                      (tDOUBLE)123456.7890
                     );


    ////////////////////////////////////////////////////////////////////////////
    l_pTrace->P7_DEBUG(5, TM("Long VA test"), 0);

    l_pTrace->P7_INFO(5, 
                      TM("%s (String1), %d(1), %d(2), %d(3), %d(4) ")
                      TM("%I64X(ABCDEF12345678) %d(5), %d(6), %d(7), %d(8) ")
                      TM("%llx(abcdef12345678) %d(9), %d(10), %d(11), %d(12) ")
                      TM("%f(987654321.12345) %d(13), %d(14), %d(15), %d(16) ")
                      TM("%c(X) %d(17), %d(18), %d(19), %d(20) ")
                      TM("%hd(32000) %d(21), %d(22), %d(23), %d(24) ")
                      TM("%s(String2) %d(25), %d(26), %d(27), %d(28) "),
                      TM("String1"),
                      (tINT32)1,
                      (tINT32)2,
                      (tINT32)3,
                      (tINT32)4,
                      (tUINT64)0xABCDEF12345678ULL,
                      (tINT32)5,
                      (tINT32)6,
                      (tINT32)7,
                      (tINT32)8,
                      (tUINT64)0xABCDEF12345678ULL,
                      (tINT32)9,
                      (tINT32)10,
                      (tINT32)11,
                      (tINT32)12,
                      (tDOUBLE)987654321.12345,
                      (tINT32)13,
                      (tINT32)14,
                      (tINT32)15,
                      (tINT32)16,
                      TM('X'),
                      (tINT32)17,
                      (tINT32)18,
                      (tINT32)19,
                      (tINT32)20,
                      (tINT16)32000,
                      (tINT32)21,
                      (tINT32)22,
                      (tINT32)23,
                      (tINT32)24,
                      TM("String2"),
                      (tINT32)25,
                      (tINT32)26,
                      (tINT32)27,
                      (tINT32)28
                     );


l_lExit:
    if (l_pTrace)
    {
        l_pTrace->Release();
        l_pTrace = NULL;
    }

    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }

    return 0;
}//Test_03


////////////////////////////////////////////////////////////////////////////////
//                                   TEST 4                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we send so much traces as we can                                      //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//Test_04
int Test_04(int i_iArgC, char* i_pArgV[])
{
    IP7_Client     *l_pClient    = NULL;
    tBOOL           l_bExit      = FALSE;
    tUINT32         l_dwTime     = GetTickCount();
    tUINT32         l_dwSent     = 0;
    tUINT32         l_dwRejected = 0;
    IP7_Trace      *l_pTrace     = NULL;
    const tXCHAR   *l_pName      = TM("Speed test");

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test create one P7 client and one P7.Trace for sending\n");
            printf("so much traces as you hardware can and print statistics\n");
            goto l_lExit;
        }
    }

    l_pClient = P7_Create_Client(NULL);
    if (NULL == l_pClient)
    {
        printf("Error: P7 engine was not initialized. Exit\n");
        goto l_lExit;
    }

    l_pTrace = P7_Create_Trace(l_pClient, l_pName);
    if (NULL == l_pClient)
    {
        printf("Error: P7.Trace engine was not initialized. Exit\n");
        goto l_lExit;
    }


    ////////////////////////////////////////////////////////////////////////////
    while (FALSE == l_bExit)
    {
        for (tUINT32 l_dwI = 0; l_dwI < 10000; l_dwI ++)
        {
            if (l_pTrace->P7_QDEBUG(0, 1, TM("Strings test %d"), l_dwI))
            {
                l_dwSent ++;
            }
            else
            {
                l_dwRejected ++;
            }
        }

        if (CTicks::Difference(GetTickCount(), l_dwTime) >= 1000)
        {
            printf("Sent = %d Rejected = (%d) / %d ms\n",
                    l_dwSent,
                    l_dwRejected,
                    CTicks::Difference(GetTickCount(), l_dwTime)
                   );
            l_dwTime = GetTickCount();
            l_dwSent  = 0;
            l_dwRejected  = 0;
            
            if (    (Is_Key_Hit())
                 && (27 == Get_Char())
               )
            {
                printf("Esc ... exiting\n");
                l_bExit = TRUE;
            }
            
        }
    }//while (FALSE == l_bExit)
    
    l_pTrace->P7_ERROR(1, TM("Done !"), 0);

l_lExit:
    if (l_pTrace)
    {
        l_pTrace->Release();
        l_pTrace = NULL;
    }

    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }

    return 0;
}//Test_04


////////////////////////////////////////////////////////////////////////////////
//                                   TEST 5                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we test telemetry creation and destroy and simple interface tests     //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//Test_05

#define TEST_05_COUNTERS_COUNT                                               (5)

int Test_05(int i_iArgC, char* i_pArgV[])
{
    IP7_Client    *l_pClient    = NULL;
    IP7_Telemetry *l_pTelemetry = NULL;
    tUINT32        l_dwCount    = 1000;
    tBOOL          l_bError     = FALSE;
    tXCHAR         l_pChannel[128];
    tXCHAR         l_pCounter[128];
    tUINT8         l_pID[TEST_05_COUNTERS_COUNT];

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test create and destroy P7 and telemetry in loop\n");
            printf("Command line arguments:\n");
            printf("/Count    : Count iterations\n");
            printf("            Default value - 1000, min 1, max - uint32\n");
            printf("            Example: /Count=10000\n");
            goto l_lExit;
        }
        else if (0 == STRNICMP(i_pArgV[l_iI], TEST_01_COUNT, LENGTH(TEST_01_COUNT)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_01_COUNT) - 1, "%d", &l_dwCount);
            if (1 > l_dwCount)
            {
                l_dwCount = 1;
            }
        }
    }

    printf("Start: Iterations count %d\n", l_dwCount);
    
    for (tUINT32 l_dwI = 0; l_dwI < 1/*l_dwCount*/; l_dwI++)
    {
       printf("Iteration = %d\n", l_dwI);

        l_pClient = P7_Create_Client(NULL);

        if (NULL == l_pClient)
        {
            printf("Error: P7 engine was not initialized. Iteration = %d, Exit\n", l_dwI);
            l_bError = TRUE;
        }
        else
        {
            for (tUINT32 l_dwJ = 0; l_dwJ < 1/*3*/; l_dwJ++)
            {
                SPRINTF(l_pChannel, TM("Telemetry[%03d - %02d]"), l_dwI, l_dwJ);
                l_pTelemetry = P7_Create_Telemetry(l_pClient, l_pChannel);

                if (l_pTelemetry)
                {
                    memset(l_pID, 0, sizeof(l_pID));

                    if (FALSE == l_bError)
                    {
                        for (tUINT32 l_dwK = 0; l_dwK < TEST_05_COUNTERS_COUNT; l_dwK++)
                        {
                            SPRINTF(l_pCounter, TM("Counter %02d"), l_dwK);

                            if (FALSE == l_pTelemetry->Create(l_pCounter, 
                                                              0,
                                                              10000,
                                                              9000, 
                                                              1,
                                                              &l_pID[l_dwK]
                                                             )
                               )
                            {
                                printf("Error: Failed to create counter. Iteration = %d:%d:%d Exit\n",
                                       l_dwI,
                                       l_dwJ,
                                       l_dwK
                                       );
                                l_bError = TRUE;
                            }
                        }
                    }


                    if (FALSE == l_bError)
                    {
                        for (tUINT32 l_dwK = 0; l_dwK < TEST_05_COUNTERS_COUNT; l_dwK++)
                        {
                            SPRINTF(l_pCounter, TM("Counter %02d"), l_dwK);

                            if (FALSE == l_pTelemetry->Find(l_pCounter, &l_pID[l_dwK]))
                            {
                                printf("Error: Failed to find counter. Iteration = %d:%d:%d Exit\n",
                                       l_dwI,
                                       l_dwJ,
                                       l_dwK
                                       );
                                l_bError = TRUE;
                            }
                        }
                    }

                    if (FALSE == l_bError)
                    {
                        SPRINTF(l_pCounter, TM("Counter"), 0);

                        tUINT8 l_bID = 0;
                        if (TRUE == l_pTelemetry->Find(l_pCounter, &l_bID))
                        {
                            printf("Error: Failed to find counter. Iteration = %d:%d Exit\n",
                                    l_dwI,
                                    l_dwJ
                                    );
                            l_bError = TRUE;
                        }
                    }

                    if (FALSE == l_bError)
                    {
                        for (tUINT32 l_dwJ = 0; l_dwJ < 10000; l_dwJ ++)
                        {
                            l_pTelemetry->Add(l_pID[l_dwJ % TEST_05_COUNTERS_COUNT], l_dwJ);
                        }
                    }

                    l_pTelemetry->Release();
                    l_pTelemetry = NULL;

                    CThShell::Sleep(500);
                }
                else
                {
                    printf("Error: P7_Create_Telemetry failed. Iteration = %d, Exit\n", l_dwI);
                    l_bError = TRUE;
                }
            }

            l_pClient->Release();
            l_pClient = NULL;
        }

        if (l_bError)
        {
            break;
        }
    }

l_lExit:
    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }

    return 0;
}//Test_05



////////////////////////////////////////////////////////////////////////////////
//                                   TEST 6                                   //
////////////////////////////////////////////////////////////////////////////////
// Here we send so much telemetry samples as we can                           //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//Test_06

#define TEST_06_COUNTERS_COUNT                                               (2)

int Test_06(int i_iArgC, char* i_pArgV[])
{
    IP7_Client    *l_pClient    = NULL;
    IP7_Telemetry *l_pTelemetry = NULL;

    tBOOL          l_bError     = FALSE;
    tBOOL          l_bExit      = FALSE;
    tUINT32        l_dwSent     = 0;
    tUINT32        l_dwRejected = 0;
    tUINT32        l_dwTime     = 0;
    double         l_dbVal      = -1.0;

    tXCHAR         l_pCounter[128];
    tUINT8         l_pID[TEST_06_COUNTERS_COUNT];

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_HELP, LENGTH(TEST_HELP)-1))
        {
            printf("This test send telemetry samples with max. possible speed\n");
            goto l_lExit;
        }
    }

    l_pClient = P7_Create_Client(NULL);

    if (NULL == l_pClient)
    {
        printf("Error: P7 engine was not initialized");
        goto l_lExit;
    }

    l_pTelemetry = P7_Create_Telemetry(l_pClient, TM("Tel. Speed Test"));
    if (NULL == l_pTelemetry)
    {
        printf("Error: P7 Telemetry was not initialized");
        goto l_lExit;
    }


    memset(l_pID, 0, sizeof(l_pID));

//     for (tUINT32 l_dwK = 0; l_dwK < TEST_06_COUNTERS_COUNT; l_dwK++)
//     {
//         SPRINTF(l_pCounter, TM("Counter %02d"), l_dwK);
// 
//         if (FALSE == l_pTelemetry->Create(l_pCounter, 
//                                           -1000,
//                                           1000,
//                                           950, 
//                                           1,
//                                           &l_pID[l_dwK]
//                                          )
//            )
//         {
//             printf("Error: Failed to create counter %d Exit\n", l_dwK);
//             goto l_lExit;
//         }
//     }

    if (FALSE == l_pTelemetry->Create(TM("Buffer usage"), 0, 1000, 950, 1, &l_pID[0]))
    {
        printf("Error: Failed to create counter %d Exit\n", 0);
        goto l_lExit;
    }

    if (FALSE == l_pTelemetry->Create(TM("Sensor T1"), -1000, 1000, 950, 1, &l_pID[1]))
    {
        printf("Error: Failed to create counter %d Exit\n", 0);
        goto l_lExit;
    }

    ////////////////////////////////////////////////////////////////////////////

    l_dwTime = GetTickCount();

    while (FALSE == l_bExit)
    {
//         for (tUINT32 l_dwI = 0; l_dwI < 10000; l_dwI ++)
//         {
//             if (l_pTelemetry->Add(l_pID[l_dwI % TEST_06_COUNTERS_COUNT], l_dwI))
//             {
//                 l_dwSent ++;
//             }
//             else
//             {
//                 l_dwRejected ++;
//             }
//         }


        // for (tUINT32 l_dwI = 0; l_dwI < TEST_06_COUNTERS_COUNT; l_dwI ++)
        // {
        //     if (l_pTelemetry->Add(l_pID[l_dwI], 
        //                             (tINT64)(sin(l_dbVal + (double)l_dwI * 0.5) * 1000.0)
        //                             )
        //         )
        //     {
        //         l_dwSent ++;
        //     }
        //     else
        //     {
        //         l_dwRejected ++;
        //     }
        // }
        // 
        // l_dbVal += 0.05;


        for (tUINT32 l_dwJ = 0; l_dwJ < 5; l_dwJ ++)
        {
            if (l_pTelemetry->Add(l_pID[0], ((tINT64)(l_dbVal * 100.0)) % 1000ULL))
            {
                l_dwSent ++;
            }
            else
            {
                l_dwRejected ++;
            }

            if (l_pTelemetry->Add(l_pID[1], (tINT64)(sin(l_dbVal) * 1000.0)))
            {
                l_dwSent ++;
            }
            else
            {
                l_dwRejected ++;
            }

            l_dbVal += 0.05;
        }

        CThShell::Sleep(1);

        if (CTicks::Difference(GetTickCount(), l_dwTime) >= 1000)
        {
            printf("Sent = %d Rejected = (%d) / %d ms\n",
                    l_dwSent,
                    l_dwRejected,
                    CTicks::Difference(GetTickCount(), l_dwTime)
                   );
            l_dwTime     = GetTickCount();
            l_dwSent     = 0;
            l_dwRejected = 0;
            
            if (    (Is_Key_Hit())
                 && (27 == Get_Char())
               )
            {
                printf("Esc ... exiting\n");
                l_bExit = TRUE;
            }
            
        }
    }//while (FALSE == l_bExit)


l_lExit:
    if (l_pTelemetry)
    {
        l_pTelemetry->Release();
        l_pTelemetry = NULL;
    }

    if (l_pClient)
    {
        l_pClient->Release();
        l_pClient = NULL;
    }

    return 0;
}//Test_06



////////////////////////////////////////////////////////////////////////////////
//                                   MAIN                                     //
////////////////////////////////////////////////////////////////////////////////
int main(int i_iArgC, char* i_pArgV[])
{
    printf("******************************************************************\n");
    printf("P7 Traces test tool\n");
    printf("******************************************************************\n");


    int l_iTest    = -1;
    int l_iReturn  = 0;

    for (int l_iI = 0; l_iI < i_iArgC; l_iI++)
    {
        if (0 == STRNICMP(i_pArgV[l_iI], TEST_NUMBER, LENGTH(TEST_NUMBER)-1))
        {
            sscanf(i_pArgV[l_iI] + LENGTH(TEST_NUMBER)-1, "%d", &l_iTest);
        }
    }

    if (1 == l_iTest)
    {
        l_iReturn = Test_01(i_iArgC, i_pArgV);
    }
    else if (2 == l_iTest)
    {
        l_iReturn = Test_02(i_iArgC, i_pArgV);
    }
    else if (3 == l_iTest)
    {
        l_iReturn = Test_03(i_iArgC, i_pArgV);
    }
    else if (4 == l_iTest)
    {
        l_iReturn = Test_04(i_iArgC, i_pArgV);
    }
    else if (5 == l_iTest)
    {
        l_iReturn = Test_05(i_iArgC, i_pArgV);
    }
    else if (6 == l_iTest)
    {
        l_iReturn = Test_06(i_iArgC, i_pArgV);
    }
    else
    {
        printf("ERROR: Test number is not specified or out of range\n");
        printf("Test command line arguments:\n");
        printf("   /Test : test number to execute. Min value = 1, max value = 6.\n");
        printf("           Example: /Test=1\n");
        printf("           To get help about test: /Test=1 /?\n");
        printf("   /?    : show help for specific test.\n");
        printf("           Example: /Test=1 /?\n");
        PRINTF(CLIENT_HELP_STRING, 0);
    }

    
    
    DUMP_MEMORY_LEAKS();

    return l_iReturn;
}


//"${OUTPUT_PATH}" /P7.Addr=192.168.0.3 /P7.Port=9009 /P7.Verb=1 /P7.Window=128 /P7.PSize=1474 /Test=3 /Speed=10 /Count=3 /P7.Pool=4096
