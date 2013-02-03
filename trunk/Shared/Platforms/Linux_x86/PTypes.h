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
#ifndef PTYPES_H_AZH
#define PTYPES_H_AZH


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

#endif //PTYPES_H_AZH
