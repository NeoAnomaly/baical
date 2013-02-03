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

#include "PTypes.h"
#include "GTypes.h"

#include "PLock.h"
#include "PTime.h"
#include "PThreadShell.h"
#include "Ticks.h"
#include "PConsole.h"

#include "P7_Client.h"
#include "P7_Trace.h"



tBOOL Print2Buffer(char       *o_pBuffer, 
                   const char *i_pFormat,
                   ...
                  )
{
    if (    (NULL == o_pBuffer)
         || (NULL == i_pFormat)
       )
    {
        return FALSE;
    }

    va_list arglist;
    va_start(arglist, i_pFormat);
    vsprintf(o_pBuffer, i_pFormat, arglist);
    va_end(arglist);

    return TRUE;
} 


#define ITERATIONS_COUNT                                                (100000)


int main(int i_iArgC, char* i_pArgV[])
{
    printf("This test measures the time it is necessary for:\n");
    printf(" 1. print %d times simple string to buffer\n", ITERATIONS_COUNT);
    printf(" 2. print %d times simple string to console\n", ITERATIONS_COUNT);
    printf("    this test combine test #1 and console printing\n", ITERATIONS_COUNT);
    printf(" 3. print & deliver %d times simple string to trace server\n", ITERATIONS_COUNT);
    printf("    server will receive:\n", ITERATIONS_COUNT);
    printf("       * Text message\n");
    printf("       * Level (error, warning, .. etc)\n");
    printf("       * Time with 100ns granularity\n");
    printf("       * Current thread ID\n");
    printf("       * Current module ID\n");
    printf("       * Current processor number\n");
    printf("       * Function name\n");
    printf("       * File name\n");
    printf("       * File line number\n");
    printf("       * Sequence number\n");
    printf("*************************************************************************\n");
    printf("Press ENTER to start ....\n");

    Get_Char();

    tUINT64 l_dwTime   = 0;
    tUINT64 l_dwTime_1 = 0;
    tUINT64 l_dwTime_2 = 0;
    tUINT64 l_dwTime_3 = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Test 1
    char l_pText[256];
    l_dwTime = GetPerformanceCounter();

    for (tUINT32 l_dwI = 0; l_dwI < ITERATIONS_COUNT; l_dwI ++)
    {
        Print2Buffer(l_pText, "Test format string, iteration is %d", l_dwI);
    }

    l_dwTime_1 = (GetPerformanceCounter() - l_dwTime) / (GetPerformanceFrequency() / 1000);


    ////////////////////////////////////////////////////////////////////////////
    //Test 2
    l_dwTime = GetPerformanceCounter();

    for (tUINT32 l_dwI = 0; l_dwI < ITERATIONS_COUNT; l_dwI ++)
    {
        printf("Test format string, iteration is %d", l_dwI);
    }

    l_dwTime_2 = (GetPerformanceCounter() - l_dwTime) / (GetPerformanceFrequency() / 1000);


    ////////////////////////////////////////////////////////////////////////////
    //Test 3
    IP7_Client *l_pClient = P7_Create_Client(0);
    IP7_Trace  *l_pTrace  = P7_Create_Trace(l_pClient, TM("Speed Test"));

    if (    (l_pClient)
         && (l_pTrace)
       )
    {
        l_dwTime = GetPerformanceCounter();

        for (tUINT32 l_dwI = 0; l_dwI < ITERATIONS_COUNT; l_dwI ++)
        {
            l_pTrace->P7_QTRACE(0, 0, TM("Test format string, iteration is %d"), l_dwI);
        }

        l_dwTime_3 = (GetPerformanceCounter() - l_dwTime) / (GetPerformanceFrequency() / 1000);
    }

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


    printf("\n*****************************************************************\n");
    printf("Test results:\n");
    printf("  Test 1 duration: %d ms\n", (tUINT32)l_dwTime_1);
    printf("  Test 2 duration: %d ms\n", (tUINT32)l_dwTime_2);
    printf("  Test 3 duration: %d ms\n", (tUINT32)l_dwTime_3);

    return 0;
}

