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
#ifndef LENGTH_H_AZH
#define LENGTH_H_AZH

template <typename Type, size_t i_szLength>
char (&GetCharArray(Type (&i_pArray)[i_szLength]))[i_szLength];
#define LENGTH(i_pArray) (sizeof(GetCharArray(i_pArray)))

#define MEMBER_SIZE(Structure, Memeber)         sizeof(((Structure*)0)->Memeber)


#endif //LENGTH_H_AZH