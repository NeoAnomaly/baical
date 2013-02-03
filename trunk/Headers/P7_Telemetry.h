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
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                            P7.Telemetry                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#ifndef P7_TELEMETRY_H_AZH
#define P7_TELEMETRY_H_AZH


//__declspec(novtable)
class IP7_Telemetry
{
public:
    ////////////////////////////////////////////////////////////////////////////
    //Add_Ref - increase object's reference count
    //
    //Output:
    // tINT32 - new reference count value
    virtual tINT32 Add_Ref()                                                = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Release - decrease object's reference count. If reference count less or
    //          equal to 0 - object will be destroyed
    //
    //Output:
    // tINT32 - new reference count value
    virtual tINT32 Release()                                                = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Create - create new telemetry counter, max amount of counters - 256
    //
    //Inputs:
    // i_pName   - Counter name, max length - 64 characters
    // i_bSize   - value size in bytes, acceptable values are: 1,2,4,8
    // i_llMin   - minimal counter value
    // i_llMax   - max counter value
    // i_llAlarm - alarm value
    // i_bOn     - Disable or enable counter. If you disable counter in your 
    //             code and try to add value to that counter - the value will be
    //             discarded. Later user can enable your counter through Baical
    // o_pID     - in success case function will return ID of the new counter  
    //
    //Output:
    // tBOOL - TRUE/FALSE
    virtual tBOOL Create(const tXCHAR  *i_pName, 
                         tINT64         i_llMin,
                         tINT64         i_llMax,
                         tINT64         i_llAlarm,
                         tUINT8         i_bOn,
                         tUINT8        *o_pID 
                        )                                                   = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Add - add counter value
    //
    //Inputs:
    // i_bID     - counter ID
    // i_llValue - counter value:
    //
    //Output:
    // tBOOL - TRUE/FALSE
    virtual tBOOL Add(tUINT8 i_bID, tINT64 i_llValue)                       = 0;

    ////////////////////////////////////////////////////////////////////////////
    //Find - find counter ID by name
    //
    //Inputs:
    // i_pName  - counter name
    // o_pID    - counter ID
    //
    //Output:
    // tBOOL - TRUE/FALSE
    virtual tBOOL Find(const tXCHAR *i_pName, tUINT8 *o_pID)                = 0;
};


////////////////////////////////////////////////////////////////////////////////
//P7_Create_Telemetry - function create new instance of IP7_Telemetry object
//
//Inputs:
// i_pClient - P7 network engine. This engine will be used to deliver trace to
//             Baical server
// i_pName   - Name of the session, you can specify any name. It will be used
//             by Baical to show you trace session
//Output:
// IP7_Telemetry - object for telemetry counters. If NULL - error, see logs
extern IP7_Telemetry * __stdcall P7_Create_Telemetry(IP7_Client   *i_pClient,
                                                     const tXCHAR *i_pName
                                                    );

#endif //P7_TELEMETRY_H_AZH