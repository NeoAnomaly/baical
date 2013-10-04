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
#ifndef GTYPE_H_AZH
#define GTYPE_H_AZH

////////////////////////////////////////////////////////////////////////////////
//WINDOWS specific definitions & types
#if defined(_WIN32) || defined(_WIN64)
    #define _WINSOCKAPI_
    #include <windows.h>

    //Text marco, allow to use wchar_t automatically
    #define TM(i_pStr)         L##i_pStr

    #define XCHAR              wchar_t

    #define PRAGMA_PACK_ENTER(x)  __pragma(pack(push, x))
    #define PRAGMA_PACK_EXIT()   __pragma(pack(pop))

    #define ATTR_PACK(x)
////////////////////////////////////////////////////////////////////////////////
//LINUX specific definitions & types
#elif defined(__linux__)
    #define UTF8_ENCODING

    //Text marco, allow to use char automatically
    #define TM(i_pStr)    i_pStr

    #define XCHAR         char

    #define __stdcall

    #define __forceinline  __attribute__((always_inline))

    #define PRAGMA_PACK_ENTER(x) 
    #define PRAGMA_PACK_EXIT(x) 

    #define ATTR_PACK(x) __attribute__ ((aligned(x), packed))
    //#define ATTR_PACK(x) __attribute__ ((alignment(x), packed))
#endif


#define TRUE         1
#define FALSE        0

#define UNUSED_ARG(x)        (void)(x)


typedef unsigned long long   tUINT64;
typedef long long            tINT64;
typedef unsigned int         tUINT32;
typedef int                  tINT32;
typedef unsigned short       tUINT16;
typedef short                tINT16;
typedef unsigned char        tUINT8;
typedef char                 tINT8;
typedef char                 tACHAR;
typedef short                tWCHAR;
//platfrorm specific char, Windows - wchar_t, Linix - char,
//XCHAR defined in PTypes.hpp specific for each platform or project.
#define tXCHAR               XCHAR  
//typedef XCHAR                tXCHAR;
typedef unsigned int         tBOOL;
typedef double               tDOUBLE;


#endif //GTYPE_H_AZH
