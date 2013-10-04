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
#ifndef PSTRING_H_AZH
#define PSTRING_H_AZH

#define PRINTF(...)     printf(__VA_ARGS__)
//wprintf(__VA_ARGS__)

#include "UTF.h"

////////////////////////////////////////////////////////////////////////////////
//PStrLen
static __attribute__ ((unused)) tUINT32 PStrLen(const tXCHAR *i_pText) 
{
    return strlen(i_pText);
}//PStrLen

////////////////////////////////////////////////////////////////////////////////
//PStrCpy
static __attribute__ ((unused)) tXCHAR* PStrCpy(tXCHAR       *i_pDst,
                                                size_t        i_szDst,
                                                const tXCHAR *i_pSrc)
{
    UNUSED_ARG(i_szDst);
    return strcpy(i_pDst, i_pSrc);
}//PStrLen


////////////////////////////////////////////////////////////////////////////////
//PStrNCmp
static __attribute__ ((unused)) tINT32 PStrNCmp(const tXCHAR *i_pS1,
                                                const tXCHAR *i_pS2,
                                                size_t        i_szLen
                                               )
{
    return strncmp(i_pS1, i_pS2, i_szLen);
}//PStrNCmp


////////////////////////////////////////////////////////////////////////////////
//PStrNCmp
static __attribute__ ((unused)) tINT32 PStrNiCmp(const tXCHAR *i_pS1,
                                                 const tXCHAR *i_pS2,
                                                 size_t        i_szLen
                                                )
{
    return strncasecmp(i_pS1, i_pS2, i_szLen);
}//PStrNCmp


////////////////////////////////////////////////////////////////////////////////
//PStrToInt
static __attribute__ ((unused)) tINT32 PStrToInt(const tXCHAR *i_pS1)
{
    return atoi(i_pS1);
}//PStrToInt


////////////////////////////////////////////////////////////////////////////////
//PUStrLen
static __attribute__ ((unused)) tUINT32 PUStrLen(const tXCHAR *i_pText)
{
    return Get_UTF8_Length(i_pText);
}//PUStrLen



////////////////////////////////////////////////////////////////////////////////
//PStrCpy
static __attribute__ ((unused)) void PUStrCpy(tWCHAR       *i_pDst,
                                              tUINT32       i_dwMax_Len,
                                              const tXCHAR *i_pSrc)
{
    Convert_UTF8_To_UTF16(i_pSrc, i_pDst, i_dwMax_Len);
    //wcscpy_s(i_pDst, i_dwMax_Len, i_pSrc);
}//PStrCpy

#endif //PSTRING_H_AZH
