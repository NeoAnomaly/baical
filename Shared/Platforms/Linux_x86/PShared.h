//******************************************************************************
// Copyright 2013  Zheltovskiy Andrey                                          *
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
// Implementation of shared & named memory, memory is shared in process bounds *
// Implementation is not thread safe !                                         *
// NOT IMPLEMENTED !!!!                                                        *
//******************************************************************************

#pragma once


typedef void hShared;


////////////////////////////////////////////////////////////////////////////////
//Shared_Close
static tBOOL Shared_Close(hShared *i_pShared)
{
    return FALSE;
}//Shared_Close


////////////////////////////////////////////////////////////////////////////////
//Shared_Create
static hShared *Shared_Create(const tXCHAR *i_pName,
                              tUINT8       *i_pData, 
                              tUINT16       i_wSize
                             )
{
    return NULL;
}//Shared_Create


////////////////////////////////////////////////////////////////////////////////
//Shared_Get
static tBOOL Shared_Read(const tXCHAR  *i_pName, 
                         tUINT8        *o_pData,
                         tUINT16        i_wSize
                        )
{
    return FALSE;
}//Shared_Get
