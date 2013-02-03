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
#ifndef IMEVENT_H_AZH
#define IMEVENT_H_AZH


////////////////////////////////////////////////////////////////////////////////
enum eMEvent_Type
{
    EMEVENT_SINGLE_AUTO         = 0,
    EMEVENT_SINGLE_MANUAL          ,
    EMEVENT_MULTI                  ,
};


#define MEVENT_TIME_OUT        (0xFFFFFFF)
#define MEVENT_SIGNAL_0        (0)


////////////////////////////////////////////////////////////////////////////////
class IMEvent
{
public:
    //For example 
    //Init(3, EMEVENT_SINGLE_AUTO, EMEVENT_SINGLE_MANUAL, EMEVENT_MULTI);  
    virtual tBOOL   Init(tUINT8 i_bCount, ...)                              = 0;
    virtual tBOOL   Set(tUINT32 i_dwID)                                     = 0;
    virtual tBOOL   Clr(tUINT32 i_dwID)                                     = 0;
    virtual tUINT32 Wait()                                                  = 0;
    virtual tUINT32 Wait(tUINT32 i_dwMSec)                                  = 0;
};

#endif //IMEVENT_H_AZH