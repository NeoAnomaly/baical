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

#pragma once

////////////////////////////////////////////////////////////////////////////////
//GetTickCount - exist in OS
//tUINT32 GetTickCount()
//{
//}//GetTickCount


////////////////////////////////////////////////////////////////////////////////
//GetPerformanceCounter
static tUINT64 GetPerformanceCounter()
{
    LARGE_INTEGER l_qwValue;
    l_qwValue.QuadPart = 0;
    QueryPerformanceCounter(&l_qwValue);
    return l_qwValue.QuadPart;
}//GetPerformanceCounter


////////////////////////////////////////////////////////////////////////////////
//GetPerformanceFrequency
static tUINT64 GetPerformanceFrequency()
{
    LARGE_INTEGER l_qwValue;
    l_qwValue.QuadPart = 0;
    QueryPerformanceFrequency(&l_qwValue);
    return l_qwValue.QuadPart;
}//GetPerformanceFrequency


////////////////////////////////////////////////////////////////////////////////
//GetEpochTime
//return a 64-bit value of 100-nanosecond intervals since January 1, 1601 (UTC).
static void GetEpochTime(tUINT32 *o_pHi, tUINT32 *o_pLow)
{
    SYSTEMTIME l_sSTime = {0};
    FILETIME   l_sFTime = {0};
    GetSystemTime(&l_sSTime);
    
    SystemTimeToFileTime(&l_sSTime, &l_sFTime);
    if (o_pHi)
    {
        *o_pHi = l_sFTime.dwHighDateTime;
    }
    
    if (o_pLow)
    {
        *o_pLow = l_sFTime.dwLowDateTime;
    }
}//GetEpochTime
