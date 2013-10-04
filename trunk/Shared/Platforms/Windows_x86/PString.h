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

#include <Strsafe.h>

#define PRINTF(...)     wprintf(__VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
//PStrLen
static tUINT32 PStrLen(const tXCHAR *i_pText)
{
    return (tUINT32)wcslen(i_pText);
}//PStrLen

///////////////////////////////////////////////////////////////////////////////
//PUStrLen
static tUINT32 PUStrLen(const tXCHAR *i_pText)
{
    return (tUINT32)wcslen(i_pText);
}//PUStrLen


////////////////////////////////////////////////////////////////////////////////
//PStrCpy
static tXCHAR* PStrCpy(tXCHAR       *i_pDst,
                       size_t        i_szDst,
                       const tXCHAR *i_pSrc
                      )
{
    wcscpy_s((wchar_t*)i_pDst, i_szDst, (wchar_t*)i_pSrc);
    return i_pDst;
}//PStrLen


///////////////////////////////////////////////////////////////////////////////
//PStrNCmp
static tINT32 PStrNCmp(const tXCHAR *i_pS1, const tXCHAR *i_pS2, size_t i_szLen)
{
    return _wcsnicmp(i_pS1, i_pS2, i_szLen);
}//PStrNCmp


////////////////////////////////////////////////////////////////////////////////
//PStrNCmp
static tINT32 PStrNiCmp(const tXCHAR *i_pS1,
                        const tXCHAR *i_pS2,
                        size_t        i_szLen
                       )
{
    return _wcsnicmp(i_pS1, i_pS2, i_szLen);
}//PStrNCmp


///////////////////////////////////////////////////////////////////////////////
//PStrToInt
static tINT32 PStrToInt(const tXCHAR *i_pS1)
{
    return _wtoi(i_pS1);
}//PStrToInt


///////////////////////////////////////////////////////////////////////////////
//PStrCpy
static void PUStrCpy(tWCHAR *i_pDst, tUINT32 i_dwMax_Len, const tXCHAR *i_pSrc)
{
    wcscpy_s((wchar_t *)i_pDst, i_dwMax_Len, i_pSrc);
}//PStrCpy
