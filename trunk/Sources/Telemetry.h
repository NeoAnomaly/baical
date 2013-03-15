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
// This file provide trace functionality                                       /
////////////////////////////////////////////////////////////////////////////////
#ifndef TELEMETRY_H_AZH
#define TELEMETRY_H_AZH

#define P7_ASSERT(cond) typedef int assert_type[(cond) ? 1 : -1]


////////////////////////////////////////////////////////////////////////////////
class CP7Tel_Counter
{
private:
    tBOOL          m_bInitialized;

public:
    sP7Tel_Counter m_sCounter;

    CP7Tel_Counter(tUINT8        i_bID,
                   tUINT8        i_bOn,
                   tINT64        i_llMin,
                   tINT64        i_llMax,
                   tINT64        i_llAlarm,
                   const tXCHAR *i_pName
                   );
    ~CP7Tel_Counter();

    tBOOL           Is_Name(const tWCHAR *i_pName);
    tBOOL           Is_Initialized();
    void            Enable(tUINT8 i_bOn);
};


class CP7Telemetry:
    public IP7C_Channel
   ,public IP7_Telemetry
{
    //put volatile variables at the top, to obtain 32 bit alignment. 
    //Project has 8 bytes alignment by default
    tINT32 volatile         m_lReference;

    IP7_Client             *m_pClient; 
    tUINT32                 m_dwChannel_ID;

    tBOOL                   m_bInitialized;

    tLOCK                   m_sCS; 

    sP7Tel_Info             m_sHeader_Info;
    sP7Tel_Value            m_sValue;

    CP7Tel_Counter         *m_pCounters[P7TELEMETRY_COUNTERS_MAX_COUNT];

    tUINT32                 m_dwResets_Channel; 
    tUINT32                 m_dwResets_Counters; 
    sP7C_Status             m_sStatus;

    sP7C_Data_Chunk        *m_pChunks;
    tUINT32                 m_dwChunks_Max_Count;

    tBOOL                   m_bIs_Channel;

    tUINT32                 m_dwLast_ID;

    hShared                *m_pShared;

public:
    CP7Telemetry(IP7_Client *i_pClient, const tXCHAR *i_pName); 
    ~CP7Telemetry();

    tBOOL          Is_Initialized();

    void           Init(sP7C_Channel_Info *i_pInfo);
    void           On_Receive(tUINT32 i_dwChannel, tUINT8 *i_pBuffer, tUINT32 i_dwSize);

    virtual tBOOL  Create(const tXCHAR  *i_pName, 
                          tINT64         i_llMin,
                          tINT64         i_llMax,
                          tINT64         i_llAlarm,
                          tUINT8         i_bOn,
                          tUINT8        *o_pID 
                         );

    virtual tBOOL  Find(const tXCHAR *i_pName, tUINT8 *o_pID);

    virtual tBOOL  Add(tUINT8 i_bID, tINT64 i_llValue);

    virtual tBOOL  Share(const tXCHAR *i_pName);

    virtual tINT32 Add_Ref()
    {
        return ATOMIC_INC(&m_lReference);
    }

    virtual tINT32 Release()
    {
        tINT32 l_lResult = ATOMIC_DEC(&m_lReference);
        if ( 0 >= l_lResult )
        {
            delete this;
        }

        return l_lResult;
    }

private:
    tBOOL   Inc_Chunks(tUINT32 i_dwInc);
};

#endif //TELEMETRY_H_AZH
