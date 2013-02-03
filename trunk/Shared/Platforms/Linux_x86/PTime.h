//******************************************************************************
// Copyright 2012  Zheltovskiy Andrey                                          *
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
#ifndef PTIME_H_AZH
#define PTIME_H_AZH

#include <sys/time.h>

//time offset from January 1, 1601 to January 1, 1970, resolution 100ns
#define TIME_OFFSET_1601_1970                            (116444736000000000ULL)


////////////////////////////////////////////////////////////////////////////////
//GetTickCount
static tUINT32 GetTickCount()
{
    tUINT64 l_qwReturn; //Warn:without initialization !
    //timeval l_sTime;
    //gettimeofday(&l_sTime, NULL);

    struct timespec l_sTime = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &l_sTime);
    
    l_qwReturn  = l_sTime.tv_sec;
    l_qwReturn *= 1000;
    l_qwReturn += l_sTime.tv_nsec/1000000;
    
    return (tUINT32)l_qwReturn;    
}//GetTickCount


////////////////////////////////////////////////////////////////////////////////
//GetPerformanceCounter
static tUINT64 GetPerformanceCounter()
{
    tUINT64 l_qwReturn      = 0;
    struct timespec l_sTime = {0, 0};
    
    clock_gettime(CLOCK_MONOTONIC, &l_sTime);
    
    l_qwReturn  = (tUINT64)(l_sTime.tv_sec) * 10000000;
    l_qwReturn += (tUINT64)(l_sTime.tv_nsec) / 100;
    
    return l_qwReturn;
    
    //LARGE_INTEGER l_qwValue;
    //l_qwValue.QuadPart = 0;
    //QueryPerformanceCounter(&l_qwValue);
    //return l_qwValue.QuadPart;
}//GetPerformanceCounter


////////////////////////////////////////////////////////////////////////////////
//GetPerformanceFrequency
static tUINT64 GetPerformanceFrequency()
{
    return 10000000; //100 nano second
    //LARGE_INTEGER l_qwValue;
    //l_qwValue.QuadPart = 0;
    //QueryPerformanceFrequency(&l_qwValue);
    //return l_qwValue.QuadPart;
}//GetPerformanceFrequency


////////////////////////////////////////////////////////////////////////////////
//GetEpochTime
//return a 64-bit value of 100-nanosecond intervals since January 1, 1601 (UTC).
static void GetEpochTime(tUINT32 *o_pHi, tUINT32 *o_pLow)
{
    tUINT64        l_qwResult = 0;
    struct timeval l_sTime    = {0, 0};
    
    gettimeofday(&l_sTime, NULL);

    l_qwResult  = (tUINT64)(l_sTime.tv_sec) * 10000000;
    l_qwResult += (tUINT64)(l_sTime.tv_usec) * 100;
    l_qwResult += TIME_OFFSET_1601_1970;

    if (o_pHi)
    {
        *o_pHi  = (tUINT32)(l_qwResult >> 32);
    }
    
    if (o_pLow)
    {
        *o_pLow = (tUINT32)(l_qwResult & 0xFFFFFFFF);
    }
    
    
    //SYSTEMTIME l_sSTime = {0};
    //FILETIME   l_sFTime = {0};
    //GetSystemTime(&l_sSTime);
    //
    //SystemTimeToFileTime(&l_sSTime, &l_sFTime);
    //if (o_pHi)
    //{
    //    *o_pHi = l_sFTime.dwHighDateTime;
    //}
    //
    //if (o_pLow)
    //{
    //    *o_pLow = l_sFTime.dwLowDateTime;
    //}
}//GetEpochTime

#endif //PTIME_H_AZH
