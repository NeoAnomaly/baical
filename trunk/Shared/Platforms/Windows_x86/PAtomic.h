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

///////////////////////////////////////////////////////////////////////////////
#define  ATOMIC_ADD(io_Val, i_Add)  InterlockedExchangeAdd((LONG volatile *)io_Val, i_Add)
#define  ATOMIC_SUB(io_Val, i_Sub)  InterlockedExchangeAdd((LONG volatile *)io_Val, -((tINT32)i_Sub))

#define  ATOMIC_INC(io_Val)         InterlockedIncrement((LONG volatile *)io_Val)
#define  ATOMIC_DEC(io_Val)         InterlockedDecrement((LONG volatile *)io_Val)
