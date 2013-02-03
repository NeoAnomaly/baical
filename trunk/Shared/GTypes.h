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

#define TRUE         1
#define FALSE        0

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
typedef XCHAR                tXCHAR;
typedef unsigned int         tBOOL;
typedef double               tDOUBLE;


#endif //GTYPE_H_AZH
