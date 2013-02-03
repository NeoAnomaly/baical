////////////////////////////////////////////////////////////////////////////////
// Copyright 2011 Zheltovskiy Andrey                                           /
//                                                                             /
//   Licensed under the Apache License, Version 2.0 (the "License");           /
//   you may not use this file except in compliance with the License.          /
//   You may obtain a copy of the License at                                   /
//                                                                             /
//       http://www.apache.org/licenses/LICENSE-2.0                            /
//                                                                             /
//  Unless  required  by  applicable  law  or  agreed  to in writing, software /
//  distributed  under the License is distributed on an "AS IS" BASIS, WITHOUT /
//  WARRANTIES  OR  CONDITIONS OF ANY KIND, either express or implied. See the /
//  License  for  the  specific language governing permissions and limitations /
//  under the License.                                                         /
//                                                                             /
////////////////////////////////////////////////////////////////////////////////
#pragma once

#define _WINSOCKAPI_
#include <windows.h>

//Text marco, allow to use wchar_t automatically
#define TM(i_pStr)         L##i_pStr

#define XCHAR              wchar_t

#define PRAGMA_PACK_ENTER(x)  __pragma(pack(push, x))
#define PRAGMA_PACK_EXIT()   __pragma(pack(pop))

#define ATTR_PACK(x)
