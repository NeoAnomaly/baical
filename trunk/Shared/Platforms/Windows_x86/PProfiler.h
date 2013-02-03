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
// This header file provide hi&low resolution timer functions                  *
//******************************************************************************
#pragma once

#include "Ticks.h"


class CTickHi
{
    LARGE_INTEGER m_qwCounter;
    LARGE_INTEGER m_qwMarkIn;
    LARGE_INTEGER m_qwMarkOut;
    LARGE_INTEGER m_qwResolution;
public:
    CTickHi()
    {
        m_qwMarkIn.QuadPart = 0;
        m_qwMarkOut.QuadPart = 0;
        
        QueryPerformanceFrequency(&m_qwResolution);

        Reset();
        Start();
    }

    void Start()
    {
        QueryPerformanceCounter(&m_qwMarkIn);
    }

    void Stop()
    {
        QueryPerformanceCounter(&m_qwMarkOut);
        m_qwCounter.QuadPart += m_qwMarkOut.QuadPart - m_qwMarkIn.QuadPart;
    }

    void Reset()
    {
        m_qwCounter.QuadPart = 0;
    }

    //Second == 100 000;
    DWORD Get() 
    {
        return (DWORD)(m_qwCounter.QuadPart * 100000ULL / m_qwResolution.QuadPart);
    }
};

class CTickLow
{
    DWORD m_dwStart;
    DWORD m_dwStop;
public:
    CTickLow()
    {
        m_dwStart = 0;
        m_dwStop = 0;

        Reset();
        Start();
    }

    void Start()
    {
        m_dwStart = GetTickCount();
    }

    void Stop()
    {
        m_dwStop = GetTickCount();
    }

    void Reset()
    {
        m_dwStop = 0;
    }

    //frequency 1 000 per second
    DWORD Get() 
    {
        return CTicks::Difference(m_dwStop, m_dwStart);
    }
};

