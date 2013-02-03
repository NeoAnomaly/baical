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
#ifndef TICKS_H_AZH
#define TICKS_H_AZH


////////////////////////////////////////////////////////////////////////////////
class CTicks
{
public:
    static tUINT32 inline Difference(tUINT32 i_dwCur, tUINT32 i_dwPrev)
    {
        tUINT32 l_dwReturn = 0;

        if (i_dwCur >= i_dwPrev)
        {
            l_dwReturn = i_dwCur - i_dwPrev;
        }
        else
        {
            l_dwReturn = (0xFFFFFFFF - i_dwPrev) + i_dwCur;
        }

        return l_dwReturn;
    }
};

#endif //TICKS_H_AZH